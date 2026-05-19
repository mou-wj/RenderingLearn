#pragma once
#include "Shader.h"
#include "HashHelper.hpp"
#include "EngineExport.h"
#include <unordered_map>
namespace RenderCore {
    class VertexFactoryType;
}

namespace Engine {
    class MaterialShaderType;

    class MeshMaterialShaderType;
    struct MeshShaderKey
    {
        MeshMaterialShaderType* Type = nullptr;

        RenderCore::VertexFactoryType* VF = nullptr;

        RenderCore::ShaderPermutationId PermutationId = 0;

        bool operator==(const MeshShaderKey& rhs) const
        {
            return
                Type == rhs.Type &&
                VF == rhs.VF &&
                PermutationId == rhs.PermutationId;
        }
    };

}

namespace std {
    template<>
    struct std::hash<Engine::MeshShaderKey> {
        size_t operator()(const Engine::MeshShaderKey& Key) const
        {
            size_t h = 0;
            HashCombine(h, std::hash<void*>()(Key.Type));
            HashCombine(h, std::hash<void*>()(Key.VF));
            HashCombine(h, std::hash<RenderCore::ShaderPermutationId>()(Key.PermutationId));
            return h;
        }
    };
}

namespace Engine {



    struct MeshMaterialShaderPermutationParameters : public RenderCore::ShaderPermutationParameters
    {
        const RenderCore::VertexFactoryType* VFType;
    };


    /*
    ===============================================================================

        MaterialShaderType

        ShaderType specialization for material-based shaders.

    ===============================================================================
    */
    class ENGINE_API MeshMaterialShaderType : public RenderCore::ShaderType
    {
    public:
        struct ShaderCompiledInitializer : public RenderCore::ShaderCompiledInitializer {
            RenderCore::VertexFactoryType* FactoryType;
        };
        MeshMaterialShaderType(
            const std::string& InName,
            const std::string& InSourceFile,
            const std::string& InEntryPoint,
            RHI::ERHIShaderFrequency InFrequency,
            RenderCore::ModifyCompilationEnvironmentFuncType InModifyCompilationEnvironment,
            RenderCore::ShouldCompilePermutationFuncType InShouldCompilePermutation,
            RenderCore::ConstructCompiledFuncType InConstructCompiled,
            int32_t InTotalPermutationCount = 1,
            const RenderCore::ShaderParametersMetadata* InRootParametersMetadata = nullptr
        )
            : ShaderType(
                InName,
                InSourceFile,
                InEntryPoint,
                InFrequency,
                InModifyCompilationEnvironment,
                InShouldCompilePermutation,
                InConstructCompiled,
                InTotalPermutationCount,
                InRootParametersMetadata,
                EShaderTypeFlag::Global
            )
        {
        }

        virtual ~MeshMaterialShaderType() = default;

    public:

    };



    /*
    ===============================================================================

        MaterialShaderMap

        Stores compiled shader permutations for a specific material.

    ===============================================================================
    */
    class ENGINE_API MeshMaterialShaderMap
    {
    public:
        using ShaderMetaType = MeshMaterialShaderType;
        bool Initialize();

    public:

        RenderCore::Shader* GetShader(
            MeshMaterialShaderType* ShaderTypePtr,
            RenderCore::VertexFactoryType* VF,
            RenderCore::ShaderPermutationId PermutationId,
            const RenderCore::VertexFactoryFeatureFlags& VertexFactoryFlags
        );

    private:
        RenderCore::Shader* CompileShader(
            MeshMaterialShaderType* ShaderTypePtr,
            RenderCore::VertexFactoryType* VF,
            RenderCore::ShaderPermutationId PermutationId,
            const RenderCore::VertexFactoryFeatureFlags& VertexFactoryFlags
        );


        std::unordered_map<
            MeshShaderKey,
            RenderCore::ShaderSP
        > ShaderMap;

    private:

    };

    extern ENGINE_API MeshMaterialShaderMap GMeshMaterialShaderMap;

#define DECLARE_MESH_MATERIAL_SHADER_TYPE(ClassType) \
    DECLARE_SHADER_TYPE(ClassType)\
    ClassType(const ShaderMetaType::ShaderCompiledInitializer& Initializer) : MeshMaterialShader(Initializer) {}\

#define IMPLEMENT_MESH_MATERIAL_SHADER_TYPE(ClassType,ShaderPath,ShaderName,EntryPoint,Frequency ) \
    IMPLEMENT_SHADER_TYPE(ClassType,ShaderPath,ShaderName,EntryPoint,Frequency)


    class Material;
    class MaterialShaderType;
    class RenderCore::VertexFactoryType;

    /*
   ===============================================================================

       MaterialShaderMap

       Stores compiled shader permutations for a specific material.

   ===============================================================================
   */
    class ENGINE_API MaterialShaderMap
    {
    public:

        RenderCore::Shader* GetShader(
            RenderCore::ShaderType* ShaderTypePtr,
            RenderCore::ShaderPermutationId PermutationId)
        {
            auto TypeIt = ShaderMap.find(ShaderTypePtr);
            if (TypeIt == ShaderMap.end())
            {
                return nullptr;
            }

            auto& PermMap = TypeIt->second;

            auto PermIt = PermMap.find(PermutationId);
            if (PermIt == PermMap.end())
            {
                return nullptr;
            }

            return PermIt->second.get();
        }

        void AddShader(
            RenderCore::ShaderType* ShaderTypePtr,
            RenderCore::ShaderPermutationId PermutationId,
            RenderCore::ShaderSP ShaderInstance)
        {
            ShaderMap[ShaderTypePtr][PermutationId] =
                std::move(ShaderInstance);
        }

        void Clear()
        {
            ShaderMap.clear();
        }

    private:

        std::unordered_map<
            RenderCore::ShaderType*,
            std::unordered_map<
            RenderCore::ShaderPermutationId,
            RenderCore::ShaderSP
            >
        > ShaderMap;
    };



    class ENGINE_API MaterialShaderType : public RenderCore::ShaderType
    {
    public:

        using Super = RenderCore::ShaderType;

        MaterialShaderType(
            const std::string& InName,
            const std::string& InShaderPath,
            const std::string& InEntryPoint,
            ERHIShaderFrequency InFrequency,
            RenderCore::ModifyCompilationEnvironmentFuncType InModifyCompilationEnvironment,
            RenderCore::ShouldCompilePermutationFuncType InShouldCompilePermutation,
            RenderCore::ConstructCompiledFuncType InConstructInstance,
            int32_t InTotalPermutationCount = 1,
            const RenderCore::ShaderParametersMetadata* InRootParametersMetadata = nullptr
        )
            : ShaderType(
                InName,
                InShaderPath,
                InEntryPoint,
                InFrequency,
                InModifyCompilationEnvironment,
                InShouldCompilePermutation,
                InConstructInstance,
                InTotalPermutationCount,
                InRootParametersMetadata,
                EShaderTypeFlag::Material
            )
        {
        }

        virtual ~MaterialShaderType() = default;

    public:

    };

    extern ENGINE_API MaterialShaderMap GMaterialShaderMap;

#define DECLARE_MATERIAL_SHADER_TYPE(ClassType) \
    DECLARE_SHADER_TYPE(ClassType)
#define IMPLEMENT_MATERIAL_SHADER_TYPE(ClassType,ShaderPath,ShaderName,EntryPoint,Frequency ) \
    IMPLEMENT_SHADER_TYPE(ClassType,ShaderPath,ShaderName,EntryPoint,Frequency)

}