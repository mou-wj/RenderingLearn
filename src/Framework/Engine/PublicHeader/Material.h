#pragma once
#include "EngineExport.h"
#include "MaterialCore.h" // 包含底层的 ShaderMap
#include <string>
#include <unordered_map>
#include <memory>

// 前置声明你自己的资产类型和底层的渲染资源
namespace RenderCore {
    class RenderTexture;
    class Shader;
}

namespace Engine {
    class Material;
    class MaterialRenderProxy;
    class TextureAsset; // 使用你定义的资产类

    enum class EBlendMode : uint8_t {
        Opaque,
        Masked,
        Translucent
    };

    /*
    ===============================================================================
        MaterialInterface (游戏线程基类)
    ===============================================================================
    */
    class ENGINE_API MaterialInterface {
    public:
        MaterialInterface() = default;
        virtual ~MaterialInterface() = default;

        virtual const Material* GetMaterial() const = 0;
        virtual MaterialRenderProxy* GetRenderProxy() const = 0;

        // 核心变更：参数查询直接返回你封装好的 TextureAsset 的智能指针或裸指针
        virtual bool GetTextureParameterValue(const std::string& Name, std::shared_ptr<TextureAsset>& OutTexture) const = 0;
        virtual bool GetScalarParameterValue(const std::string& Name, float& OutValue) const = 0;

        virtual EBlendMode GetBlendMode() const = 0;
        virtual bool IsTwoSided() const = 0;

        MaterialInterface(const MaterialInterface&) = delete;
        MaterialInterface& operator=(const MaterialInterface&) = delete;
        static RHIGraphicsPipelineStateSP GetGraphicPipelineState(const MaterialInterface* Material);
        static RHIComputePipelineStateSP GetComputePipelineState(const MaterialInterface* Material);
        static RenderCore::Shader* GetShader(const MaterialInterface* Material,ERHIShaderFrequency frequency);
    };

    /*
    ===============================================================================
        Material (物理材质资产)
    ===============================================================================
    */
    class ENGINE_API Material : public MaterialInterface {
    public:
        Material();
        ~Material() override;

        const Material* GetMaterial() const override { return this; }
        MaterialRenderProxy* GetRenderProxy() const override;

        bool GetTextureParameterValue(const std::string& Name, std::shared_ptr<TextureAsset>& OutTexture) const override;
        bool GetScalarParameterValue(const std::string& Name, float& OutValue) const override;
        EBlendMode GetBlendMode() const override { return BlendMode; }
        bool IsTwoSided() const override { return bIsTwoSided; }

        bool CompileShaders();

        // 编辑器接口
        void SetTextureParameter(const std::string& Name, std::shared_ptr<TextureAsset> Texture);
        void SetScalarParameter(const std::string& Name, float Value);

    private:
        EBlendMode BlendMode = EBlendMode::Opaque;
        bool bIsTwoSided = false;

        std::unique_ptr<MaterialRenderProxy> RenderProxy;

        // 对应你资产系统的映射表
        std::unordered_map<std::string, std::shared_ptr<TextureAsset>> TextureParameters;
        std::unordered_map<std::string, float> ScalarParameters;
    };

    /*
    ===============================================================================
        MaterialInstance (材质实例)
    ===============================================================================
    */
    class ENGINE_API MaterialInstance : public MaterialInterface {
    public:
        MaterialInstance(MaterialInterface* InParent);
        ~MaterialInstance() override;

        const Material* GetMaterial() const override;
        MaterialRenderProxy* GetRenderProxy() const override;

        bool GetTextureParameterValue(const std::string& Name, std::shared_ptr<TextureAsset>& OutTexture) const override;
        bool GetScalarParameterValue(const std::string& Name, float& OutValue) const override;
        EBlendMode GetBlendMode() const override;
        bool IsTwoSided() const override;

        void SetTextureParameterValue(const std::string& Name, std::shared_ptr<TextureAsset> Texture);
        void SetScalarParameterValue(const std::string& Name, float Value);

    private:
        MaterialInterface* Parent;
        std::unique_ptr<MaterialRenderProxy> RenderProxy;

        std::unordered_map<std::string, std::shared_ptr<TextureAsset>> OverriddenTextures;
        std::unordered_map<std::string, float> OverriddenScalars;
    };

    /*
    ===============================================================================
        MaterialRenderProxy (渲染线程专用只读代理)
    ===============================================================================
    */
    class ENGINE_API MaterialRenderProxy {
    public:
        MaterialRenderProxy(const MaterialInterface* InOwnerMaterial);
        virtual ~MaterialRenderProxy() = default;

        virtual bool GetScalarParam(const std::string& Name, float& OutValue) const = 0;

        // 关键打通：直接向渲染器返回底层的 RenderCore::RenderTexture 指针
        virtual bool GetTextureParam(const std::string& Name, const RenderCore::RenderTexture*& OutTexture) const = 0;
        const MaterialInterface* GetOwnerMaterial() const { return OwnerMaterial; }
    protected:
        const MaterialInterface* OwnerMaterial;
    };
}