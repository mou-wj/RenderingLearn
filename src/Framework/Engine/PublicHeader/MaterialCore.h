#pragma once
#include "Shader.h"
#include "HashHelper.hpp"
#include "EngineExport.h"
#include <unordered_map>
namespace RenderCore {
    class VertexFactoryType;
}

namespace Engine {
    enum class EBlendMode : uint8_t {
        Opaque,
        Masked,
        Translucent
    };

    enum class EShadingModel : uint8_t {
        Unlit = 0,
        Lit,
        COUNT_MAX
    };

    struct MaterialPermutationParameters
    {
        EShadingModel ShadingModel = EShadingModel::Unlit;

        MaterialPermutationParameters() = default;
        MaterialPermutationParameters(EBlendMode blend, EShadingModel shading)
            : ShadingModel(shading) {}

        bool operator==(const MaterialPermutationParameters& other) const {
            return  ShadingModel == other.ShadingModel;
        }

        MaterialPermutationParameters& operator=(const MaterialPermutationParameters& other) {
            if (this != &other) {
                ShadingModel = other.ShadingModel;
            }
            return *this;
        }
    };

    ENGINE_API void ModifyShaderCompilerEnvironment(
        const MaterialPermutationParameters& Parameters,
        RenderCore::ShaderCompilerEnvironment& OutEnvironment);

    class MaterialShaderType;

    class MeshMaterialShaderType;

    struct MaterialShaderKey {
        MaterialShaderType* ShaderType;
        RenderCore::ShaderPermutationId PermutationId;
        MaterialPermutationParameters MaterialParameter;

        MaterialShaderKey(
            MaterialShaderType* shaderType,
            RenderCore::ShaderPermutationId permutationId,
            const MaterialPermutationParameters& matParam)
            : ShaderType(shaderType), PermutationId(permutationId), MaterialParameter(matParam) {}

        // Default constructor
        MaterialShaderKey(
            ) : ShaderType(nullptr), PermutationId(), MaterialParameter(*(MaterialPermutationParameters*)nullptr) {}

        bool operator==(const MaterialShaderKey& other) const {
            return ShaderType == other.ShaderType &&
                PermutationId == other.PermutationId &&
                MaterialParameter == other.MaterialParameter;
        }

        MaterialShaderKey& operator=(const MaterialShaderKey& other) {
            if (this != &other) {
                ShaderType = other.ShaderType;
                PermutationId = other.PermutationId;
                // 注意：引用成员不能重新赋值，只能在构造时初始化
                MaterialParameter = other.MaterialParameter;
            }
            return *this;
        }
    };

    struct MeshMaterialShaderKey {
        MeshMaterialShaderType* ShaderType;
        RenderCore::VertexFactoryType* VF;
        RenderCore::ShaderPermutationId PermutationId;
        RenderCore::VertexFactoryFeatureFlags VertexFactoryFlags;
        MaterialPermutationParameters MaterialParameter;

        MeshMaterialShaderKey(
            MeshMaterialShaderType* shaderType,
            RenderCore::VertexFactoryType* vf,
            RenderCore::ShaderPermutationId permutationId,
            const RenderCore::VertexFactoryFeatureFlags& vfFlags,
            const MaterialPermutationParameters& matParam)
            : ShaderType(shaderType), VF(vf), PermutationId(permutationId), VertexFactoryFlags(vfFlags), MaterialParameter(matParam) {}

        // Default constructor
        MeshMaterialShaderKey()
            : ShaderType(nullptr), VF(nullptr), PermutationId(), VertexFactoryFlags(0), MaterialParameter(MaterialPermutationParameters()) {
        }

        bool operator==(const MeshMaterialShaderKey& other) const {
            return ShaderType == other.ShaderType &&
                VF == other.VF &&
                PermutationId == other.PermutationId &&
                VertexFactoryFlags == other.VertexFactoryFlags &&
                MaterialParameter == other.MaterialParameter;
        }

        MeshMaterialShaderKey& operator=(const MeshMaterialShaderKey& other) {
            if (this != &other) {
                ShaderType = other.ShaderType;
                VF = other.VF;
                PermutationId = other.PermutationId;
                // 注意：引用成员不能重新赋值，只能在构造时初始化
                VertexFactoryFlags = other.VertexFactoryFlags;
                MaterialParameter = other.MaterialParameter;
            }
            return *this;
        }
    };
}

namespace std {
    template<>
    struct std::hash<Engine::MaterialPermutationParameters> {
        size_t operator()(const Engine::MaterialPermutationParameters& Key) const
        {
            size_t h = 0;
            HashCombine(h, std::hash<uint8_t>()(static_cast<uint8_t>(Key.ShadingModel)));
            return h;
        }
    };

    template<>
    struct std::hash<Engine::MaterialShaderKey> {
        size_t operator()(const Engine::MaterialShaderKey& Key) const
        {
            size_t h = 0;
            HashCombine(h, std::hash<void*>()(Key.ShaderType));
            HashCombine(h, std::hash<RenderCore::ShaderPermutationId>()(Key.PermutationId));
            HashCombine(h, std::hash<Engine::MaterialPermutationParameters>()(Key.MaterialParameter));
            return h;
        }
    };
    template<>
    struct std::hash<Engine::MeshMaterialShaderKey> {
        size_t operator()(const Engine::MeshMaterialShaderKey& Key) const
        {
            size_t h = 0;
            HashCombine(h, std::hash<void*>()(Key.ShaderType));
            HashCombine(h, std::hash<void*>()(Key.VF));
            HashCombine(h, std::hash<RenderCore::ShaderPermutationId>()(Key.PermutationId));
            HashCombine(h, std::hash<RenderCore::VertexFactoryFeatureFlags>()(Key.VertexFactoryFlags));
            HashCombine(h, std::hash<Engine::MaterialPermutationParameters>()(Key.MaterialParameter));
            return h;
        }
    };
}

namespace Engine {


    struct MeshMaterialShaderPermutationParameters : public RenderCore::ShaderPermutationParameters
    {
        MaterialPermutationParameters MaterialParams;
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
			ShaderCompiledInitializer(
				const RenderCore::ShaderType* Type,
				const std::vector<char>& Code,
				const RenderCore::ShaderParameterAllocationMap& ParameterMap,
				uint32_t PermutationId,
				RenderCore::VertexFactoryType* InFactoryType)
				: RenderCore::ShaderCompiledInitializer(Type, Code, ParameterMap, PermutationId),
				FactoryType(InFactoryType) {
			}
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
                EShaderTypeFlag::MeshMaterial
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
        void Initialize();
        void Clear();
    public:

        RenderCore::Shader* GetShader(const MeshMaterialShaderKey& Key);

    private:
        RenderCore::Shader* CompileShader(const MeshMaterialShaderKey& Key);


        std::unordered_map<
            MeshMaterialShaderKey,
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
        using ShaderMetaType = MaterialShaderType;
        void Initialize();
        void Clear();
    public:

        RenderCore::Shader* GetShader(const MaterialShaderKey& Key);

    private:
        RenderCore::Shader* CompileShader(const MaterialShaderKey& Key);


        std::unordered_map<
            MaterialShaderKey,
            RenderCore::ShaderSP
        > ShaderMap;

    private:

    };

    struct MaterialShaderPermutationParameters : public RenderCore::ShaderPermutationParameters
    {
        MaterialPermutationParameters MaterialParams;
    };

    class ENGINE_API MaterialShaderType : public RenderCore::ShaderType
    {
    public:
        using ShaderCompiledInitializer = RenderCore::ShaderCompiledInitializer;
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