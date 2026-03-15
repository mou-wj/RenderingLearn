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
#include <glslang/Public/ShaderLang.h>
#include "spirv_glsl.hpp"
#include "spirv_cross.hpp"
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "PathInfo.h"
#include "ShaderCompiledDataPacker.h"

namespace RenderCore {

struct SPIRVPackSource {
    std::vector<uint32_t> *spirvCode;
	spirv_cross::Compiler *compiler;
    ERHIShaderFrequency frequency;
    std::string entryPoint;
};


ShaderCompiler::ShaderCompiler()
{
    glslang::InitializeProcess();
}

ShaderCompiler::~ShaderCompiler()
{
}

bool ShaderCompiler::Initialize(const std::string& shaderSourceDir)
{
    if (shaderSourceDir == "") {
        ShaderSourceDirectory = Core::GetProjectDir() + "/shaders";
    }
    if (!std::filesystem::exists(shaderSourceDir))
    {
        return false;
    }
    ShaderSourceDirectory = shaderSourceDir;
    return true;
}

ShaderCompilationOutput ShaderCompiler::Compile(const ShaderCompileInput& input)
{
    ShaderCompilationOutput output;
    output.Platform = input.Platform;

    std::string source;
    std::vector<std::string> includedFiles;

    if (!PreprocessSource(input, source, includedFiles))
    {
        output.Success = false;
        output.ErrorMessage = "Failed to preprocess shader.";
        return output;
    }

#if defined(SHADER_DEBUG)
    output.PreprocessedSource = source;
#endif
    output.IncludedFiles = includedFiles;

    bool compiled = false;
    switch (input.Platform)
    {
    case ERHIShaderPlatform::Vulkan:
        CompileToSPIRV(source, input, output);
        break;
    default:
        output.ErrorMessage = "Platform not implemented!";
        compiled = false;
        break;
    }

    return output;
}

bool ShaderCompiler::LoadShaderSource(const ShaderCompileInput& input, std::string& outSource)
{
    // 先检查 Environment.VirtualIncludes
    auto it = input.Environment.VirtualIncludes.find(input.VirtualSourceFilePath);
    if (it != input.Environment.VirtualIncludes.end())
    {
        outSource = it->second;
        return true;
    }

    // 尝试从 ShaderSourceDirectory + VirtualSourceFilePath 读取
    std::string fullPath = ShaderSourceDirectory + "/" + input.VirtualSourceFilePath;
    std::ifstream file(fullPath, std::ios::in | std::ios::binary);
    if (!file.is_open())
        return false;

    std::ostringstream ss;
    ss << file.rdbuf();
    outSource = ss.str();
    return true;
}

bool ShaderCompiler::PreprocessSource(const ShaderCompileInput& input, std::string& outSource, std::vector<std::string>& outIncludedFiles)
{
    std::string src;
    // 1. 读取 shader 源
    if (!LoadShaderSource(input, src))
        return false;

    // 2. 使用 ExpandIncludes 展开所有 include
    std::set<std::string> includeStack; // 用于循环 include 检测
    if (!ExpandIncludes(src, input.Environment, outSource, outIncludedFiles, 16, &includeStack))
        return false;

    // 3. 应用宏定义
    ApplyMacros(outSource, input.Environment.Definitions);
    return true;
}

bool ShaderCompiler::ExpandIncludes(const std::string& source, const ShaderCompilerEnvironment& env, std::string& outExpanded, std::vector<std::string>& outIncludedFiles, int depth, std::set<std::string>* includeStack)
{
    bool localStack = false;
    if (!includeStack)
    {
        includeStack = new std::set<std::string>();
        localStack = true;
    }

    std::istringstream stream(source);
    std::ostringstream result;
    std::string line;

    while (std::getline(stream, line))
    {
        size_t includePos = line.find("#include");
        if (includePos != std::string::npos)
        {
            size_t startQuote = line.find_first_of("\"<", includePos + 8);
            size_t endQuote = line.find_first_of("\">", startQuote + 1);
            if (startQuote != std::string::npos && endQuote != std::string::npos)
            {
                std::string includePath = line.substr(startQuote + 1, endQuote - startQuote - 1);

                // 检查循环包含
                if (includeStack->count(includePath))
                    continue; // 已经包含过，跳过

                includeStack->insert(includePath);
                std::string includedSource;

                // 虚拟 include
                auto it = env.VirtualIncludes.find(includePath);
                if (it != env.VirtualIncludes.end())
                {
                    includedSource = it->second;
                }
                else
                {
                    // 文件系统 include
                    bool loaded = false;
                    for (const auto& incDir : env.IncludePaths)
                    {
                        std::string fullPath = incDir + "/" + includePath;
                        std::ifstream file(fullPath, std::ios::in | std::ios::binary);
                        if (file.is_open())
                        {
                            std::ostringstream ss;
                            ss << file.rdbuf();
                            includedSource = ss.str();
                            loaded = true;
                            break;
                        }
                    }
                    if (!loaded)
                        return false;
                }

                std::string expandedInclude;
                if (!ExpandIncludes(includedSource, env, expandedInclude, outIncludedFiles, depth + 1, includeStack))
                    return false;

                result << expandedInclude << "\n";
                outIncludedFiles.push_back(includePath);

                includeStack->erase(includePath);
                continue;
            }
        }

        result << line << "\n";
    }

    outExpanded = result.str();
    if (localStack)
        delete includeStack;

    return true;
}

void ShaderCompiler::ApplyMacros(std::string& source, const std::map<std::string, std::string>& macros)
{
    for (const auto& m : macros)
    {
        std::string define = "#define " + m.first + " " + m.second + "\n";
        source = define + source;
    }
}

void ShaderCompiler::CompileToSPIRV(const std::string& preprocessedSource, const ShaderCompileInput& input, ShaderCompilationOutput& out)
{
    out.Platform = ERHIShaderPlatform::Vulkan;
    out.Success = false;
    out.PackedBinaryData.clear();

    // 1. 映射 Shader Stage
    EShLanguage stage;
    switch (input.Frequency)
    {
    case ERHIShaderFrequency::Vertex:         stage = EShLangVertex; break;
    case ERHIShaderFrequency::Fragment:       stage = EShLangFragment; break;
    case ERHIShaderFrequency::Compute:        stage = EShLangCompute; break;
    case ERHIShaderFrequency::Geometry:       stage = EShLangGeometry; break;
    case ERHIShaderFrequency::TessControl:    stage = EShLangTessControl; break;
    case ERHIShaderFrequency::TessEvaluation: stage = EShLangTessEvaluation; break;
    case ERHIShaderFrequency::Mesh:           stage = EShLangMeshNV; break;
    case ERHIShaderFrequency::Task:           stage = EShLangTaskNV; break;
    default:
        out.ErrorMessage = "Unsupported shader stage for SPIR-V compilation";
        return;
    }

    // 2. 创建 TShader
    glslang::TShader shader(stage);
    const char* sourceCStr = preprocessedSource.c_str();
    shader.setStrings(&sourceCStr, 1);
    shader.setEntryPoint(input.EntryPoint.c_str());
    shader.setEnvInput(glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

    // 3. 添加宏定义
    TBuiltInResource resources = {};
    // 初始化默认资源限制
    resources.maxLights = 32;
    resources.maxClipPlanes = 6;
    resources.maxTextureUnits = 32;
    resources.maxVertexAttribs = 64;
    resources.maxVertexUniformComponents = 4096;
    resources.maxVaryingFloats = 64;
    resources.maxVertexTextureImageUnits = 32;
    resources.maxCombinedTextureImageUnits = 80;
    resources.maxTextureImageUnits = 32;
    resources.maxFragmentUniformComponents = 4096;
    resources.maxDrawBuffers = 32;
    resources.maxVertexUniformVectors = 128;
    resources.maxVaryingVectors = 8;
    resources.maxFragmentUniformVectors = 16;
    resources.maxVertexOutputVectors = 16;
    resources.maxFragmentInputVectors = 15;
    resources.minProgramTexelOffset = -8;
    resources.maxProgramTexelOffset = 7;
    resources.maxClipDistances = 8;
    resources.maxComputeWorkGroupCountX = 65535;
    resources.maxComputeWorkGroupCountY = 65535;
    resources.maxComputeWorkGroupCountZ = 65535;
    resources.maxComputeWorkGroupSizeX = 1024;
    resources.maxComputeWorkGroupSizeY = 1024;
    resources.maxComputeWorkGroupSizeZ = 64;
    resources.maxComputeUniformComponents = 1024;
    resources.maxComputeTextureImageUnits = 16;
    resources.maxComputeImageUniforms = 8;
    resources.maxComputeAtomicCounters = 8;
    resources.maxComputeAtomicCounterBuffers = 1;
    resources.maxVaryingComponents = 60;
    resources.maxVertexOutputComponents = 64;
    resources.maxGeometryInputComponents = 64;
    resources.maxGeometryOutputComponents = 128;
    resources.maxFragmentInputComponents = 128;
    resources.maxImageUnits = 8;
    resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
    resources.maxCombinedShaderOutputResources = 8;
    resources.maxImageSamples = 0;
    resources.maxVertexImageUniforms = 0;
    resources.maxTessControlImageUniforms = 0;
    resources.maxTessEvaluationImageUniforms = 0;
    resources.maxGeometryImageUniforms = 0;
    resources.maxFragmentImageUniforms = 8;
    resources.maxCombinedImageUniforms = 8;

    // 构建宏数组
    std::vector<const char*> preprocessorDefines;
    for (const auto& define : input.Environment.Definitions)
    {
        std::string def = define.first + "=" + define.second;
        char* defStr = new char[def.size() + 1];
        memcpy(defStr, def.c_str(), def.size() + 1);
        preprocessorDefines.push_back(defStr);
    }

    if (!shader.parse(&resources, 100, false, EShMsgDefault))
    {
        out.ErrorMessage = shader.getInfoLog();
        out.ErrorMessage += "\n";
        out.ErrorMessage += shader.getInfoDebugLog();
        // 清理
        for (auto p : preprocessorDefines) delete[] p;
        return;
    }
    for (auto p : preprocessorDefines) delete[] p;

    // 4. 链接程序
    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(EShMsgDefault))
    {
        out.ErrorMessage = program.getInfoLog();
        out.ErrorMessage += "\n";
        out.ErrorMessage += program.getInfoDebugLog();
        return;
    }

    // 5. 生成 SPIR-V
    std::vector<uint32_t> spirv;
    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

    

    // 6. 使用 SPIRV-Cross 反射资源
    spirv_cross::Compiler compiler(spirv);
    spirv_cross::ShaderResources resourcesSC = compiler.get_shader_resources();

    // ---------- Uniform Buffers ----------
    for (const auto& ub : resourcesSC.uniform_buffers)
    {
        std::string name = compiler.get_name(ub.id);

        uint32_t binding = compiler.get_decoration(ub.id, spv::DecorationBinding);
        uint32_t set = compiler.get_decoration(ub.id, spv::DecorationDescriptorSet);

        auto& type = compiler.get_type(ub.base_type_id);
        uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_size(type));

        out.ParameterMap.AddParameterAllocation(
            name,
            static_cast<uint16_t>(set),
            static_cast<uint16_t>(binding),
            static_cast<uint16_t>(size),
            EShaderParameterType::UniformBuffer
        );
    }

    // ---------- Sampled Images (Texture + Sampler) ----------
    for (const auto& img : resourcesSC.sampled_images)
    {
        std::string name = compiler.get_name(img.id);

        uint32_t binding = compiler.get_decoration(img.id, spv::DecorationBinding);
        uint32_t set = compiler.get_decoration(img.id, spv::DecorationDescriptorSet);

        out.ParameterMap.AddParameterAllocation(
            name,
            static_cast<uint16_t>(set),
            static_cast<uint16_t>(binding),
            0,
            EShaderParameterType::SRV
        );
    }

    // ---------- Separate Samplers ----------
    for (const auto& sampler : resourcesSC.separate_samplers)
    {
        std::string name = compiler.get_name(sampler.id);

        uint32_t binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
        uint32_t set = compiler.get_decoration(sampler.id, spv::DecorationDescriptorSet);

        out.ParameterMap.AddParameterAllocation(
            name,
            static_cast<uint16_t>(set),
            static_cast<uint16_t>(binding),
            0,
            EShaderParameterType::Sampler
        );
    }

    // ---------- Storage Images ----------
    for (const auto& img : resourcesSC.storage_images)
    {
        std::string name = compiler.get_name(img.id);

        uint32_t binding = compiler.get_decoration(img.id, spv::DecorationBinding);
        uint32_t set = compiler.get_decoration(img.id, spv::DecorationDescriptorSet);

        out.ParameterMap.AddParameterAllocation(
            name,
            static_cast<uint16_t>(set),
            static_cast<uint16_t>(binding),
            0,
            EShaderParameterType::UAV
        );
    }

    // ---------- Storage Buffers ----------
    for (const auto& sb : resourcesSC.storage_buffers)
    {
        std::string name = compiler.get_name(sb.id);

        uint32_t binding = compiler.get_decoration(sb.id, spv::DecorationBinding);
        uint32_t set = compiler.get_decoration(sb.id, spv::DecorationDescriptorSet);

        auto& type = compiler.get_type(sb.base_type_id);
        uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_size(type));

        out.ParameterMap.AddParameterAllocation(
            name,
            static_cast<uint16_t>(set),
            static_cast<uint16_t>(binding),
            static_cast<uint16_t>(size),
            EShaderParameterType::UAV
        );
    }
    // 6. 填充输出
    out.PackedBinaryData.resize(spirv.size() * sizeof(uint32_t));
    memcpy(out.PackedBinaryData.data(), spirv.data(), spirv.size() * sizeof(uint32_t));

	SPIRVPackSource packSource;
	packSource.compiler = &compiler;
	packSource.spirvCode = &spirv;
    packSource.frequency = input.Frequency;
    packSource.entryPoint = input.EntryPoint;
	SPIRVCompiledBinaryResultPacker packer;
	std::vector<char> packedData;

	if (packer.Pack(&packSource, packedData))
	{
		out.PackedBinaryData = std::move(packedData);
        out.Success = true;
	}


}

void ShaderCompiler::CompileToDirectX(const std::string& preprocessedSource, const ShaderCompileInput& input, ShaderCompilationOutput& out)
{

}

void ShaderCompiler::CompileToMetal(const std::string& preprocessedSource, const ShaderCompileInput& input, ShaderCompilationOutput& out)
{

}

void ShaderCompiler::CompileToOpenGL(const std::string& preprocessedSource, const ShaderCompileInput& input, ShaderCompilationOutput& out)
{

}












ShaderCompilationCache* GShaderCompilationCache = nullptr;
void SPIRVCompiledBinaryResultPacker::Depack(const std::vector<char>& packedResult)
{
    size_t offset = 0;

    auto read = [&](void* dst, size_t size)
        {
            memcpy(dst, packedResult.data() + offset, size);
            offset += size;
        };

    // =========================
    // SPIR-V code
    // =========================

    uint32_t codeSize = 0;
    read(&codeSize, sizeof(uint32_t));

    DepackedData.SpirvCode.resize(codeSize);

    read(DepackedData.SpirvCode.data(), codeSize * sizeof(uint32_t));

    // =========================
    // Header basic info
    // =========================

    read(&DepackedData.header.Frequency, sizeof(ERHIShaderFrequency));

    read(DepackedData.header.EntryPoint, sizeof(DepackedData.header.EntryPoint));

    read(&DepackedData.header.ShaderHash, sizeof(uint64_t));

    // =========================
    // Descriptor bindings
    // =========================

    uint32_t bindingCount = 0;
    read(&bindingCount, sizeof(uint32_t));

    DepackedData.header.DescriptorBindings.resize(bindingCount);

    for (uint32_t i = 0; i < bindingCount; ++i)
    {
        read(&DepackedData.header.DescriptorBindings[i], sizeof(DescriptorBindingInfo));
    }

    // =========================
    // Push constant
    // =========================

    read(&DepackedData.header.HasPushConstant, sizeof(bool));

    if (DepackedData.header.HasPushConstant)
    {
        read(&DepackedData.header.PushConstant, sizeof(PushConstantInfo));
    }
}
bool SPIRVCompiledBinaryResultPacker::Pack(void* packSource, std::vector<char>& packedResultOut)
{
    if (!packSource)
        return false;

    SPIRVPackSource* src = reinterpret_cast<SPIRVPackSource*>(packSource);

    DepackedData.SpirvCode = *(src->spirvCode);

    spirv_cross::Compiler* compiler = src->compiler;

    DepackedData.header.Frequency = src->frequency;

    strncpy(
        DepackedData.header.EntryPoint,
        src->entryPoint.c_str(),
        sizeof(DepackedData.header.EntryPoint));

    // 简单 hash
    DepackedData.header.ShaderHash =
        std::hash<std::string>()(
            std::string(
                reinterpret_cast<char*>(DepackedData.SpirvCode.data()),
                DepackedData.SpirvCode.size() * sizeof(uint32_t)));

    auto resources = compiler->get_shader_resources();

    DepackedData.header.DescriptorBindings.clear();


    auto addBinding =
        [&](const spirv_cross::Resource& res, ESPIRVShaderResourceType type)
        {
            DescriptorBindingInfo info{};

            info.Binding =
                (uint16_t)compiler->get_decoration(res.id, spv::DecorationBinding);

            info.Set =
                (uint16_t)compiler->get_decoration(res.id, spv::DecorationDescriptorSet);

            auto& typeInfo = compiler->get_type(res.type_id);

            if (!typeInfo.array.empty())
                info.Count = (uint16_t)typeInfo.array[0];
            else
                info.Count = 1;

            info.Type = type;

            std::string name = compiler->get_name(res.id);

            if (!name.empty())
            {
                strncpy(info.Name, name.c_str(), sizeof(info.Name));
            }

            DepackedData.header.DescriptorBindings.push_back(info);
        };

    // =========================
    // Uniform buffers
    // =========================

    for (auto& ub : resources.uniform_buffers)
    {
        addBinding(ub, ESPIRVShaderResourceType::UniformBuffer);
    }

    // =========================
    // Storage buffers
    // =========================

    for (auto& sb : resources.storage_buffers)
    {
        addBinding(sb, ESPIRVShaderResourceType::StorageBuffer);
    }

    // =========================
    // Sampled images
    // =========================

    for (auto& img : resources.sampled_images)
    {
        addBinding(img, ESPIRVShaderResourceType::SampledImage);
    }

    // =========================
    // Storage images
    // =========================

    for (auto& img : resources.storage_images)
    {
        addBinding(img, ESPIRVShaderResourceType::StorageImage);
    }

    // =========================
    // Push constants
    // =========================

    DepackedData.header.HasPushConstant = false;

    if (!resources.push_constant_buffers.empty())
    {
        auto& pcb = resources.push_constant_buffers[0];

        auto& type = compiler->get_type(pcb.base_type_id);

        DepackedData.header.PushConstant.Size =
            (uint16_t)compiler->get_declared_struct_size(type);
        DepackedData.header.HasPushConstant = true;
    }

    // =========================
    // Descriptor sort（非常重要）
    // =========================

    std::sort(
        DepackedData.header.DescriptorBindings.begin(),
        DepackedData.header.DescriptorBindings.end(),
        [](const DescriptorBindingInfo& a, const DescriptorBindingInfo& b)
        {
            if (a.Set != b.Set)
                return a.Set < b.Set;

            return a.Binding < b.Binding;
        });

    // =========================
    // Serialize
    // =========================

    packedResultOut.clear();

    auto write = [&](const void* data, size_t size)
        {
            const char* c = reinterpret_cast<const char*>(data);
            packedResultOut.insert(packedResultOut.end(), c, c + size);
        };

    // SPIRV

    uint32_t codeSize = (uint32_t)DepackedData.SpirvCode.size();

    write(&codeSize, sizeof(uint32_t));

    write(
        DepackedData.SpirvCode.data(),
        codeSize * sizeof(uint32_t));

    // Header basic

    write(&DepackedData.header.Frequency, sizeof(ERHIShaderFrequency));

    write(
        DepackedData.header.EntryPoint,
        sizeof(DepackedData.header.EntryPoint));

    write(&DepackedData.header.ShaderHash, sizeof(uint64_t));

    // Descriptor bindings

    uint32_t bindingCount =
        (uint32_t)DepackedData.header.DescriptorBindings.size();

    write(&bindingCount, sizeof(uint32_t));

    for (auto& b : DepackedData.header.DescriptorBindings)
    {
        write(&b, sizeof(DescriptorBindingInfo));
    }

    // Push constant

    write(&DepackedData.header.HasPushConstant, sizeof(bool));

    if (DepackedData.header.HasPushConstant)
    {
        write(&DepackedData.header.PushConstant, sizeof(PushConstantInfo));
    }

    return true;
}
} // namespace RenderCore