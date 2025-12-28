#include "ShaderCompiler.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <algorithm>

// Include platform-specific compilation libraries
// For DXC (DirectX Shader Compiler)
// #include <dxcapi.h>

// For glslang
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "PathInfo.h"

namespace RenderCore {

ShaderCompiler::ShaderCompiler()
{
    ShaderSourceDirectory = Core::GetProjectDir() + "/shaders";
}

ShaderCompiler::~ShaderCompiler()
{
}

bool ShaderCompiler::Initialize()
{
    // Verify shader source directory exists
    if (!std::filesystem::exists(ShaderSourceDirectory))
    {
        return false;
    }

    // Initialize glslang if needed
    glslang::InitializeProcess();
    return true;
}

ShaderCompilationResult ShaderCompiler::CompileShader(const ShaderSourceInfo& sourceInfo)
{
    ShaderCompilationResult result;

    // Load source file
    std::string source;
    if (!LoadShaderSource(sourceInfo.SourcePath, source))
    {
        result.Success = false;
        result.ErrorMessage = "Failed to load shader source: " + sourceInfo.SourcePath;
        return result;
    }

    // Preprocess source
    std::string preprocessedSource;
    if (!PreprocessSource(source, preprocessedSource))
    {
        result.Success = false;
        result.ErrorMessage = "Failed to preprocess shader source";
        return result;
    }
    result.PreprocessedSource = preprocessedSource;

    // Extract parameter binding info
    if (!ExtractParameterBindingInfo(preprocessedSource, sourceInfo.ShaderType, result.BindingInfo))
    {
        result.Success = false;
        result.ErrorMessage = "Failed to extract parameter binding info";
        return result;
    }

    // Compile for each target platform
    std::vector<RHI::ERHIShaderPlatform> platforms = sourceInfo.TargetPlatforms;
    if (platforms.empty())
    {
        // Default to Vulkan if no platforms specified
        platforms.push_back(RHI::ERHIShaderPlatform::Vulkan);
    }

    bool allSuccess = true;
    for (auto platform : platforms)
    {
        auto platformResult = CompileShaderForPlatform(preprocessedSource, sourceInfo.ShaderType, platform);
        result.PlatformResults[static_cast<int>(platform)] = platformResult;
        
        if (!platformResult.Success)
        {
            allSuccess = false;
        }
    }

    result.Success = allSuccess;
    return result;
}

ShaderCompilationResultPerPlatform ShaderCompiler::CompileShaderForPlatform(
    const std::string& preprocessedSource,
    RHI::ERHIShaderType shaderType,
    RHI::ERHIShaderPlatform platform)
{
    ShaderCompilationResultPerPlatform result;
    result.Platform = platform;

    switch (platform)
    {
    case RHI::ERHIShaderPlatform::Vulkan:
        if (CompileHLSLToSPIRV(preprocessedSource, shaderType, result.BinaryData))
        {
            result.Success = true;
        }
        else
        {
            result.Success = false;
            result.ErrorMessage = "Failed to compile HLSL to SPIR-V for Vulkan";
        }
        break;

    case RHI::ERHIShaderPlatform::D3D11:
    case RHI::ERHIShaderPlatform::D3D12:
        if (CompileHLSLToDirectXBytecode(preprocessedSource, shaderType, platform, result.BinaryData))
        {
            result.Success = true;
        }
        else
        {
            result.Success = false;
            result.ErrorMessage = "Failed to compile HLSL to DirectX bytecode";
        }
        break;

    case RHI::ERHIShaderPlatform::Metal:
        if (CompileHLSLToMetalBytecode(preprocessedSource, shaderType, result.BinaryData))
        {
            result.Success = true;
        }
        else
        {
            result.Success = false;
            result.ErrorMessage = "Failed to compile HLSL to Metal bytecode";
        }
        break;

    case RHI::ERHIShaderPlatform::OpenGL:
        if (CompileGLSLToOpenGLBytecode(preprocessedSource, shaderType, result.BinaryData))
        {
            result.Success = true;
        }
        else
        {
            result.Success = false;
            result.ErrorMessage = "Failed to compile GLSL to OpenGL bytecode";
        }
        break;

    default:
        result.Success = false;
        result.ErrorMessage = "Unsupported shader platform";
        break;
    }

    return result;
}

void ShaderCompiler::AddMacroDefinition(const std::string& name, const std::string& value)
{
    MacroDefinitions[name] = value;
}

void ShaderCompiler::ClearMacroDefinitions()
{
    MacroDefinitions.clear();
}

bool ShaderCompiler::PreprocessShader(const std::string& sourcePath, std::string& outPreprocessedSource)
{
    std::string source;
    if (!LoadShaderSource(sourcePath, source))
    {
        return false;
    }
    return PreprocessSource(source, outPreprocessedSource);
}

bool ShaderCompiler::ExtractParameterBindingInfo(
    const std::string& preprocessedSource,
    RHI::ERHIShaderType shaderType,
    ShaderParameterBindingInfo& outBindingInfo)
{
    return ParseHLSLResources(preprocessedSource, outBindingInfo);
}

bool ShaderCompiler::LoadShaderSource(const std::string& sourcePath, std::string& outSource)
{
    std::ifstream file(sourcePath);
    if (!file.is_open())
    {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    outSource = buffer.str();
    file.close();
    return true;
}

bool ShaderCompiler::PreprocessSource(const std::string& source, std::string& outProcessed)
{
    // Expand includes
    if (!ExpandIncludes(source, outProcessed))
    {
        return false;
    }

    // Apply macro definitions
    ApplyMacroDefinitions(outProcessed);

    return true;
}

bool ShaderCompiler::ExpandIncludes(const std::string& source, std::string& outExpanded, int depth)
{
    if (depth > MaxIncludeDepth)
    {
        return false; // Prevent infinite recursion
    }

    std::string result = source;
    std::regex includeRegex(R"(#include\s+[\"<]([^\">]+)[\">])");
    std::smatch match;
    std::string::const_iterator searchStart(result.cbegin());

    while (std::regex_search(searchStart, result.cend(), match, includeRegex))
    {
        std::string includePath = match[1].str();
        std::string fullPath = ShaderSourceDirectory + "/" + includePath;

        std::string includedSource;
        if (!LoadShaderSource(fullPath, includedSource))
        {
            return false;
        }

        // Recursively expand includes in the included file
        std::string expandedInclude;
        if (!ExpandIncludes(includedSource, expandedInclude, depth + 1))
        {
            return false;
        }

        // Replace #include directive with expanded content
        std::string matchStr = match[0].str();
        size_t pos = result.find(matchStr);
        result.replace(pos, matchStr.length(), expandedInclude);

        searchStart = result.cbegin() + pos + expandedInclude.length();
    }

    outExpanded = result;
    return true;
}

void ShaderCompiler::ApplyMacroDefinitions(std::string& source)
{
    for (const auto& macro : MacroDefinitions)
    {
        std::regex macroRegex("\\b" + macro.first + "\\b");
        source = std::regex_replace(source, macroRegex, macro.second);
    }
}

bool ShaderCompiler::CompileHLSLToSPIRV(
    const std::string& preprocessedSource,
    RHI::ERHIShaderType shaderType,
    std::vector<uint8_t>& outBinaryData)
{
    // Using DXC (DirectX Shader Compiler) to compile HLSL -> SPIR-V
    // This requires DXC library with SPIR-V support
    
    // Pseudocode - actual implementation would depend on DXC API
    /*
    IDxcLibrary* library = nullptr;
    IDxcCompiler* compiler = nullptr;
    IDxcOperationResult* result = nullptr;
    
    DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
    
    IDxcBlobEncoding* sourceBlob = nullptr;
    library->CreateBlobWithEncodingOnHeapCopy(
        preprocessedSource.c_str(),
        preprocessedSource.length(),
        CP_UTF8,
        &sourceBlob
    );
    
    LPCWSTR arguments[] = {
        L"-E", L"main",
        L"-T", ShaderTypeToHLSLTarget(shaderType).c_str(),
        L"-spirv"
    };
    
    compiler->Compile(sourceBlob, L"", L"main", 
        ShaderTypeToHLSLTarget(shaderType).c_str(), 
        arguments, _countof(arguments), nullptr, 0, nullptr, &result);
    
    HRESULT hr;
    result->GetStatus(&hr);
    if (SUCCEEDED(hr)) {
        IDxcBlob* code = nullptr;
        result->GetResult(&code);
        outBinaryData.resize(code->GetBufferSize());
        memcpy(outBinaryData.data(), code->GetBufferPointer(), code->GetBufferSize());
        return true;
    }
    */

    // Placeholder: return empty for now
    outBinaryData.clear();
    return true;
}

bool ShaderCompiler::CompileHLSLToDirectXBytecode(
    const std::string& preprocessedSource,
    RHI::ERHIShaderType shaderType,
    RHI::ERHIShaderPlatform platform,
    std::vector<uint8_t>& outBinaryData)
{
    // Using D3DCompile or DXC to compile HLSL bytecode
    // Pseudocode - actual implementation would use D3DCompile or DXC API
    
    /*
    IDxcLibrary* library = nullptr;
    IDxcCompiler* compiler = nullptr;
    IDxcOperationResult* result = nullptr;
    
    DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
    
    IDxcBlobEncoding* sourceBlob = nullptr;
    library->CreateBlobWithEncodingOnHeapCopy(
        preprocessedSource.c_str(),
        preprocessedSource.length(),
        CP_UTF8,
        &sourceBlob
    );
    
    LPCWSTR targetProfile = (platform == RHI::ERHIShaderPlatform::D3D12) ? L"6_0" : L"5_0";
    LPCWSTR arguments[] = {
        L"-E", L"main",
        L"-T", ShaderTypeToHLSLTarget(shaderType).c_str(),
    };
    
    compiler->Compile(sourceBlob, L"", L"main",
        ShaderTypeToHLSLTarget(shaderType).c_str(),
        arguments, _countof(arguments), nullptr, 0, nullptr, &result);
    
    HRESULT hr;
    result->GetStatus(&hr);
    if (SUCCEEDED(hr)) {
        IDxcBlob* code = nullptr;
        result->GetResult(&code);
        outBinaryData.resize(code->GetBufferSize());
        memcpy(outBinaryData.data(), code->GetBufferPointer(), code->GetBufferSize());
        return true;
    }
    */

    // Placeholder: return empty for now
    outBinaryData.clear();
    return true;
}

bool ShaderCompiler::CompileHLSLToMetalBytecode(
    const std::string& preprocessedSource,
    RHI::ERHIShaderType shaderType,
    std::vector<uint8_t>& outBinaryData)
{
    // Metal compilation would require translating HLSL -> Metal Shading Language (MSL)
    // Then compiling with Metal compiler (metallib)
    
    // Placeholder: return empty for now
    outBinaryData.clear();
    return true;
}

bool ShaderCompiler::CompileGLSLToOpenGLBytecode(
    const std::string& preprocessedSource,
    RHI::ERHIShaderType shaderType,
    std::vector<uint8_t>& outBinaryData)
{
    // For OpenGL, we typically store GLSL source as-is or compile to SPIR-V
    // OpenGL 4.6+ supports SPIR-V
    
    // First translate HLSL to GLSL
    std::string glslSource;
    if (!TranslateHLSLToGLSL(preprocessedSource, shaderType, glslSource))
    {
        return false;
    }

    // Store GLSL source as binary (or could compile to SPIR-V for OpenGL 4.6+)
    outBinaryData.assign(glslSource.begin(), glslSource.end());
    return true;
}

bool ShaderCompiler::ParseHLSLResources(
    const std::string& preprocessedSource,
    ShaderParameterBindingInfo& outBindingInfo)
{
    // Parse cbuffer declarations
    std::regex cbufferRegex(
        R"(cbuffer\s+(\w+)\s*:\s*register\s*\(\s*b(\d+)\s*\)\s*\{([^}]*)\})"
    );
    std::smatch match;
    std::string::const_iterator searchStart(preprocessedSource.cbegin());

    while (std::regex_search(searchStart, preprocessedSource.cend(), match, cbufferRegex))
    {
        std::string cbufferName = match[1].str();
        uint32_t bindSlot = std::stoul(match[2].str());
        std::string cbufferContent = match[3].str();

        // Parse member variables in cbuffer
        std::regex memberRegex(R"((\w+)\s+(\w+)\s*;)");
        std::smatch memberMatch;
        std::string::const_iterator memberSearchStart(cbufferContent.cbegin());

        while (std::regex_search(memberSearchStart, cbufferContent.cend(), memberMatch, memberRegex))
        {
            std::string typeName = memberMatch[1].str();
            std::string memberName = memberMatch[2].str();

            ShaderParameterBinding binding;
            binding.BindSlot = bindSlot;

            // Map HLSL types to parameter base types
            if (typeName == "float")
                binding.ParameterBaseType = RHI::EShaderUniformBaseType::Float32;
            else if (typeName == "int")
                binding.ParameterBaseType = RHI::EShaderUniformBaseType::Int32;
            else if (typeName == "uint")
                binding.ParameterBaseType = RHI::EShaderUniformBaseType::UInt32;
            else
                binding.ParameterBaseType = RHI::EShaderUniformBaseType::Unknown;

            outBindingInfo.AddParameterBinding(memberName, binding);

            memberSearchStart = memberMatch.suffix().first;
        }

        searchStart = match.suffix().first;
    }

    // Parse texture/sampler declarations
    std::regex textureRegex(
        R"(Texture2D\s+(\w+)\s*:\s*register\s*\(\s*t(\d+)\s*\))"
    );
    searchStart = preprocessedSource.cbegin();

    while (std::regex_search(searchStart, preprocessedSource.cend(), match, textureRegex))
    {
        std::string textureName = match[1].str();
        uint32_t bindSlot = std::stoul(match[2].str());

        ShaderParameterBinding binding;
        binding.BindSlot = bindSlot;
        binding.ParameterBaseType = RHI::EShaderUniformBaseType::Texture;

        outBindingInfo.AddParameterBinding(textureName, binding);
        searchStart = match.suffix().first;
    }

    // Parse RWTexture/UAV declarations
    std::regex uavRegex(
        R"(RWTexture2D\s*<\s*\w+\s*>\s+(\w+)\s*:\s*register\s*\(\s*u(\d+)\s*\))"
    );
    searchStart = preprocessedSource.cbegin();

    while (std::regex_search(searchStart, preprocessedSource.cend(), match, uavRegex))
    {
        std::string uavName = match[1].str();
        uint32_t bindSlot = std::stoul(match[2].str());

        ShaderParameterBinding binding;
        binding.BindSlot = bindSlot;
        binding.ParameterBaseType = RHI::EShaderUniformBaseType::Texture_UAV;

        outBindingInfo.AddParameterBinding(uavName, binding);
        searchStart = match.suffix().first;
    }

    return true;
}

std::string ShaderCompiler::ShaderTypeToString(RHI::ERHIShaderType shaderType)
{
    switch (shaderType)
    {
    case RHI::ERHIShaderType::Vertex:
        return "vertex";
    case RHI::ERHIShaderType::Fragment:
        return "fragment";
    case RHI::ERHIShaderType::Compute:
        return "compute";
    case RHI::ERHIShaderType::Geometry:
        return "geometry";
    case RHI::ERHIShaderType::TessControl:
        return "tesscontrol";
    case RHI::ERHIShaderType::TessEvaluation:
        return "tesseval";
    default:
        return "unknown";
    }
}

std::string ShaderCompiler::ShaderTypeToHLSLTarget(RHI::ERHIShaderType shaderType)
{
    switch (shaderType)
    {
    case RHI::ERHIShaderType::Vertex:
        return "vs_6_0";
    case RHI::ERHIShaderType::Fragment:
        return "ps_6_0";
    case RHI::ERHIShaderType::Compute:
        return "cs_6_0";
    case RHI::ERHIShaderType::Geometry:
        return "gs_6_0";
    case RHI::ERHIShaderType::TessControl:
        return "hs_6_0";
    case RHI::ERHIShaderType::TessEvaluation:
        return "ds_6_0";
    default:
        return "unknown";
    }
}

std::string ShaderCompiler::ShaderTypeToMetalEntry(RHI::ERHIShaderType shaderType)
{
    switch (shaderType)
    {
    case RHI::ERHIShaderType::Vertex:
        return "vertexShader";
    case RHI::ERHIShaderType::Fragment:
        return "fragmentShader";
    case RHI::ERHIShaderType::Compute:
        return "computeShader";
    default:
        return "unknown";
    }
}

bool ShaderCompiler::TranslateHLSLToGLSL(
    const std::string& hlslSource,
    RHI::ERHIShaderType shaderType,
    std::string& outGLSLSource)
{
    // This would require HLSL-to-GLSL translation
    // Could use ANGLE or similar translation layer
    // For now, this is a placeholder that returns the source as-is
    outGLSLSource = hlslSource;
    return true;
}

} // namespace RenderCore