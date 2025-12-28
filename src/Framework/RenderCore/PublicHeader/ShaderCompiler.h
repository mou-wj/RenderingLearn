#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "Shader.h"
#include "RHIDefine.h"

namespace RenderCore {

// Shader compilation result for a specific platform
struct RENDERCORE_API ShaderCompilationResultPerPlatform
{
    RHI::ERHIShaderPlatform Platform = RHI::ERHIShaderPlatform::Unknown;
    bool Success = false;
    std::string ErrorMessage;
    std::vector<uint8_t> BinaryData;  // Platform-specific compiled binary
};

// Shader compilation result (multi-platform)
struct RENDERCORE_API ShaderCompilationResult
{
    bool Success = false;
    std::string ErrorMessage;
    std::string PreprocessedSource;            // Source after preprocessing
    ShaderParameterBindingInfo BindingInfo;    // Extracted parameter binding info

    // Platform-specific compilation results
    std::unordered_map<int, ShaderCompilationResultPerPlatform> PlatformResults;

    // Get compilation result for specific platform
    const ShaderCompilationResultPerPlatform* GetPlatformResult(RHI::ERHIShaderPlatform platform) const
    {
        auto it = PlatformResults.find(static_cast<int>(platform));
        if (it == PlatformResults.end()) return nullptr;
        return &it->second;
    }
};

// Shader source file information
struct RENDERCORE_API ShaderSourceInfo
{
    std::string SourcePath;
    RHI::ERHIShaderType ShaderType;
    std::unordered_map<std::string, std::string> MacroDefinitions;
    std::vector<RHI::ERHIShaderPlatform> TargetPlatforms;  // Platforms to compile for
};

// -------------------------------------------------------------------------------------------------
//  Shader Compiler
// -------------------------------------------------------------------------------------------------
class RENDERCORE_API ShaderCompiler
{
public:
    ShaderCompiler();
    ~ShaderCompiler();

    // Initialize the compiler (setup paths, environments)
    bool Initialize();

    // Compile a shader source file for multiple platforms
    ShaderCompilationResult CompileShader(const ShaderSourceInfo& sourceInfo);

    // Compile shader for a specific platform
    ShaderCompilationResultPerPlatform CompileShaderForPlatform(
        const std::string& preprocessedSource,
        RHI::ERHIShaderType shaderType,
        RHI::ERHIShaderPlatform platform
    );

    // Add a macro definition for preprocessing
    void AddMacroDefinition(const std::string& name, const std::string& value);

    // Clear all macro definitions
    void ClearMacroDefinitions();

    // Get the preprocessed shader source (without compilation)
    bool PreprocessShader(const std::string& sourcePath, std::string& outPreprocessedSource);

    // Extract shader parameter binding info from source
    bool ExtractParameterBindingInfo(
        const std::string& preprocessedSource,
        RHI::ERHIShaderType shaderType,
        ShaderParameterBindingInfo& outBindingInfo
    );

private:
    // Load shader source file
    bool LoadShaderSource(const std::string& sourcePath, std::string& outSource);

    // Preprocess shader source (expand includes, apply macros)
    bool PreprocessSource(const std::string& source, std::string& outProcessed);

    // Expand #include directives recursively
    bool ExpandIncludes(const std::string& source, std::string& outExpanded, int depth = 0);

    // Apply macro definitions
    void ApplyMacroDefinitions(std::string& source);

    // Compile HLSL to SPIR-V (for Vulkan)
    bool CompileHLSLToSPIRV(
        const std::string& preprocessedSource,
        RHI::ERHIShaderType shaderType,
        std::vector<uint8_t>& outBinaryData
    );

    // Compile HLSL to DirectX bytecode (D3D11/D3D12)
    bool CompileHLSLToDirectXBytecode(
        const std::string& preprocessedSource,
        RHI::ERHIShaderType shaderType,
        RHI::ERHIShaderPlatform platform,
        std::vector<uint8_t>& outBinaryData
    );

    // Compile HLSL to Metal bytecode (Metal)
    bool CompileHLSLToMetalBytecode(
        const std::string& preprocessedSource,
        RHI::ERHIShaderType shaderType,
        std::vector<uint8_t>& outBinaryData
    );

    // Compile GLSL to OpenGL bytecode
    bool CompileGLSLToOpenGLBytecode(
        const std::string& preprocessedSource,
        RHI::ERHIShaderType shaderType,
        std::vector<uint8_t>& outBinaryData
    );

    // Parse HLSL source for cbuffer/resource declarations
    bool ParseHLSLResources(
        const std::string& preprocessedSource,
        ShaderParameterBindingInfo& outBindingInfo
    );

    // Convert shader type enum to string
    std::string ShaderTypeToString(RHI::ERHIShaderType shaderType);

    // Convert shader type enum to HLSL target string (e.g., "vs_6_0")
    std::string ShaderTypeToHLSLTarget(RHI::ERHIShaderType shaderType);

    // Convert shader type to Metal entry point name
    std::string ShaderTypeToMetalEntry(RHI::ERHIShaderType shaderType);

    // Translate HLSL to GLSL if needed
    bool TranslateHLSLToGLSL(
        const std::string& hlslSource,
        RHI::ERHIShaderType shaderType,
        std::string& outGLSLSource
    );

private:
    std::unordered_map<std::string, std::string> MacroDefinitions;
    std::string ShaderSourceDirectory;
    int MaxIncludeDepth = 10;
};

} // namespace RenderCore