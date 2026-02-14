#include "Material.h"
namespace Engine {

    Shader* MaterialShaderMap::FindShader(const MaterialShaderKey& key) const {
        auto it = ShaderPermutations.find(key);
        return it != ShaderPermutations.end() ? it->second : nullptr;
    }

    void MaterialInstance::GatherStaticParameters(MaterialStaticParameters& Out) const {
        if (Parent) Parent->GatherStaticParameters(Out);
        // 然后覆盖本实例的 StaticSwitches
        for (auto& kv : OverrideStaticParams.StaticSwitches) {
            Out.StaticSwitches[kv.first] = kv.second;
        }
    }
    void MaterialInstance::GatherRuntimeParameters(MaterialRuntimeParameters& Out) const {
        if (Parent) Parent->GatherRuntimeParameters(Out);
        // 然后覆盖本实例的 Runtime 参数
        for (auto& kv : OverrideRuntimeParams.Scalars)    Out.Scalars[kv.first] = kv.second;
        for (auto& kv : OverrideRuntimeParams.Vectors)    Out.Vectors[kv.first] = kv.second;
        for (auto& kv : OverrideRuntimeParams.Textures)   Out.Textures[kv.first] = kv.second;
    }

    Shader* Material::GetShader(const MaterialShaderKey& Key) const {
        if (ShaderMap) return ShaderMap->FindShader(Key);
        return nullptr;
    }
    std::unique_ptr<MaterialRenderProxy> Material::CreateRenderProxy() const {
        auto Proxy = std::make_unique<MaterialRenderProxy>();
        MaterialShaderKey key;
        Proxy->Shader = GetShader(key);
        Proxy->Parameters = DefaultRuntimeParams;
        return Proxy;
    }
    void Material::GatherStaticParameters(MaterialStaticParameters& Out) const {
        Out = StaticParams;
    }
    void Material::GatherRuntimeParameters(MaterialRuntimeParameters& Out) const {
        Out = DefaultRuntimeParams;
    }
}