#include "MaterialCore.h"
#include "VertexFactory.h"
#include "RHIApi.h"
#include "ShaderCompiler.h"
using namespace RenderCore;
namespace Engine {



    bool MeshMaterialShaderMap::Initialize()
    {
        auto allVFType = VertexFactoryType::GetRegisterMap();
        auto allShaderType = MeshMaterialShaderType::GetRegisterMap()[RenderCore::ShaderType::EShaderTypeFlag::MeshMaterial];
        for (auto& vf : allVFType) {
            for (auto& shader : allShaderType) {
                auto vfType = vf.second;
                auto shaderType = shader.second;
                for (uint32_t permId = 0; permId < shaderType->TotalPermutationCount; ++permId) {
                    for (uint64_t vfPermutationid = 0; vfPermutationid < vfType->PermutationTotalCount; ++vfPermutationid)
                    {
                        CompileShader(static_cast<MeshMaterialShaderType*>(shaderType), vfType, permId, vfPermutationid);
                    }
                }

            }
        }

        return false;
    }

    RenderCore::Shader* MeshMaterialShaderMap::GetShader(
        MeshMaterialShaderType* ShaderTypePtr,
        VertexFactoryType* VF,
        RenderCore::ShaderPermutationId PermutationId,
        const RenderCore::VertexFactoryFeatureFlags& VertexFactoryFlags
    ) {
        MeshShaderKey Key{ ShaderTypePtr, VF, PermutationId };
        auto it = ShaderMap.find(Key);
        if (it != ShaderMap.end()) {
            return it->second.get();
        }
        return CompileShader(ShaderTypePtr, VF, PermutationId, VertexFactoryFlags);
    }

    RenderCore::Shader* MeshMaterialShaderMap::CompileShader(MeshMaterialShaderType* ShaderTypePtr, RenderCore::VertexFactoryType* VF, RenderCore::ShaderPermutationId PermutationId, const RenderCore::VertexFactoryFeatureFlags& VertexFactoryFlags)
    {
        auto* st =
            ShaderTypePtr;

        if (!st)
        {
            return nullptr;
        }

        RHI::ERHIShaderPlatform platform =
            GShaderPlatform;
        auto vfType = VF;


        // 2. 遍历该 ShaderType 的所有变体组合 (由 IMPLEMENT_SHADER_TYPE 宏自动提取)

        // 3. 检查当前变体是否需要编译
        MeshMaterialShaderPermutationParameters permParams;
        permParams.Platform = static_cast<uint32_t>(platform);
        permParams.PermutationId = static_cast<uint32_t>(PermutationId);
        permParams.VFType = vfType;
        if (st->ShouldCompilePermutation && !st->ShouldCompilePermutation(permParams))
        {
            return nullptr;
        }

        // 4. 准备编译环境 (Environment)
        ShaderCompilerEnvironment env;
        if (vfType) {
            VertexFactoryShaderPermutationParameters vfPermParams;
            vfPermParams.VertexFactoryFlags = VertexFactoryFlags;
            vfPermParams.Platform = platform;
            auto parameterInfos = ShaderCompiler::GenerateOrGetShaderPrameterMetaDataSF(*(vfType->RootParametersMetadata));
            vfType->ModifyCompilationEnvironment(vfPermParams, env);

        }

        // A. 处理 Shader 参数 (VFS 注册)
        if (st->RootParametersMetadata)
        {
            // 生成 HLSL 并注册到 GShaderVirtualFileSystem
            auto parameterInfos = ShaderCompiler::GenerateOrGetShaderPrameterMetaDataSF(*(st->RootParametersMetadata));
        }

        // B. 处理变体宏注入 (调用子类静态方法)
        if (st->ModifyCompilationEnvironment)
        {
            // 内部会调用 Domain.SetFromId(permId) 和 Domain.ModifyCompilationEnvironment(env)
            st->ModifyCompilationEnvironment(permParams, env);
        }



        // 5. 构造编译输入并调用编译器
        ShaderCompileInput input;
        input.VirtualSourceFilePath = st->SourceFile;
        input.EntryPoint = st->EntryPoint;
        input.Frequency = st->Frequency;
        input.Platform = platform;
        input.Environment = std::move(env); // 移动环境数据

        ShaderCompilationOutput output = ShaderCompiler::Compile(input);

        if (!output.Success)
        {
            // 记录错误并跳过此变体
            // LOG_ERROR("Failed to compile %s (Permutation %d)", st->Name.c_str(), permId);
            return nullptr;
        }

        // 6. 构造 RHI Shader 对象
        ShaderCompiledInitializer initializer(st, output.PackedBinaryData, output.ParameterMap, PermutationId);

        // 7. 实例化并缓存
        ShaderSP shaderInstance = std::shared_ptr<Shader>(st->ConstructCompiled(initializer));

        // ShaderMap 结构: std::unordered_map<ShaderType*, std::map<ShaderPermutationId, ShaderSP>>

        MeshShaderKey key;

        key.Type = st;

        key.VF = vfType;

        key.PermutationId = PermutationId;

        ShaderMap[key] =
            shaderInstance;


    }



}