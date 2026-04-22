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
#include "spirv_hlsl.hpp"
#include "PathInfo.h"
#include "ShaderCompiledDataPacker.h"

namespace RenderCore {

    class ShaderVirtualFileSystem {
    public:
        // 1. 注册虚拟文件（例如由 Generator 生成的参数代码）
        void RegisterVirtualFile(const std::string& VirtualPath, std::string Content) {
            VirtualFiles[VirtualPath] = std::move(Content);
        }

        // 2. 注册物理路径映射（将 /Engine/ 映射到磁盘实际路径）
        void RegisterMountPoint(const std::string& VirtualDir, const std::string& PhysicalDir) {
            MountPoints[VirtualDir] = PhysicalDir;
        }

        // 3. 核心接口：获取文件内容（供编译器后端调用）
        std::optional<std::string> GetFileContent(const std::string& Path) const {
            // A. 先检查是否是内存中的虚拟文件
            auto it = VirtualFiles.find(Path);
            if (it != VirtualFiles.end()) {
                return it->second;
            }

            // B. 检查物理挂载点
            for (const auto& [VirtualPrefix, PhysicalBase] : MountPoints) {
                if (Path.find(VirtualPrefix) == 0) {
                    std::string RelativePath = Path.substr(VirtualPrefix.length());
                    std::string FullPhysicalPath = PhysicalBase + "/" + RelativePath;
                    return ReadPhysicalFile(FullPhysicalPath);
                }
            }

            return std::nullopt; // 未找到
        }


    private:
        std::map<std::string, std::string> VirtualFiles;
        std::map<std::string, std::string> MountPoints;

        std::optional<std::string> ReadPhysicalFile(const std::string& FullPath) const {
            std::ifstream File(FullPath);
            if (!File.is_open()) return std::nullopt;
            return std::string((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
        }
    };
    ShaderVirtualFileSystem GShaderVirtualFileSystem;

    class ShaderParameterSFGenerator {
    public:
        // 统一定义虚拟路径的前缀
        static std::string GetVirtualPath(const ShaderParametersMetadata& root) {
            return std::string("/Engine/ShaderParameters/") + root.GetStructName() + ".sf";
        }

        // 用户调用的接口：只返回字符串
        std::string GenerateOrGetShaderParameterMetaDataSF(const ShaderParametersMetadata& root) {
            std::string VirtualPath = GetVirtualPath(root);
            // 1. 检查全局虚拟文件系统是否已有缓存
            auto CachedContent = GShaderVirtualFileSystem.GetFileContent(VirtualPath);
            if (CachedContent.has_value()) {
                return CachedContent.value();
            }

            // 2. 准备编译上下文
            std::stringstream FinalCode;
            std::unordered_set<std::string> DefinedTypes;
            uint32_t TextureSlot = 0;
            uint32_t SamplerSlot = 0;
            uint32_t UavSlot = 0;

            // 添加文件头注释，方便调试查看生成来源
            FinalCode << "// Generated for " << root.GetStructName() << "\n\n";

            // 3. 执行递归生成
            RecursiveProcess(root, "", FinalCode, DefinedTypes, TextureSlot, SamplerSlot, UavSlot);

            // 4. 存入全局虚拟文件系统并返回结果
            std::string Result = FinalCode.str();
            GShaderVirtualFileSystem.RegisterVirtualFile(VirtualPath, Result);

            return Result;
        }

    private:

        void RecursiveProcess(
            const ShaderParametersMetadata& Metadata,
            std::string PathPrefix,
            std::stringstream& OutCode,
            std::unordered_set<std::string>& DefinedTypes,
            uint32_t& TSlot, uint32_t& SSlot, uint32_t& USlot)
        {
            // --- 步骤 A: 确保类型定义 (Type Definitions) ---
            // 先处理所有嵌套结构体的类型声明，且全局只声明一次
            for (const auto& Member : Metadata.GetMembers()) {
                if (Member.IsStruct() && Member.StructMetadata) {
                    if (DefinedTypes.find(Member.StructMetadata->GetStructName()) == DefinedTypes.end()) {
                        // 递归声明子结构体类型（此时不传路径，因为是类型定义）
                        RecursiveProcess(*Member.StructMetadata, "", OutCode, DefinedTypes, TSlot, SSlot, USlot);
                    }
                }
            }

            // 生成当前结构体的 struct 定义
            if (DefinedTypes.find(Metadata.GetStructName()) == DefinedTypes.end()) {
                OutCode << "struct " << Metadata.GetStructName() << "\n{\n";
                for (const auto& Member : Metadata.GetMembers()) {
                    if (Member.IsResource()) continue; // struct 内部不放资源

                    std::string TypeName = Member.IsStruct() ?
                        Member.StructMetadata->GetStructName() : MapBaseType(Member.BaseType);

                    OutCode << "    " << TypeName << " " << Member.Name << ";\n";
                }
                OutCode << "};\n\n";
                DefinedTypes.insert(Metadata.GetStructName());
            }

            // --- 步骤 B: 资源绑定平铺 (Resource Flattening) ---
            // 只有当 PathPrefix 不为空时，才表示我们在处理某个具体变量的资源展开
            // 如果是顶层调用，我们也需要遍历其成员展开资源
            for (const auto& Member : Metadata.GetMembers()) {
                std::string FullName = PathPrefix.empty() ? Member.Name : PathPrefix + "_" + Member.Name;

                if (Member.IsResource()) {
                    // 根据类型分配寄存器
                    std::string Reg;
                    if (Member.BaseType == EShaderUniformBaseType::Texture || Member.BaseType == EShaderUniformBaseType::Texture_SRV) {
                        Reg = "t" + std::to_string(TSlot++);
                    }
                    else if (Member.BaseType == EShaderUniformBaseType::Sampler) {
                        Reg = "s" + std::to_string(SSlot++);
                    }
                    else if (Member.BaseType == EShaderUniformBaseType::Texture_UAV) {
                        Reg = "u" + std::to_string(USlot++);
                    }

                    OutCode << MapBaseType(Member.BaseType) << " " << FullName << " : register(" << Reg << ");\n";
                }
                else if (Member.IsStruct()) {
                    // 如果是嵌套结构体，继续向下探测其内部是否有资源需要展开
                    RecursiveProcess(*Member.StructMetadata, FullName, OutCode, DefinedTypes, TSlot, SSlot, USlot);
                }
            }
        }

        std::string MapBaseType(EShaderUniformBaseType type) {
            switch (type) {
            case EShaderUniformBaseType::Float32:     return "float";
            case EShaderUniformBaseType::Int32:       return "int";
            case EShaderUniformBaseType::UInt32:      return "uint";
            case EShaderUniformBaseType::Bool:        return "bool";
            case EShaderUniformBaseType::Texture:     return "Texture2D";
            case EShaderUniformBaseType::Texture_SRV: return "Texture2D";
            case EShaderUniformBaseType::Texture_UAV: return "RWTexture2D<float4>";
            case EShaderUniformBaseType::Sampler:     return "SamplerState";
            case EShaderUniformBaseType::Buffer_SRV:  return "Buffer<float4>";
            case EShaderUniformBaseType::Buffer_UAV:  return "RWBuffer<float4>";
            default: return "float";
            }
        }
    };

    ShaderParameterSFGenerator GShaderParameterSFGenerator;
struct SPIRVPackSource {
    std::vector<uint32_t> *spirvCode;
	spirv_cross::Compiler *compiler;
    ERHIShaderFrequency frequency;
    std::string entryPoint;
    int globalUniformBufferBinding = -1;
    int globalUniformBufferSet = -1;
};

std::string ShaderCompiler::ShaderSourceDirectory = "";

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


std::string ShaderCompiler::GenerateOrGetShaderPrameterMetaDataSF(const ShaderParametersMetadata& root)
{
    return GShaderParameterSFGenerator.GenerateOrGetShaderParameterMetaDataSF(root);
}


std::optional<std::string> ShaderCompiler::GetFileContent(const std::string& Path)
{
    return GShaderVirtualFileSystem.GetFileContent(Path);
}

bool ShaderCompiler::LoadShaderSource(const ShaderCompileInput& input, std::string& outSource)
{
    // �ȼ�� Environment.VirtualIncludes
    auto it = input.Environment.VirtualIncludes.find(input.VirtualSourceFilePath);
    if (it != input.Environment.VirtualIncludes.end())
    {
        outSource = it->second;
        return true;
    }

    // ���Դ� ShaderSourceDirectory + VirtualSourceFilePath ��ȡ
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
    // 1. ��ȡ shader Դ
    if (!LoadShaderSource(input, src))
        return false;

    // 2. ʹ�� ExpandIncludes չ������ include
    std::set<std::string> includeStack; // ����ѭ�� include ���
    if (!ExpandIncludes(src, input.Environment, outSource, outIncludedFiles, 16, &includeStack))
        return false;

    // 3. Ӧ�ú궨��
    ApplyMacros(outSource, input.Environment.Definitions);
    return true;
}

bool ShaderCompiler::ExpandIncludes(const std::string& source, const ShaderCompilerEnvironment& env, std::string& outExpanded, std::vector<std::string>& outIncludedFiles, int depth, std::set<std::string>* includeStack)
{
    // 1. 深度限制，防止恶意递归
    if (depth > 32) return false;

    bool bIsRoot = (includeStack == nullptr);
    std::set<std::string> localStack;
    if (bIsRoot) includeStack = &localStack;

    std::istringstream stream(source);
    std::ostringstream result;
    std::string line;

    while (std::getline(stream, line))
    {
        // 匹配 #include "..." 或 #include <...>
        // 建议增加对宏定义 include 的支持处理（可选）
        size_t includePos = line.find("#include");
        if (includePos != std::string::npos)
        {
            size_t startQuote = line.find_first_of("\"<", includePos + 8);
            size_t endQuote = line.find_first_of("\">", startQuote + 1);

            if (startQuote != std::string::npos && endQuote != std::string::npos)
            {
                std::string includePath = line.substr(startQuote + 1, endQuote - startQuote - 1);

                // 防止循环引用
                if (includeStack->count(includePath)) continue;

                std::string includedSource;
                bool bFound = false;

                // --- 优先级 A: 从环境私有虚拟内容中查找 ---
                auto it = env.VirtualIncludes.find(includePath);
                if (it != env.VirtualIncludes.end())
                {
                    includedSource = it->second;
                    bFound = true;
                }

                // --- 优先级 B: 从全局虚拟文件系统 (GShaderVirtualFileSystem) 查找 ---
                // 这一点很重要，因为自动生成的 .sf 文件注册在这里
                if (!bFound)
                {
                    auto vfsContent = GShaderVirtualFileSystem.GetFileContent(includePath);
                    if (vfsContent.has_value())
                    {
                        includedSource = vfsContent.value();
                        bFound = true;
                    }
                }

                // --- 优先级 C: 磁盘文件系统 ---
                if (!bFound)
                {
                    for (const auto& incDir : env.IncludePaths)
                    {
                        std::string fullPath = incDir + "/" + includePath;
                        std::ifstream file(fullPath, std::ios::in | std::ios::binary);
                        if (file.is_open())
                        {
                            std::ostringstream ss;
                            ss << file.rdbuf();
                            includedSource = ss.str();
                            bFound = true;
                            break;
                        }
                    }
                }

                if (!bFound)
                {
                    // 记录未找到的文件日志...
                    return false;
                }

                // 递归展开
                includeStack->insert(includePath);
                std::string expandedInclude;
                if (!ExpandIncludes(includedSource, env, expandedInclude, outIncludedFiles, depth + 1, includeStack))
                {
                    return false;
                }

                result << "// Start Include: " << includePath << "\n";
                result << expandedInclude << "\n";
                result << "// End Include: " << includePath << "\n";

                outIncludedFiles.push_back(includePath);
                includeStack->erase(includePath);
                continue;
            }
        }

        result << line << "\n";
    }

    outExpanded = result.str();
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

    // 1. ӳ�� Shader Stage
    EShLanguage stage;
    std::string setId = "0";
    switch (input.Frequency)
    {
        // --- ��� A: �������еļ�����ɫ�� ---
    case ERHIShaderFrequency::Compute:
        stage = EShLangCompute;
        setId = "0"; // ��Զ�� 0 ��ʼ
        break;

        // --- ��� B: ��׼ͼ�ι��� (ͨ�� Set 0 Ϊ VS, Set 1 Ϊ PS) ---
    case ERHIShaderFrequency::Vertex:         stage = EShLangVertex;    setId = "0"; break;
    case ERHIShaderFrequency::Fragment:       stage = EShLangFragment;  setId = "1"; break;
    case ERHIShaderFrequency::Geometry:       stage = EShLangGeometry;  setId = "2"; break;
        // ���δ���ͨ���� VS ���ܽ�ϣ����Ը�������΢��
    case ERHIShaderFrequency::TessControl:    stage = EShLangTessControl;    setId = "3"; break;
    case ERHIShaderFrequency::TessEvaluation: stage = EShLangTessEvaluation; setId = "4"; break;

        // --- ��� C: �ִ� Mesh ��Ⱦ���� ---
    case ERHIShaderFrequency::Task:           stage = EShLangTaskNV; setId = "0"; break;
    case ERHIShaderFrequency::Mesh:           stage = EShLangMeshNV; setId = "1"; break;

        // --- ��� D: ����׷�ٹ��� (�ؼ�����������) ---
    case ERHIShaderFrequency::RayGen:
    case ERHIShaderFrequency::ClosestHit:
    case ERHIShaderFrequency::Miss:
    case ERHIShaderFrequency::AnyHit:
    case ERHIShaderFrequency::Intersection:
    case ERHIShaderFrequency::Callable:
        // ��׷�׶ν���ȫ��ӳ�䵽��ͬ�ļ����߼� Set (���� 0, 1, 2)
        // ����� stage ӳ��...
        setId = "0"; // ���߸�����ԴƵ����Ϊ "0", "1"
        break;

    default:
        out.ErrorMessage = "Unsupported shader stage";
        return;
    }

    // 2. ���� TShader
    glslang::TShader shader(stage);
    const char* sourceCStr = preprocessedSource.c_str();
    shader.setStrings(&sourceCStr, 1);
    shader.setEntryPoint(input.EntryPoint.c_str());
    shader.setEnvInput(glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_2);
    // ����ƫ���� (����Ը����Լ� RHI ��ϰ�ߵ�����Щ����)
    const int CBV_SHIFT = 100;   // b �Ĵ��� (Constant Buffer)
    const int SRV_SHIFT = 200; // t �Ĵ��� (Texture/Buffer SRV)
    const int SAMPLER_SHIFT = 300; // s �Ĵ��� (Sampler)
    const int UAV_SHIFT = 400; // u �Ĵ��� (RWTexture/RWBuffer UAV)

    // --- �����Զ�ӳ��binding ---
    shader.setShiftBindingForSet(glslang::EResUbo, CBV_SHIFT,0);
    shader.setShiftBindingForSet(glslang::EResUbo, CBV_SHIFT,0);
    shader.setShiftBindingForSet(glslang::EResTexture, SRV_SHIFT,0);
    shader.setShiftBindingForSet(glslang::EResSampler, SAMPLER_SHIFT, 0);
    shader.setShiftBindingForSet(glslang::EResUav, UAV_SHIFT, 0);
    shader.setShiftBindingForSet(glslang::EResImage, UAV_SHIFT, 0);
    shader.setShiftBindingForSet(glslang::EResSsbo, UAV_SHIFT,0);
    shader.setResourceSetBinding({ setId });

    // 3. ���Ӻ궨��
    TBuiltInResource resources = {};
    // ��ʼ��Ĭ����Դ����
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

    // ����������
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
        // ����
        for (auto p : preprocessorDefines) delete[] p;
        return;
    }
    for (auto p : preprocessorDefines) delete[] p;

    // 4. ���ӳ���
    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(EShMsgDefault))
    {
        out.ErrorMessage = program.getInfoLog();
        out.ErrorMessage += "\n";
        out.ErrorMessage += program.getInfoDebugLog();
        return;
    }
    program.mapIO();
    // 5. ���� SPIR-V
    std::vector<uint32_t> spirv;
    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

    

    // 6. ʹ�� SPIRV-Cross ������Դ
    spirv_cross::Compiler compiler(spirv);
    spirv_cross::ShaderResources resourcesSC = compiler.get_shader_resources();

    int globalUniformBufferIndex = -1;
    int globalUniformBufferSet = -1;
    // ---------- Uniform Buffers ----------
    for (const auto& ub : resourcesSC.uniform_buffers)
    {
        std::string bufferName = compiler.get_name(ub.id);
        if (bufferName.empty()) {
            bufferName = compiler.get_name(ub.base_type_id); // ���û��ʵ��������ȡ������ ($Globals)
        }

        uint32_t binding = compiler.get_decoration(ub.id, spv::DecorationBinding);
        uint32_t set = compiler.get_decoration(ub.id, spv::DecorationDescriptorSet);

        auto& type = compiler.get_type(ub.base_type_id);
        uint32_t bufferSize = static_cast<uint32_t>(compiler.get_declared_struct_size(type));

        // 2. ��������ʽ cbuffer ������ʽ LooseData ��
        // ͨ������ "$Globals" ����û��ʵ�����Ŀ���� LooseData
        bool bIsLooseDataBlock = (bufferName.find("$Global") != std::string::npos);

        if (bIsLooseDataBlock)
        {
            globalUniformBufferIndex = static_cast<int>(binding);
            globalUniformBufferSet = static_cast<int>(set);
            // --- ���� LooseData: ���ṹ���Ա ---
            uint32_t memberCount = (uint32_t)type.member_types.size();
            for (uint32_t i = 0; i < memberCount; i++)
            {
                // ��ȡ��Ա������ (�� "bExtraParam")
                std::string memberName = compiler.get_member_name(ub.base_type_id, i);
                // ��ȡ��Ա�� Buffer �ڲ���ƫ����
                uint32_t memberOffset = compiler.type_struct_member_offset(type, i);
                // ��ȡ��Ա�Ĵ�С
                uint32_t memberSize = static_cast<uint32_t>(compiler.get_declared_struct_member_size(type, i));

                // ע�⣺���� LooseData��������Ҫ�洢 Binding �� Offset ������Ϣ
                out.ParameterMap.AddParameterAllocation(
                    memberName,
                    static_cast<uint32_t>(binding),
                    static_cast<uint32_t>(memberOffset), // ����� Offset
                    static_cast<uint32_t>(memberSize),   // ����� Size
                    EShaderParameterType::LooseData      // ��ȷ��������
                );
            }
        }
        else
        {
            // --- ������ʽ Uniform Buffer (�� ComputeConstants) ---
            out.ParameterMap.AddParameterAllocation(
                bufferName,
                static_cast<uint32_t>(set),
                static_cast<uint32_t>(binding),  
                static_cast<uint32_t>(bufferSize),
                EShaderParameterType::UniformBuffer
            );
        }
    }

    // ---------- Sampled Images (Texture + Sampler) ----------
    for (const auto& img : resourcesSC.sampled_images)
    {
        std::string name = compiler.get_name(img.id);

        uint32_t binding = compiler.get_decoration(img.id, spv::DecorationBinding);
        uint32_t set = compiler.get_decoration(img.id, spv::DecorationDescriptorSet);

        out.ParameterMap.AddParameterAllocation(
            name,
            static_cast<uint32_t>(set),
            static_cast<uint32_t>(binding),
            1,
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
            static_cast<uint32_t>(set),
            static_cast<uint32_t>(binding),
            1,
            EShaderParameterType::Sampler
        );
    }
    // ---------- Separate Images ----------
    for (const auto& img : resourcesSC.separate_images)
    {
        std::string name = compiler.get_name(img.id);
        uint32_t binding = compiler.get_decoration(img.id, spv::DecorationBinding);
        uint32_t set = compiler.get_decoration(img.id, spv::DecorationDescriptorSet);

        out.ParameterMap.AddParameterAllocation(
            name, (uint32_t)set, (uint32_t)binding,1, EShaderParameterType::SRV
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
            static_cast<uint32_t>(set),
            static_cast<uint32_t>(binding),
            1,
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
            static_cast<uint32_t>(set),
            static_cast<uint32_t>(binding),
            1,
            EShaderParameterType::UAV
        );
    }

    // ���� Push Constants �ķ����߼�
    for (const auto& pc : resourcesSC.push_constant_buffers)
    {
        // ��ȡ��� PC ���������Ϣ
        auto& type = compiler.get_type(pc.base_type_id);

        // �����ڲ���Ա������Щ��������ѡ�е� Loose Data��
        for (uint32_t i = 0; i < type.member_types.size(); i++)
        {
            std::string name = compiler.get_member_name(pc.base_type_id, i);
            uint32_t offset = compiler.type_struct_member_offset(type, i);
            uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_member_size(type, i));

            // ������� ParameterMap
            out.ParameterMap.AddParameterAllocation(
                name,
                0,      // BufferIndex �� PC ͨ��û���壬����Ϊ�ض���ʶ
                static_cast<uint16_t>(offset), // BaseIndex ���� PC �� Offset
                static_cast<uint16_t>(size),
                EShaderParameterType::LooseData // �������
            );
        }
    }

    // 6. ������
    out.PackedBinaryData.resize(spirv.size() * sizeof(uint32_t));
    memcpy(out.PackedBinaryData.data(), spirv.data(), spirv.size() * sizeof(uint32_t));

	SPIRVPackSource packSource;
	packSource.compiler = &compiler;
	packSource.spirvCode = &spirv;
    packSource.frequency = input.Frequency;
    packSource.entryPoint = input.EntryPoint;
    packSource.globalUniformBufferBinding = globalUniformBufferIndex;
    packSource.globalUniformBufferSet = globalUniformBufferSet;
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

    read(&DepackedData.header.GlobalUniformBufferBinding, sizeof(int));
	read(&DepackedData.header.GlobalUniformBufferSet, sizeof(int));
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

    // �� hash
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
    // Separated images
    // =========================
    for (auto& img : resources.separate_images)
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
    // Descriptor sort���ǳ���Ҫ��
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

    DepackedData.header.GlobalUniformBufferBinding = src->globalUniformBufferBinding;
	DepackedData.header.GlobalUniformBufferSet = src->globalUniformBufferSet;
    write(&DepackedData.header.GlobalUniformBufferBinding, sizeof(int));
    write(&DepackedData.header.GlobalUniformBufferSet, sizeof(int));
    return true;
}
} // namespace RenderCore