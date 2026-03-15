"""
BuildDeps.py

功能：
- 扫描 config/3rds/*.json 配置文件，每个文件描述一个第三方库
- 根据依赖关系计算构建顺序（拓扑排序）
- 对于 managed 类型库：
  - 自动 git clone 源代码
  - 使用 CMake 进行 configure/build/install
  - 支持 multi-config (Debug/Release)
  - 支持 BUILD_SHARED_LIBS 选项
- 对于 external 类型库：
  - 不进行构建，直接使用指定的路径
  - 根据 auto_generate_find_cmake 决定是否生成 FindCMake
- 生成 cmake/Find3rdsGenerated.cmake 文件，包含 IMPORTED targets
"""

import os
import sys
import json
import re
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
    """
    展开占位符：
    - ${INSTALL_DIR} -> install_dir
    - ${CONFIG} -> config
    - ${ENV:VAR} -> 环境变量
    """
    result = s.replace("${INSTALL_DIR}", install_dir).replace("${CONFIG}", config)
    
    # 替换 ${ENV:VAR} 格式的环境变量
    def _env_repl(m: re.Match) -> str:
        var = m.group(1)
        return os.environ.get(var, "")
    
    result = re.sub(r"\$\{ENV:([^}]+)\}", _env_repl, result)
    return result


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
    根据元数据计算 CMake 生成器和工具集参数
    返回 (cmake_args, env_overrides) 元组
    """
    platform_tag = detect_platform_tag()
    
    # 支持新名称 'compiler' 和旧名称 'compilers'
    compiler_conf = meta.get("compiler", meta.get("compilers", {}))
    if not isinstance(compiler_conf, dict):
        compiler_conf = {}
    
    plat_conf = compiler_conf.get(platform_tag, {})
    
    args = []
    env = {}
    
    # 获取 CMake 生成器
    gen = plat_conf.get("cmake_generator")
    toolset = plat_conf.get("cmake_toolset")
    
    if gen:
        args += ["-G", gen]
    if toolset:
        args += ["-T", toolset]
    
    # 获取首选编译器
    preferred = plat_conf.get("preferred")
    if preferred:
        env["BUILD_PREFERRED_COMPILER"] = preferred
    
    return args, env


def cmake_configure_and_build(src: Path, build_dir: Path, install_dir: Path, meta: Dict[str, Any]) -> bool:
    """
    使用 CMake 配置、构建和安装库
    
    参数：
    - src: 源代码目录
    - build_dir: 构建目录
    - install_dir: 安装目录
    - meta: 库的元数据
    """
    # 只有 managed 类型的库才进行编译
    lib_type = meta.get("type", "managed")
    if lib_type != "managed":
        print(f"[info] skipping build for non-managed library '{meta.get('name')}'")
        return True
    
    # 检查 need_compile 标志
    need_compile = meta.get("need_compile", True)
    if not need_compile:
        print(f"[info] meta indicates no compile required for {meta.get('name')}")
        return True
    
    # 获取构建配置
    build_meta = meta.get("build", {})
    system = build_meta.get("system", "cmake").lower()
    
    if system != "cmake":
        print(f"[info] unsupported build system '{system}', skipping build")
        return True
    
    configure_args = build_meta.get("configure_args", [])
    build_targets = build_meta.get("build_targets", ["install"])
    build_configs = build_meta.get("configs", ["Release"])
    build_type = build_meta.get("type", "shared")  # shared 或 static
    
    if not build_configs:
        build_configs = ["Release"]
    
    # 获取生成器和工具集
    gen_args, extra_env = compute_generator_and_toolset(meta)
    
    # 创建构建目录
    build_dir.mkdir(parents=True, exist_ok=True)
    
    success = True
    for cfg in build_configs:
        cfg_build_dir = build_dir / cfg
        cfg_build_dir.mkdir(parents=True, exist_ok=True)
        install_prefix = str(install_dir / cfg)
        
        # 基础 CMake 配置命令
        cmake_cmd = ["cmake", "-S", str(src), "-B", str(cfg_build_dir)]
        
        # 添加生成器/工具集参数
        if gen_args:
            cmake_cmd += gen_args
        
        # 设置安装前缀和构建类型
        cmake_cmd += [f"-DCMAKE_INSTALL_PREFIX={install_prefix}"]
        cmake_cmd += [f"-DCMAKE_BUILD_TYPE={cfg}"]
        
        # 设置 BUILD_SHARED_LIBS
        if build_type.lower() == "static":
            cmake_cmd += ["-DBUILD_SHARED_LIBS=OFF"]
        else:
            cmake_cmd += ["-DBUILD_SHARED_LIBS=ON"]
        
        # 添加用户指定的配置参数（展开占位符）
        for arg in (configure_args or []):
            cmake_cmd.append(expand_placeholders(arg, str(install_dir), cfg))
        
        # 执行 CMake 配置
        env = os.environ.copy()
        env.update(extra_env)
        
        if run(cmake_cmd, cwd=cfg_build_dir, env=env) != 0:
            print(f"[error] cmake configure failed for '{meta.get('name')}' cfg={cfg}")
            success = False
            break
        
        # 执行构建目标
        targets = build_targets or ["install"]
        for target in targets:
            build_cmd = [
                "cmake", "--build", str(cfg_build_dir),
                "--config", cfg,
                "--target", target,
                "--parallel", str(CPU_COUNT)
            ]
            
            if run(build_cmd, cwd=cfg_build_dir, env=env) != 0:
                print(f"[error] cmake build target '{target}' failed for '{meta.get('name')}' cfg={cfg}")
                success = False
                break
        
        if not success:
            break
    
    return success


# ---- dependency sorting utilities ----
def build_dependency_graph(all_meta: List[Dict[str, Any]]) -> Dict[str, Set[str]]:
    """
    构建依赖图：name -> set(dependencies)
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
    拓扑排序（Kahn 算法）
    返回排序后的库名列表，保证依赖库在被依赖库之前
    """
    # 计算反向图和入度
    inv_graph: Dict[str, Set[str]] = {k: set() for k in graph}
    for node, deps in graph.items():
        for d in deps:
            inv_graph.setdefault(d, set()).add(node)
    
    # 找出没有依赖的节点
    no_deps = [n for n, deps in graph.items() if not deps]
    order: List[str] = []
    graph_copy = {k: set(v) for k, v in graph.items()}
    
    while no_deps:
        n = no_deps.pop()
        order.append(n)
        
        # 移除从 n 出发的边
        for dependent in list(inv_graph.get(n, [])):
            # 从被依赖者的依赖中移除 n
            if n in graph_copy.get(dependent, set()):
                graph_copy[dependent].remove(n)
            
            # 如果被依赖者没有其他依赖，加入 no_deps
            if not graph_copy.get(dependent):
                no_deps.append(dependent)
            
            # 移除反向引用
            inv_graph[n].remove(dependent)
    
    # 检查循环依赖
    if any(graph_copy.get(n) for n in graph_copy):
        raise RuntimeError("Dependency cycle detected among 3rd-party libs")
    
    # 包含仅作为依赖但未在原始图中出现的节点
    remaining = [n for n in graph.keys() if n not in order]
    order.extend(remaining)
    
    return order


def scan_config_cmake_dirs(install_base: Path) -> List[str]:
    """
    扫描安装目录，查找所有 *-config.cmake 或 *-Config.cmake 文件
    返回包含这些文件的目录列表（使用正斜杠路径）
    """
    config_dirs = set()
    
    if not install_base.exists():
        return []
    
    # 递归扫描所有子目录
    for root, dirs, files in os.walk(str(install_base)):
        for file in files:
            if file.endswith(('-config.cmake', '-Config.cmake', 'Config.cmake', 'config.cmake')):
                # 将反斜杠替换为正斜杠
                config_dir = Path(root).as_posix()
                config_dirs.add(config_dir)
    
    return sorted(list(config_dirs))


def generate_find_cmake(deps_meta: List[Dict[str, Any]]) -> str:
    """
    生成 Find3rdsGenerated.cmake 文件内容
    
    设置 XXX_ROOT 变量，让 CMake 的 find_package 能够找到库
    不创建自定义的 INTERFACE targets，让标准 find_package 机制工作
    """
    lines: List[str] = []
    lines.append("# Auto-generated by BuildDeps.py - do not edit by hand")
    lines.append("cmake_minimum_required(VERSION 3.16)\n")
    lines.append("if(NOT DEFINED _3RDS_FIND_GENERATED)")
    lines.append("  set(_3RDS_FIND_GENERATED TRUE)")
    lines.append("endif()\n")
    
    def cmake_safe_name(s: str) -> str:
        return s.replace(".", "_").replace("-", "_")
    
    for meta in deps_meta:
        name = meta.get("name")
        if not name:
            continue
        
        var_prefix = cmake_safe_name(name).upper()
        lines.append(f"# ---- {name} ----")
        
        install_layout = meta.get("install_layout", {})
        root_tpl = install_layout.get("root", "${INSTALL_DIR}/${CONFIG}")
        
        build_meta = meta.get("build", {})
        find_via = build_meta.get("find_via", meta.get("find_via"))
        
        lib_type = meta.get("type", "managed")
        need_compile = (lib_type == "managed" and meta.get("need_compile", True))
        
        # 获取构建的配置列表
        build_configs = build_meta.get("configs", ["Debug", "Release"])
        
        # === External 类型库处理 ===
        if lib_type == "external":
            # 如果声明了 root，保存为 CMake 变量
            root_val = meta.get("root")
            if root_val:
                expanded = expand_placeholders(root_val, "", "")
                # 将反斜杠替换为正斜杠，确保 CMake 兼容性
                expanded = expanded.replace("\\", "/")
                lines.append(f"set({var_prefix}_ROOT \"{expanded}\")")
                
                # 扫描外部库目录，查找 config.cmake 文件
                external_path = Path(expanded)
                if external_path.exists():
                    config_dirs = scan_config_cmake_dirs(external_path)
                    for config_dir in config_dirs:
                        lines.append(f"list(APPEND CMAKE_PREFIX_PATH \"{config_dir}\")")
                    
                    # 如果没有找到 config.cmake，也添加根目录
                    if not config_dirs:
                        lines.append(f"list(APPEND CMAKE_PREFIX_PATH \"{expanded}\")")
            
            # 如果有 find_via，添加自定义查找逻辑
            if find_via:
                if isinstance(find_via, list):
                    for item in find_via:
                        lines.append(item)
                else:
                    lines.append(find_via)
            
            lines.append("")
            continue
        
        # === Managed 类型库处理 ===
        if need_compile:
            install_base_src = ROOT / "deps" / "install" / name
            
            # 设置 XXX_ROOT 变量，让 find_package 能够找到库
            # 对于 multi-config，根据 CMAKE_BUILD_TYPE 设置不同的路径
            root_path = str(install_base_src).replace("\\", "/")
            lines.append(f"set({var_prefix}_ROOT \"{root_path}\")")
            
            # 根据 CMAKE_BUILD_TYPE 添加相应的配置路径
            if len(build_configs) == 1:
                # 单配置
                config_path = str(install_base_src / build_configs[0]).replace("\\", "/")
                lines.append(f"list(APPEND CMAKE_PREFIX_PATH \"{config_path}\")")
                
                # 扫描配置目录的 config.cmake 文件
                config_dirs = scan_config_cmake_dirs(install_base_src / build_configs[0])
                for config_dir in config_dirs:
                    lines.append(f"list(APPEND CMAKE_PREFIX_PATH \"{config_dir}\")")
            else:
                # 多配置，使用条件设置
                for config in build_configs:
                    config_path = str(install_base_src / config).replace("\\", "/")
                    lines.append(f"if(CMAKE_BUILD_TYPE STREQUAL \"{config}\")")
                    lines.append(f"  list(APPEND CMAKE_PREFIX_PATH \"{config_path}\")")
                    
                    # 扫描配置目录的 config.cmake 文件
                    config_dirs = scan_config_cmake_dirs(install_base_src / config)
                    for config_dir in config_dirs:
                        lines.append(f"  list(APPEND CMAKE_PREFIX_PATH \"{config_dir}\")")
                    
                    lines.append("endif()")
            
            # 如果没有找到任何 config.cmake，也添加基础安装目录
            all_config_dirs = []
            for config in build_configs:
                all_config_dirs.extend(scan_config_cmake_dirs(install_base_src / config))
            
            if not all_config_dirs:
                if len(build_configs) == 1:
                    config_path = str(install_base_src / build_configs[0]).replace("\\", "/")
                    lines.append(f"list(APPEND CMAKE_PREFIX_PATH \"{config_path}\")")
                else:
                    for config in build_configs:
                        config_path = str(install_base_src / config).replace("\\", "/")
                        lines.append(f"if(CMAKE_BUILD_TYPE STREQUAL \"{config}\")")
                        lines.append(f"  list(APPEND CMAKE_PREFIX_PATH \"{config_path}\")")
                        lines.append("endif()")
            
            # 如果有 find_via，添加自定义查找逻辑
            if find_via:
                if isinstance(find_via, list):
                    for item in find_via:
                        lines.append(item)
                else:
                    lines.append(find_via)
            
            lines.append("")
    
    return "\n".join(lines)


def main():
    """主函数"""
    ensure_dirs()
    
    # 加载所有配置文件
    config_files = sorted(CONFIG_DIR.glob("*.json"))
    if not config_files:
        print(f"[info] no 3rd-party config json files found in {CONFIG_DIR}")
        return 0
    
    all_meta: List[Dict[str, Any]] = []
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
    
    # 构建依赖图并计算构建顺序
    graph = build_dependency_graph(all_meta)
    try:
        order = topological_sort(graph)
    except RuntimeError as e:
        print(f"[error] {e}")
        return 1
    
    # 为快速查询创建映射
    name_to_meta = {m["name"]: m for m in all_meta}
    processed_meta: List[Dict[str, Any]] = []
    
    # 处理每个库
    for name in order:
        meta = name_to_meta.get(name)
        if not meta:
            continue
        
        print(f"[info] processing {name}")
        
        # 确定库类型
        lib_type = meta.get("type", "managed")
        need_compile = (lib_type == "managed" and meta.get("need_compile", True))
        
        install_base = INSTALL_DIR_BASE / name
        
        # 检查是否已安装（仅对 managed 库）
        if lib_type == "managed" and install_base.exists() and any(install_base.iterdir()):
            print(f"[info] {name} appears installed at {install_base}, skipping build")
            processed_meta.append(meta)
            continue
        
        # External 库无需构建
        if lib_type == "external":
            print(f"[info] {name} marked external, skipping pull/build")
            processed_meta.append(meta)
            continue
        
        # 处理 managed 库
        src_target = DEPS_DIR / name
        
        # 获取源代码信息
        source_info = meta.get("source") or {}
        git_url = source_info.get("git")
        branch = source_info.get("branch", "main")
        
        # 判断是否需要克隆
        need_pull = bool(meta.get("need_pull", False) or git_url)
        
        if need_pull:
            if not git_url:
                print(f"[error] {name} needs to be pulled but no git url specified")
                continue
            
            print(f"[info] cloning {name} -> {src_target}")
            if not clone_repo(name, git_url, branch, src_target):
                print(f"[error] git clone failed for {name}")
                continue
        else:
            # 如果不克隆，检查源代码是否存在
            if not src_target.exists():
                if not need_compile:
                    print(f"[info] {name} does not require compile and source not present. Skipping build; expect preinstalled files.")
                    processed_meta.append(meta)
                    continue
                
                print(f"[warn] {name} source not found at {src_target}, skipping build")
                continue
        
        # 执行 CMake 构建
        build_dir = BUILD_DIR / name
        install_dir = INSTALL_DIR_BASE / name
        
        print(f"[info] building {name} -> {install_dir}")
        ok = cmake_configure_and_build(src_target, build_dir, install_dir, meta)
        if not ok:
            print(f"[error] build/install failed for {name}")
            continue
        
        processed_meta.append(meta)
    
    # 生成 CMake find 模块
    # 根据 auto_generate_find_cmake 的值决定是否生成
    to_generate = []
    for m in processed_meta:
        ag = m.get("auto_generate_find_cmake", True)
        # 生成条件：true, "if_missing", 或未设置（默认为 true）
        if ag is True or ag == "if_missing" or (ag is not False and ag != "false"):
            to_generate.append(m)
    
    print(f"[info] generating {CMAKE_OUTPUT / 'Find3rdsGenerated.cmake'}")
    content = generate_find_cmake(to_generate)
    with open(CMAKE_OUTPUT / "Find3rdsGenerated.cmake", "w", encoding="utf-8") as f:
        f.write(content)
    
    print("[done] BuildDeps finished")
    return 0


if __name__ == "__main__":
    sys.exit(main())