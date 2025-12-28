#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "EngineExport.h"
namespace Engine {

    // 前置声明
    class Texture;
    class Shader;

    // 枚举
    enum class EShadingModel { DefaultLit, Unlit, Subsurface };
    enum class EBlendMode { Opaque, Masked, Translucent };
    enum class EMaterialParamType { Scalar, Vector, Texture, StaticSwitch };

    // Material
    class ENGINE_API Material
    {
    public:
        Material()
            : ShadingModel(EShadingModel::DefaultLit), BlendMode(EBlendMode::Opaque), bTwoSided(false),
            BaseColor{ 1,1,1,1 }, Roughness(1.f), Metallic(0.f), Specular(0.f), Emissive{ 0,0,0,0 },
            BaseColorTex(nullptr), NormalTex(nullptr), RoughnessTex(nullptr), MetallicTex(nullptr) {
        }

        EShadingModel ShadingModel;
        EBlendMode BlendMode;
        bool bTwoSided;

        // 基础参数
        struct Vec4 { float x, y, z, w; };
        Vec4 BaseColor;
        float Roughness;
        float Metallic;
        float Specular;
        Vec4 Emissive;

        // 贴图
        Texture* BaseColorTex;
        Texture* NormalTex;
        Texture* RoughnessTex;
        Texture* MetallicTex;

        // 静态开关参数
        std::unordered_map<std::string, bool> StaticSwitches;
    };

    // MaterialParameter
    class ENGINE_API MaterialParameter
    {
    public:
        std::string Name;
        EMaterialParamType Type;

        union Value
        {
            float ScalarValue;
            Material::Vec4 VectorValue;
            Texture* TextureValue;
            bool SwitchValue;

            Value() : ScalarValue(0.f) {}
        } ParamValue;
    };

    // MaterialShaderKey
    class MaterialShaderKey
    {
    public:
        EShadingModel ShadingModel;
        EBlendMode BlendMode;
        std::unordered_map<std::string, bool> StaticSwitches;

        bool operator==(const MaterialShaderKey& other) const
        {
            return ShadingModel == other.ShadingModel &&
                BlendMode == other.BlendMode &&
                StaticSwitches == other.StaticSwitches;
        }
    };

    // MaterialShaderPermutation
    class ENGINE_API MaterialShaderPermutation
    {
    public:
        MaterialShaderKey Key;
        Shader* CompiledShader; // GPU Shader
    };


    // MaterialRenderProxy
    class ENGINE_API MaterialRenderProxy
    {
    public:
        MaterialRenderProxy(Material* parent)
            : ParentMaterial(parent) {
        }

        Shader* GetShader(const MaterialShaderKey& key);

        Material* ParentMaterial;
        std::unordered_map<std::string, MaterialParameter> Parameters;
    };

    // MaterialShaderMap
    class ENGINE_API MaterialShaderMap
    {
    public:
        std::unordered_map<MaterialShaderKey, Shader*> ShaderPermutations;
    };

}