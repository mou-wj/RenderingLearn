#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <set>
#include <map>
#include <mutex>
#include "Shader.h"
#include "RHIDefine.h"

namespace RenderCore{
    // ============================================================
// Utility: Stable Hash Combine
// ============================================================

    inline void HashCombine(size_t& seed, size_t value)
    {
        seed ^= value + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2);
    }

    // ============================================================
    // Compile Flags
    // ============================================================

    enum class EShaderCompileFlags : uint32_t
    {
        None = 0,
        DebugInfo = 1 << 0,
        DisableOptimize = 1 << 1,
        WarningsAsError = 1 << 2,
    };

    inline EShaderCompileFlags operator|(EShaderCompileFlags a, EShaderCompileFlags b)
    {
        return static_cast<EShaderCompileFlags>(
            static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    // ============================================================
    // Shader Compiler Environment
    // ============================================================

    struct ShaderCompilerEnvironment
    {
        // 核心宏定义
        std::map<std::string, std::string> Definitions;

        // 虚拟 include 内容
        std::map<std::string, std::string> VirtualIncludes;

        // include 搜索路径
        std::vector<std::string> IncludePaths;

        // 可选移动平台全精度控制
        bool FullPrecisionInPS = false;

        void Merge(const ShaderCompilerEnvironment& other)
        {
            Definitions.insert(other.Definitions.begin(), other.Definitions.end());
            for (const auto& it : other.VirtualIncludes)
            {
                auto existing = VirtualIncludes.find(it.first);
                if (existing != VirtualIncludes.end())
                    existing->second.append(it.second);
                else
                    VirtualIncludes[it.first] = it.second;
            }
            IncludePaths.insert(IncludePaths.end(), other.IncludePaths.begin(), other.IncludePaths.end());
            FullPrecisionInPS |= other.FullPrecisionInPS;
        }
        bool operator==(const ShaderCompilerEnvironment& other) const
        {
            return Definitions == other.Definitions &&
                VirtualIncludes == other.VirtualIncludes &&
				IncludePaths == other.IncludePaths &&
				FullPrecisionInPS == other.FullPrecisionInPS;
        }
        void SetDefine(const std::string& name, const std::string& value) { Definitions[name] = value; }
        void SetDefine(const std::string& name, int32_t value) { Definitions[name] = std::to_string(value); }
        void SetDefine(const std::string& name, bool value) { Definitions[name] = value ? "1" : "0"; }
        void SetDefine(const std::string& name, float value) { Definitions[name] = std::to_string(value); }
    };

    // ============================================================
    // Shader Compile Input
    // ============================================================

    struct RENDERCORE_API ShaderCompileInput
    {
        // 原始文件路径（用于日志/调试）
        std::string VirtualSourceFilePath;

        std::string EntryPoint = "main";

        RHI::ERHIShaderFrequency Frequency;
        RHI::ERHIShaderPlatform Platform;

        // 包含宏定义 / include 搜索路径 / 虚拟 include
        ShaderCompilerEnvironment Environment;

        std::string TargetProfile;     // vs_6_6 / ps_6_6 等
        EShaderCompileFlags Flags = EShaderCompileFlags::None;

        // 是否属于 shader pipeline（可选）
        bool bCompilingForPipeline = false;

        bool operator==(const ShaderCompileInput& other) const
        {
            return VirtualSourceFilePath == other.VirtualSourceFilePath &&
                EntryPoint == other.EntryPoint &&
                Frequency == other.Frequency &&
                Platform == other.Platform &&
                Environment == other.Environment &&
                TargetProfile == other.TargetProfile &&
                Flags == other.Flags &&
                bCompilingForPipeline == other.bCompilingForPipeline;
        }
    };

    // ============================================================
    // Hash for ShaderCompileInput
    // ============================================================

} // namespace RenderCore

namespace std
{
    template<>
    struct hash<RenderCore::ShaderCompileInput>
    {
        size_t operator()(const RenderCore::ShaderCompileInput& input) const
        {
            using namespace RenderCore;
            size_t seed = 0;

            HashCombine(seed, std::hash<std::string>()(input.VirtualSourceFilePath));
            HashCombine(seed, std::hash<std::string>()(input.EntryPoint));
            HashCombine(seed, std::hash<int>()(static_cast<int>(input.Frequency)));
            HashCombine(seed, std::hash<int>()(static_cast<int>(input.Platform)));
            HashCombine(seed, std::hash<std::string>()(input.TargetProfile));
            HashCombine(seed, std::hash<uint32_t>()(static_cast<uint32_t>(input.Flags)));
            HashCombine(seed, std::hash<int>()(input.bCompilingForPipeline ? 1 : 0));

            for (const auto& m : input.Environment.Definitions)
            {
                HashCombine(seed, std::hash<std::string>()(m.first));
                HashCombine(seed, std::hash<std::string>()(m.second));
            }
            for (const auto& m : input.Environment.VirtualIncludes)
            {
                HashCombine(seed, std::hash<std::string>()(m.first));
                HashCombine(seed, std::hash<std::string>()(m.second));
            }
            for (const auto& path : input.Environment.IncludePaths)
            {
                HashCombine(seed, std::hash<std::string>()(path));
            }

            return seed;
        }
    };
}

namespace RenderCore
{

    // ============================================================
    // Shader Compilation Output
    // ============================================================

    struct RENDERCORE_API ShaderCompilationOutput
    {
        RHI::ERHIShaderPlatform Platform;

        bool Success = false;
        std::string ErrorMessage;

        std::vector<char> PackedBinaryData;
        ShaderParameterAllocationMap ParameterMap;

#if defined(SHADER_DEBUG)
        std::string PreprocessedSource;
#endif

        // 依赖文件（用于热重载）
        std::vector<std::string> IncludedFiles;
    };

    // ============================================================
    // Shader Compiler
    // ============================================================

    class RENDERCORE_API ShaderCompiler
    {
    public:
        ShaderCompiler();
        ~ShaderCompiler();

        bool Initialize(const std::string& shaderSourceDir);

        // 使用 ShaderCompileInput 进行完整编译
        ShaderCompilationOutput Compile(const ShaderCompileInput& input);

    private:
        bool LoadShaderSource(
            const ShaderCompileInput& input,
            std::string& outSource);

        bool PreprocessSource(
            const ShaderCompileInput& input,
            std::string& outSource,
            std::vector<std::string>& outIncludedFiles);

        bool ExpandIncludes(
            const std::string& source,
            const ShaderCompilerEnvironment& env,
            std::string& outExpanded,
            std::vector<std::string>& outIncludedFiles,
            int depth = 0,
            std::set<std::string>* includeStack = nullptr);

        void ApplyMacros(
            std::string& source,
            const std::map<std::string, std::string>& macros);

        void CompileToSPIRV(
            const std::string& preprocessedSource,
            const ShaderCompileInput& input,
            ShaderCompilationOutput& out);

        void CompileToDirectX(
            const std::string& preprocessedSource,
            const ShaderCompileInput& input,
            ShaderCompilationOutput& out);

        void CompileToMetal(
            const std::string& preprocessedSource,
            const ShaderCompileInput& input,
            ShaderCompilationOutput& out);

        void CompileToOpenGL(
            const std::string& preprocessedSource,
            const ShaderCompileInput& input,
            ShaderCompilationOutput& out);

    private:
        std::string ShaderSourceDirectory;
        int MaxIncludeDepth = 10;
    };

    // ============================================================
    // Thread-Safe Shader Compilation Cache
    // ============================================================

    class RENDERCORE_API ShaderCompilationCache
    {
    public:
        ShaderCompilationCache() = default;
        ~ShaderCompilationCache() = default;

        ShaderCompilationOutput GetOrCompile(
            ShaderCompiler& compiler,
            const ShaderCompileInput& input)
        {
            std::lock_guard<std::mutex> lock(CacheMutex);

            auto it = Cache.find(input);
            if (it != Cache.end())
            {
                return it->second;
            }

            ShaderCompilationOutput output = compiler.Compile(input);
            Cache.emplace(input, output);

            return output;
        }

        const ShaderCompilationOutput* Find(
            const ShaderCompileInput& input) const
        {
            std::lock_guard<std::mutex> lock(CacheMutex);

            auto it = Cache.find(input);
            if (it != Cache.end())
                return &it->second;

            return nullptr;
        }

        void Clear()
        {
            std::lock_guard<std::mutex> lock(CacheMutex);
            Cache.clear();
        }

    private:
        mutable std::mutex CacheMutex;
        std::unordered_map<ShaderCompileInput, ShaderCompilationOutput> Cache;
    };

    extern RENDERCORE_API ShaderCompilationCache* GShaderCompilationCache;



} // namespace RenderCore