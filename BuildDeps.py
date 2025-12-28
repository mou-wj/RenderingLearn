"""
BuildDeps.py
- Scans config/3rds/*.json
- Resolves dependency order
- For each dependency:
    - If deps/install/<name> exists -> skip (unless force rebuild)
    - If need_pull -> git clone to deps/<name>
    - If need_compile -> run CMake configure/build/install per json.cmake settings into deps/install/<name>/<CONFIG>
- Generate cmake/Find3rdsGenerated.cmake (creates IMPORTED INTERFACE targets)
"""

import os
import sys
import json
import glob
import shutil
import subprocess
from pathlib import Path
from typing import Dict, Any, List, Set, Tuple

ROOT = Path(__file__).resolve().parent
CONFIG_DIR = ROOT / "config" / "3rds"
DEPS_DIR = ROOT / "deps"
BUILD_DIR = DEPS_DIR / "build"
INSTALL_DIR_BASE = DEPS_DIR / "install"
CMAKE_OUTPUT = ROOT / "cmake"
CPU_COUNT = max(1, os.cpu_count() or 2)


def run(cmd: List[str], cwd: Path = None, env: Dict[str, str] = None) -> int:
    print(f"[run] {' '.join(cmd)}  (cwd={cwd})")
    p = subprocess.run(cmd, cwd=str(cwd) if cwd else None, env=env, shell=(os.name == "nt"))
    return p.returncode


def load_json(path: Path) -> Dict[str, Any]:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def ensure_dirs():
    DEPS_DIR.mkdir(parents=True, exist_ok=True)
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    INSTALL_DIR_BASE.mkdir(parents=True, exist_ok=True)
    CMAKE_OUTPUT.mkdir(parents=True, exist_ok=True)


def expand_placeholders(s: str, install_dir: str, config: str) -> str:
    return s.replace("${INSTALL_DIR}", install_dir).replace("${CONFIG}", config)


def clone_repo(name: str, git_url: str, branch: str, dest: Path) -> bool:
    if dest.exists():
        print(f"[clone] destination {dest} already exists, skipping clone")
        return True
    cmd = ["git", "clone", "--branch", branch, "--single-branch", git_url, str(dest)]
    return run(cmd) == 0


def detect_platform_tag() -> str:
    if sys.platform.startswith("win"):
        return "windows"
    if sys.platform.startswith("linux"):
        return "linux"
    if sys.platform.startswith("darwin"):
        return "macos"
    return "unknown"


def compute_generator_and_toolset(meta: Dict[str, Any]) -> Tuple[List[str], Dict[str, str]]:
    """
    Return extra cmake configure args (list) and env overrides (dict) based on meta.compilers and platform.
    """
    platform_tag = detect_platform_tag()
    compilers = meta.get("compilers", {})
    plat_conf = compilers.get(platform_tag, {}) if compilers else {}
    args = []
    env = {}
    # prefer explicit cmake_generator / toolset if provided
    gen = plat_conf.get("cmake_generator")
    toolset = plat_conf.get("cmake_toolset")
    if gen:
        args += ["-G", gen]
    if toolset:
        args += ["-T", toolset]
    # allow user to pass environment specifics (not mandatory)
    preferred = plat_conf.get("preferred")
    if preferred:
        env["BUILD_PREFERRED_COMPILER"] = preferred
    return args, env


def cmake_configure_and_build(src: Path, build_dir: Path, install_dir: Path, meta: Dict[str, Any]) -> bool:
    """
    New implementation:
      - uses meta to determine whether to build (need_compile / do_build)
      - selects generator/toolset via meta.compilers for current platform
      - honors library type (shared/static) via -DBUILD_SHARED_LIBS
      - supports multi-config and single-config generators
    """
    need_compile = meta.get("need_compile", meta.get("do_build", True))
    if not need_compile:
        print(f"[info] meta indicates no compile required for {meta.get('name')}")
        return True

    cm = meta.get("cmake", {})
    configure_args = cm.get("configure_args", [])
    build_targets = cm.get("build_targets", ["install"])
    build_configs = cm.get("build_configs", ["Release"])
    lib_type = meta.get("type", "shared")  # 'shared' or 'static'

    # prepare generator/toolset and environment
    gen_args, extra_env = compute_generator_and_toolset(meta)

    # ensure build dirs
    build_dir.mkdir(parents=True, exist_ok=True)
    success = True
    for cfg in build_configs:
        cfg_build_dir = build_dir / cfg
        cfg_build_dir.mkdir(parents=True, exist_ok=True)
        install_prefix = str(install_dir / cfg)

        # base cmake configure command
        cmake_cmd = ["cmake", "-S", str(src), "-B", str(cfg_build_dir)]
        # generator/toolset args (if any) go first
        if gen_args:
            cmake_cmd += gen_args
        # set install prefix and build type
        cmake_cmd += [f"-DCMAKE_INSTALL_PREFIX={install_prefix}"]

        # if single-config generator is used we can set CMAKE_BUILD_TYPE
        # but best effort: set it anyway
        cmake_cmd += [f"-DCMAKE_BUILD_TYPE={cfg}"]

        # set BUILD_SHARED_LIBS according to type
        if lib_type.lower() == "static":
            cmake_cmd += ["-DBUILD_SHARED_LIBS=OFF"]
        else:
            cmake_cmd += ["-DBUILD_SHARED_LIBS=ON"]

        # append user configure args with placeholders expanded
        for a in (configure_args or []):
            cmake_cmd.append(expand_placeholders(a, str(install_dir), cfg))

        # run configure
        env = os.environ.copy()
        env.update(extra_env)
        if run(cmake_cmd, cwd=cfg_build_dir, env=env) != 0:
            print(f"[error] cmake configure failed for {meta.get('name')} ({src}) cfg={cfg}")
            success = False
            if not success:
                break

        # build targets
        targets = build_targets or ["install"]
        for t in targets:
            # prefer cmake --build interface (supports multi-config via --config)
            # Use CMake's --parallel to let CMake translate to the correct backend flag
            # (works for Ninja, Makefiles and will translate to /m for MSBuild)
            build_cmd = ["cmake", "--build", str(cfg_build_dir), "--config", cfg, "--target", t, "--parallel", str(CPU_COUNT)]
            if run(build_cmd, cwd=cfg_build_dir, env=env) != 0:
                print(f"[error] cmake build target '{t}' failed for {meta.get('name')} cfg={cfg}")
                success = False
                break
        if not success:
            break
    return success


# ---- dependency sorting utilities ----
def build_dependency_graph(all_meta: List[Dict[str, Any]]) -> Dict[str, Set[str]]:
    """
    Build adjacency: key -> set(dependencies)
    """
    graph: Dict[str, Set[str]] = {}
    name_to_meta = {m.get("name"): m for m in all_meta if m.get("name")}
    for m in all_meta:
        name = m.get("name")
        if not name:
            continue
        deps = set()
        for d in m.get("dependencies", []) or []:
            if d in name_to_meta:
                deps.add(d)
        graph[name] = deps
    return graph


def topological_sort(graph: Dict[str, Set[str]]) -> List[str]:
    """
    Kahn's algorithm. Returns list in order where dependencies come before dependents.
    Raises RuntimeError on cycles.
    """
    # compute in-degree
    inv_graph: Dict[str, Set[str]] = {k: set() for k in graph}
    for node, deps in graph.items():
        for d in deps:
            inv_graph.setdefault(d, set()).add(node)
    # nodes with no deps
    no_deps = [n for n, deps in graph.items() if not deps]
    order: List[str] = []
    graph_copy = {k: set(v) for k, v in graph.items()}
    while no_deps:
        n = no_deps.pop()
        order.append(n)
        # remove edges n -> m (i.e., for all nodes that depend on n)
        for dependent in list(inv_graph.get(n, [])):
            # remove n from their deps
            if n in graph_copy.get(dependent, set()):
                graph_copy[dependent].remove(n)
            # if dependent has no other deps, add to no_deps
            if not graph_copy.get(dependent):
                no_deps.append(dependent)
            # remove back-reference
            inv_graph[n].remove(dependent)
    # if any edges remain, cycle exists
    if any(graph_copy.get(n) for n in graph_copy):
        raise RuntimeError("Dependency cycle detected among 3rd-party libs")
    # There may be nodes that were only dependencies but not in original graph keys; include them
    remaining = [n for n in graph.keys() if n not in order]
    order.extend(remaining)
    return order


def generate_find_cmake(deps_meta: List[Dict[str, Any]]) -> str:
    lines: List[str] = []
    lines.append("# Auto-generated by BuildDeps.py - do not edit by hand")
    lines.append("cmake_minimum_required(VERSION 3.16)\n")
    lines.append("if(NOT DEFINED _3RDS_FIND_GENERATED)\n  set(_3RDS_FIND_GENERATED TRUE)\nendif()\n")

    def cmake_safe_name(s: str) -> str:
        return s.replace('.', '_').replace('-', '_')

    for meta in deps_meta:
        name = meta.get("name")
        if not name:
            continue
        var_prefix = cmake_safe_name(name).upper()
        lines.append(f"# ---- {name} ----")

        install_layout = meta.get("install_layout", {})
        include_tpl = install_layout.get("include", "${INSTALL_DIR}/include")
        lib_tpl = install_layout.get("lib", "${INSTALL_DIR}/lib")
        bin_tpl = install_layout.get("bin")

        cmake_cfg = meta.get("cmake", {})
        find_via = cmake_cfg.get("find_via")
        need_compile = meta.get("need_compile", meta.get("do_build", True))

        # If external and has find_via, emit find_package (or custom find_via)
        if not need_compile and find_via:
            if isinstance(find_via, list):
                for l in find_via:
                    lines.append(l)
            else:
                lines.append(find_via)

            # provide a thin INTERFACE alias named `name` that links to found targets/vars
            pkg_name = None
            if isinstance(find_via, str) and find_via.strip().lower().startswith("find_package"):
                try:
                    inside = find_via[find_via.find("(") + 1:find_via.rfind(")")]
                    pkg_name = inside.split()[0]
                except Exception:
                    pkg_name = None

            tgt = name
            lines.append(f"if(NOT TARGET {tgt})")
            lines.append(f"  add_library({tgt} INTERFACE)")
            if pkg_name:
                possible = f"{pkg_name}::{pkg_name}"
                lines.append(f"  if(TARGET {possible})")
                lines.append(f"    target_link_libraries({tgt} INTERFACE {possible})")
                lines.append(f"  elseif(DEFINED {pkg_name}_LIBRARIES)")
                lines.append(f"    target_link_libraries({tgt} INTERFACE \"${{{pkg_name}_LIBRARIES}}\")")
                lines.append(f"  endif()")
            lines.append("endif()\n")
            continue


        # For built libraries, scan the install layout on the script side and emit
        # concrete IMPORTED targets (one per library file) with per-config properties.
        if need_compile:
            install_base_src = ROOT / "deps" / "install" / name
            lines.append(f"set({var_prefix}_INSTALL_BASE \"${{CMAKE_SOURCE_DIR}}/deps/install/{name}\")")

            debug_targets: List[str] = []
            release_targets: List[str] = []
            for cfg in ["Debug", "Release"]:
                lib_dir = install_base_src / cfg / "lib"
                bin_dir = install_base_src / cfg / "bin"
                if not lib_dir.exists():
                    continue
                for p in sorted(lib_dir.iterdir()):
                    if not p.is_file():
                        continue
                    fname = p.name
                    base = p.stem
                    # per-config target name (unique per config)
                    tgt_name_cfg = f"{name}::{base}_{cfg.lower()}"
                    lines.append(f"if(NOT TARGET {tgt_name_cfg})")
                    # Decide whether this library has a corresponding dynamic
                    # runtime in the package's bin directory. Prefer that when
                    # present (treat as SHARED); otherwise fallback to the
                    # library file (STATIC unless its extension is a shared one).
                    impl_path = f"${{CMAKE_SOURCE_DIR}}/deps/install/{name}/{cfg}/lib/{fname}"
                    dll_name = None
                    if bin_dir.exists():
                        # derive candidate prefix: strip typical "lib" prefix
                        cand_prefix = base[3:] if base.lower().startswith("lib") else base
                        for fb in sorted(bin_dir.iterdir()):
                            if not fb.is_file():
                                continue
                            fb_name = fb.name
                            low = fb_name.lower()
                            # look for common dynamic extensions and matching base
                            if low.endswith(".dll") or low.endswith(".so") or low.endswith(".dylib"):
                                # match either exact base or lib<base> forms
                                if low.startswith(cand_prefix.lower()) or low.startswith(("lib" + cand_prefix).lower()):
                                    dll_name = fb_name
                                    break

                    if dll_name:
                        kind = "SHARED"
                    else:
                        # if the file in lib dir itself is a shared object, honor that
                        if p.suffix.lower() in [".dll", ".so", ".dylib"]:
                            kind = "SHARED"
                        else:
                            kind = "STATIC"

                    lines.append(f"  add_library({tgt_name_cfg} {kind} IMPORTED)")

                    had_location = False
                    if dll_name:
                        dll_path = f"${{CMAKE_SOURCE_DIR}}/deps/install/{name}/{cfg}/bin/{dll_name}"
                        lines.append(f"  set_target_properties({tgt_name_cfg} PROPERTIES IMPORTED_LOCATION_{cfg.upper()} \"{dll_path}\")")
                        # the implib (static lib) still lives in lib; keep it as IMPORTED_IMPLIB
                        lines.append(f"  set_target_properties({tgt_name_cfg} PROPERTIES IMPORTED_IMPLIB_{cfg.upper()} \"{impl_path}\")")
                        had_location = True

                    if not had_location:
                        # fallback to lib file as IMPORTED_LOCATION
                        lines.append(f"  set_target_properties({tgt_name_cfg} PROPERTIES IMPORTED_LOCATION_{cfg.upper()} \"{impl_path}\")")
                        had_location = True

                    # include dir specific to this config
                    inc_dir = f"${{CMAKE_SOURCE_DIR}}/deps/install/{name}/{cfg}/include"
                    lines.append(f"  set_target_properties({tgt_name_cfg} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES \"{inc_dir}\")")
                    lines.append("endif()\n")

                    # add to the proper collection only if we have a valid location
                    if had_location:
                        if cfg == "Debug":
                            debug_targets.append(tgt_name_cfg)
                        else:
                            release_targets.append(tgt_name_cfg)

            # aggregated interface target linking only config-appropriate targets
            agg = f"{name}_all"
            if debug_targets or release_targets:
                lines.append(f"if(NOT TARGET {agg})")
                lines.append(f"  add_library({agg} INTERFACE)")
                if debug_targets:
                    dbg_list = ";".join(debug_targets)
                    lines.append(f"  target_link_libraries({agg} INTERFACE $<$<CONFIG:Debug>:{dbg_list}>)")
                if release_targets:
                    rel_list = ";".join(release_targets)
                    lines.append(f"  target_link_libraries({agg} INTERFACE $<$<CONFIG:Release>:{rel_list}>)")
                lines.append("endif()\n")

        else:
            # fallback: create empty INTERFACE target
            tgt = name
            lines.append(f"if(NOT TARGET {tgt})")
            lines.append(f"  add_library({tgt} INTERFACE)")
            lines.append("endif()\n")

    return "\n".join(lines)


def main():
    ensure_dirs()
    config_files = sorted(CONFIG_DIR.glob("*.json"))
    if not config_files:
        print("[info] no 3rd-party config json files found in", CONFIG_DIR)
        return 0

    all_meta = []
    for cfg_path in config_files:
        try:
            meta = load_json(cfg_path)
        except Exception as e:
            print(f"[error] failed to parse {cfg_path}: {e}")
            continue
        name = meta.get("name")
        if not name:
            print(f"[warn] config {cfg_path} has no 'name', skipping")
            continue
        all_meta.append(meta)

    # build dependency graph and determine order
    graph = build_dependency_graph(all_meta)
    try:
        order = topological_sort(graph)
    except RuntimeError as e:
        print(f"[error] {e}")
        return 1

    # create a map for quick lookup
    name_to_meta = {m["name"]: m for m in all_meta}
    processed_meta = []

    for name in order:
        meta = name_to_meta.get(name)
        if not meta:
            continue
        print(f"[info] processing {name}")

        install_base = INSTALL_DIR_BASE / name
        # if install_base exists and contains subdir(s), assume installed -> skip
        if install_base.exists() and any(install_base.iterdir()):
            print(f"[info] {name} appears installed at {install_base}, skipping build")
            processed_meta.append(meta)
            continue

        need_pull = meta.get("need_pull", False)
        src_target = DEPS_DIR / name
        if need_pull:
            git_url = meta.get("git")
            branch = meta.get("branch", "main")
            if not git_url:
                print(f"[error] {name} need_pull=true but no git url specified")
                continue
            print(f"[info] cloning {name} -> {src_target}")
            if not clone_repo(name, git_url, branch, src_target):
                print(f"[error] git clone failed for {name}")
                continue
        else:
            # If not pulling, expect source already present under deps/<name> OR user will provide install manually
            if not src_target.exists():
                # if the library does not require compile, user may have installed it elsewhere
                if not meta.get("need_compile", meta.get("do_build", False)):
                    print(f"[info] {name} does not require compile and source not present. Skipping build; expect preinstalled files at deps/install/{name}")
                    processed_meta.append(meta)
                    continue
                print(f"[warn] {name} is not configured to be pulled and {src_target} not present. Skipping build. You should provide prebuilt install at deps/install/{name}")
                continue

        # run cmake configure/build/install according to meta
        build_dir = BUILD_DIR / name
        install_dir = INSTALL_DIR_BASE / name
        print(f"[info] building {name} -> install base {install_dir}")
        ok = cmake_configure_and_build(src_target, build_dir, install_dir, meta)
        if not ok:
            print(f"[error] build/install failed for {name}")
            continue
        processed_meta.append(meta)

    # generate cmake find module
    print(f"[info] generating {CMAKE_OUTPUT / 'Find3rdsGenerated.cmake'}")
    content = generate_find_cmake(processed_meta)
    with open(CMAKE_OUTPUT / "Find3rdsGenerated.cmake", "w", encoding="utf-8") as f:
        f.write(content)
    print("[done] BuildDeps finished")
    return 0


if __name__ == "__main__":
    sys.exit(main())