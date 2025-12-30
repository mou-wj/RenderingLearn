#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "EngineExport.h"
#include "Math.hpp"

namespace Engine {
    // MaterialShaderKey
    struct MaterialShaderKey
    {
        uint8_t ShadingModel;
        uint8_t BlendMode;

        uint64_t StaticSwitchMask;   // <= 64 个 switch
        uint64_t FeatureLevelMask;   // 可扩展

        bool operator==(const MaterialShaderKey& rhs) const {
            return ShadingModel == rhs.ShadingModel &&
                BlendMode == rhs.BlendMode &&
                StaticSwitchMask == rhs.StaticSwitchMask &&
                FeatureLevelMask == rhs.FeatureLevelMask;
        }
    };


}

namespace std
{
    template<>
    struct hash<Engine::MaterialShaderKey>
    {
        size_t operator()(const Engine::MaterialShaderKey& key) const noexcept
        {
            // 64-bit hash accumulator
            size_t h = 0;

            // helper lambda
            auto hash_combine = [&h](size_t v)
                {
                    // 64-bit version of boost::hash_combine
                    h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
                };

            hash_combine(static_cast<size_t>(key.ShadingModel));
            hash_combine(static_cast<size_t>(key.BlendMode));
            hash_combine(static_cast<size_t>(key.StaticSwitchMask));
            hash_combine(static_cast<size_t>(key.FeatureLevelMask));

            return h;
        }
    };
}

namespace Engine {

    // 前置声明
    class Texture;
    class Shader;

    // 枚举
    enum class EShadingModel { DefaultLit, Unlit, Subsurface };
    enum class EBlendMode { Opaque, Masked, Translucent };
    enum class EMaterialParamType { Scalar, Vector, Texture, StaticSwitch };
    struct MaterialStaticParameters
    {
        std::unordered_map<std::string, bool> StaticSwitches;
    };

    struct MaterialRuntimeParameters
    {
        std::unordered_map<std::string, float> Scalars;
        std::unordered_map<std::string, Core::Float4> Vectors;
        std::unordered_map<std::string, Texture*> Textures;
    };



    // MaterialShaderMap
    class ENGINE_API MaterialShaderMap
    {
    public:
        std::unordered_map<MaterialShaderKey, Shader*> ShaderPermutations;
    };
    class ENGINE_API MaterialInterface
    {
    public:
        virtual Shader* GetShader(const MaterialShaderKey&) = 0;
        virtual const MaterialShaderMap* GetShaderMap() const = 0;
        virtual void GetStaticParameters(MaterialStaticParameters&) const = 0;
        virtual void GetRuntimeParameters(MaterialRuntimeParameters&) const = 0;
    };
	using MaterialInterfaceSP = std::shared_ptr<MaterialInterface>;
    class ENGINE_API MaterialInstance : public MaterialInterface
    {
    public:
        MaterialInterface* Parent;
        // 覆盖参数
        MaterialRuntimeParameters OverrideRuntimeParams;
        MaterialStaticParameters  OverrideStaticParams;

        Shader* GetShader(const MaterialShaderKey&) override {
            return nullptr;
        }
    };

    class ENGINE_API Material : public MaterialInterface
    {
    public:
        EShadingModel ShadingModel;
        EBlendMode BlendMode;


        // 默认参数（母材质定义）
        MaterialRuntimeParameters DefaultRuntimeParams;
        MaterialStaticParameters  StaticParams;

        // Shader 规则
        MaterialShaderMap* ShaderMap;

        Shader* GetShader(const MaterialShaderKey&) override {
            return nullptr;
        }
    };

    // MaterialShaderPermutation,后续移走
    class ENGINE_API MaterialShaderPermutation
    {
    public:
        MaterialShaderKey Key;
        Shader* CompiledShader; // GPU Shader
    };


    class ENGINE_API MaterialRenderProxy
    {
    public:
        Shader* Shader;
        MaterialRuntimeParameters Parameters;  // uniform / texture
    };




}

