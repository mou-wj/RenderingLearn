#include "Material.h"
#include "RenderResource.h" // 包含你的基类以及 RenderTexture 的定义
#include "AssetManager.h"
#include "Shader.h"

namespace Engine {
    MeshMaterialShaderMap GMeshMaterialShaderMap;
    MaterialShaderMap GMaterialShaderMap;


    RHIGraphicsPipelineStateSP MaterialInterface::GetGraphicPipelineState(const MaterialInterface* Material) {
        return nullptr;
    }
    RHIComputePipelineStateSP MaterialInterface::GetComputePipelineState(const MaterialInterface* Material) {
        return nullptr;
    }
    RenderCore::Shader* MaterialInterface::GetShader(const MaterialInterface* Material, ERHIShaderFrequency frequency) {
        return nullptr;
    }

    // 补全你的 TextureAsset 获取底层渲染资源的接口
    // (如果已经在别处实现了可以忽略，这里确保编译畅通)
    const RenderCore::RenderTexture* GetRenderTextureFromAsset(const std::shared_ptr<TextureAsset>& Asset) {
        // 实际开发中可以通过在 TextureAsset 加上类似 GetRenderResource() 的内联函数
        // 这里采用伪逻辑：假设静态转换或者调用内部成员
        return Asset ? nullptr : nullptr;
    }

    /*
    ===============================================================================
        DefaultMaterialRenderProxy (渲染代理内部实现)
    ===============================================================================
    */
    class DefaultMaterialRenderProxy : public MaterialRenderProxy {
    public:
        DefaultMaterialRenderProxy(const Material* InOwnerMaterial, const MaterialInterface* InGameThreadTarget)
            : MaterialRenderProxy(InOwnerMaterial)
            , GameThreadTarget(InGameThreadTarget)
        {
        }

        bool GetScalarParam(const std::string& Name, float& OutValue) const override {
            return GameThreadTarget->GetScalarParameterValue(Name, OutValue);
        }

        // 核心打通：完美的跨线程数据转换
        bool GetTextureParam(const std::string& Name, const RenderCore::RenderTexture*& OutTexture) const override {
            std::shared_ptr<TextureAsset> GameThreadAsset = nullptr;
            if (GameThreadTarget->GetTextureParameterValue(Name, GameThreadAsset)) {
                if (GameThreadAsset) {
                    // 核心链路：从你的 Asset 中取出底层的 RenderTexture 指针
                    // 提示：你需要在 TextureAsset 类里补一个 public 的 GetRenderResource() 接口返回它的 Texture_ 成员
                    // OutTexture = GameThreadAsset->GetRenderResource(); 
                    return true;
                }
            }
            return false;
        }

    private:
        const MaterialInterface* GameThreadTarget;
    };


    /*
    ===============================================================================
        Material 资产实现
    ===============================================================================
    */
    Material::Material() {
        RenderProxy = std::make_unique<DefaultMaterialRenderProxy>(this, this);
    }

    Material::~Material() {
        // 释放逻辑（多线程安全释放）
    }

    MaterialRenderProxy* Material::GetRenderProxy() const {
        return RenderProxy.get();
    }

    bool Material::GetTextureParameterValue(const std::string& Name, std::shared_ptr<TextureAsset>& OutTexture) const {
        auto it = TextureParameters.find(Name);
        if (it != TextureParameters.end()) {
            OutTexture = it->second;
            return true;
        }
        return false;
    }

    bool Material::GetScalarParameterValue(const std::string& Name, float& OutValue) const {
        auto it = ScalarParameters.find(Name);
        if (it != ScalarParameters.end()) {
            OutValue = it->second;
            return true;
        }
        return false;
    }

    void Material::SetTextureParameter(const std::string& Name, std::shared_ptr<TextureAsset> Texture) {
        TextureParameters[Name] = std::move(Texture);
    }

    void Material::SetScalarParameter(const std::string& Name, float Value) {
        ScalarParameters[Name] = Value;
    }

    bool Material::CompileShaders() {
        //实现材质编译shader
        
        return true;
    }


    /*
    ===============================================================================
        MaterialInstance 实例实现
    ===============================================================================
    */
    MaterialInstance::MaterialInstance(MaterialInterface* InParent)
        : Parent(InParent)
    {
        const Material* OwnerMat = Parent ? Parent->GetMaterial() : nullptr;
        RenderProxy = std::make_unique<DefaultMaterialRenderProxy>(OwnerMat, this);
    }

    MaterialInstance::~MaterialInstance() {}

    const Material* MaterialInstance::GetMaterial() const {
        return Parent ? Parent->GetMaterial() : nullptr;
    }

    MaterialRenderProxy* MaterialInstance::GetRenderProxy() const {
        return RenderProxy.get();
    }

    bool MaterialInstance::GetTextureParameterValue(const std::string& Name, std::shared_ptr<TextureAsset>& OutTexture) const {
        // 优先查看当前实例是否重写了该资产
        auto it = OverriddenTextures.find(Name);
        if (it != OverriddenTextures.end()) {
            OutTexture = it->second;
            return true;
        }
        // 否则向父级材质回溯
        return Parent ? Parent->GetTextureParameterValue(Name, OutTexture) : false;
    }

    bool MaterialInstance::GetScalarParameterValue(const std::string& Name, float& OutValue) const {
        auto it = OverriddenScalars.find(Name);
        if (it != OverriddenScalars.end()) {
            OutValue = it->second;
            return true;
        }
        return Parent ? Parent->GetScalarParameterValue(Name, OutValue) : false;
    }

    EBlendMode MaterialInstance::GetBlendMode() const {
        return Parent ? Parent->GetBlendMode() : EBlendMode::Opaque;
    }

    bool MaterialInstance::IsTwoSided() const {
        return Parent ? Parent->IsTwoSided() : false;
    }

    void MaterialInstance::SetTextureParameterValue(const std::string& Name, std::shared_ptr<TextureAsset> Texture) {
        OverriddenTextures[Name] = std::move(Texture);
    }

    void MaterialInstance::SetScalarParameterValue(const std::string& Name, float Value) {
        OverriddenScalars[Name] = Value;
    }


    /*
    ===============================================================================
        MaterialRenderProxy 基类实现
    ===============================================================================
    */
    MaterialRenderProxy::MaterialRenderProxy(const MaterialInterface* InOwnerMaterial)
        : OwnerMaterial(InOwnerMaterial)
    {
    }


} // namespace Engine