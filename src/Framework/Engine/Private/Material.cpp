#include "Material.h"
#include "RenderResource.h" // 包含你的基类以及 RenderTexture 的定义
#include "AssetManager.h"
#include "Shader.h"

namespace Engine {
    MeshMaterialShaderMap GMeshMaterialShaderMap;
    MaterialShaderMap GMaterialShaderMap;

    // ============================================================================
// MaterialParameter 实现
// ============================================================================
    MaterialParameter::MaterialParameter(const std::string& InName, EMaterialParameterSemantic InSemantic)
        : Name(InName), Semantic(InSemantic) {
    }

    MaterialRenderProxy::MaterialRenderProxy(const MaterialInterface* parent) : Parent(parent) {

    }

    // ============================================================================
    // MaterialRenderProxy 实现
    // ============================================================================
    void MaterialRenderProxy::AddParameter(std::unique_ptr<MaterialParameter> Param) {
        if (Param) {
            Parameters.push_back(std::move(Param));
        }
    }

    // ============================================================================
    // Material 实现
    // ============================================================================
    void Material::SetBlendMode(EBlendMode InMode) {
        if (BlendMode != InMode) {
            BlendMode = InMode;
            bProxyDirty = true;
        }
    }
    void Material::SetShadingModel(EShadingModel InModel) {
		if (ShadingModel != InModel) {
			ShadingModel = InModel;
			bProxyDirty = true;
		}
    }

    MaterialRenderProxy* Material::GetRenderProxy() const {
        if (bProxyDirty || !CachedProxy) {
            CachedProxy = std::make_unique<MaterialRenderProxy>(this);
            // 将资产内持有的基础默认数据全部 Clone 进只读代理
            for (const auto& [Name, Param] : DefaultParameters) {
                CachedProxy->AddParameter(Param->Clone());
            }
            bProxyDirty = false;
        }
        return CachedProxy.get();
    }

    void Material::SetParameterValueImpl(EMaterialParameterSemantic Semantic, const std::string& Name, std::unique_ptr<MaterialParameter> Param) {
        DefaultParameters[Name] = std::move(Param);
        bProxyDirty = true;
    }

    // ============================================================================
    // MaterialInstance 实现
    // ============================================================================
    MaterialInstance::MaterialInstance(MaterialInterface* InParent)
        : Parent(InParent) {
        assert(Parent && "MaterialInstance generated with a null parent!");
    }

    const Material* MaterialInstance::GetMaterial() const {
        return Parent->GetMaterial();
    }

    EBlendMode MaterialInstance::GetBlendMode() const {
        return Parent->GetBlendMode();
    }        
    EShadingModel MaterialInstance::GetShadingModel() const {
        return Parent->GetShadingModel();
    }

    MaterialRenderProxy* MaterialInstance::GetRenderProxy() const {
        if (bProxyDirty || !CachedProxy) {
            CachedProxy = std::make_unique<MaterialRenderProxy>(this);

            const Material* BaseMaterial = GetMaterial();
            if (BaseMaterial) {
                // 1. 倒装父材质的默认参数
                for (const auto& [Name, Param] : BaseMaterial->GetDefaultParameters()) {
                    if (!OverriddenParameters.contains(Name)) {
                        CachedProxy->AddParameter(Param->Clone());
                    }
                }
            }

            // 2. 倒装实例自己重写（Override）的参数覆盖默认数据
            for (const auto& [Name, Param] : OverriddenParameters) {
                CachedProxy->AddParameter(Param->Clone());
            }

            bProxyDirty = false;
        }
        return CachedProxy.get();
    }

    void MaterialInstance::SetParameterValueImpl(EMaterialParameterSemantic Semantic, const std::string& Name, std::unique_ptr<MaterialParameter> Param) {
        OverriddenParameters[Name] = std::move(Param);
        bProxyDirty = true;
    }


} // namespace Engine