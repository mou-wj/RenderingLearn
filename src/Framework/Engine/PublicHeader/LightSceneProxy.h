#pragma once

#include "LightComponent.h"
#include "EngineExport.h"
namespace Engine
{

    class ENGINE_API LightSceneProxy
    {
    public:
        explicit LightSceneProxy(
            const LightComponent* component);

        virtual ~LightSceneProxy() = default;

    public:
        virtual ELightType GetLightType()
            const = 0;

        virtual void UpdateFromComponent();

    public:
        const Core::Float3&
            GetPosition() const;


        const Core::Float3&
            GetColor() const;

        float GetIntensity()
            const;

        bool IsCastShadow()
            const;
        bool bUpdateCastShadow = false;
    protected:
        Core::Float3 Position =
            Core::Float3(
                0.0f,
                0.0f,
                0.0f);

        // Direction is light-type specific (directional/spot)

        Core::Float3 Color =
            Core::Float3(
                1.0f,
                1.0f,
                1.0f);

        float Intensity =
            1.0f;

        bool bCastShadow =
            true;
        const LightComponent* Component = nullptr;
    };

    class ENGINE_API DirectionalLightSceneProxy
        : public LightSceneProxy
    {
    public:
        explicit DirectionalLightSceneProxy(
            const DirectionalLightComponent*
            component);

    public:
        void UpdateFromComponent() override;

    public:
        const Core::Float3& GetDirection() const;

    protected:
        Core::Float3 Direction = Core::Float3(0.0f, -1.0f, 0.0f);

    public:
        ELightType GetLightType()
            const override;
    };

    class ENGINE_API LocalLightSceneProxy
        : public LightSceneProxy
    {
    public:
        explicit LocalLightSceneProxy(
            const LocalLightComponent*
            component);

    public:
        void UpdateFromComponent()
            override;

    public:
        float GetAttenuationRadius()
            const;

    protected:
        float AttenuationRadius =
            1000.0f;
    };

    class ENGINE_API PointLightSceneProxy
        : public LocalLightSceneProxy
    {
    public:
        explicit PointLightSceneProxy(
            const PointLightComponent*
            component);

    public:
        ELightType GetLightType()
            const override;
    };

    class ENGINE_API SpotLightSceneProxy
        : public LocalLightSceneProxy
    {
    public:
        explicit SpotLightSceneProxy(
            const SpotLightComponent*
            component);

    public:
        void UpdateFromComponent()
            override;

        ELightType GetLightType()
            const override;

    public:
        const Core::Float3& GetDirection() const;

    public:
        float GetInnerConeAngle()
            const;

        float GetOuterConeAngle()
            const;

    protected:
        float InnerConeAngle =
            20.0f;

        float OuterConeAngle =
            45.0f;

        Core::Float3 Direction = Core::Float3(0.0f, -1.0f, 0.0f);
    };

    class ENGINE_API SkyLightSceneProxy
        : public LightSceneProxy
    {
    public:
        explicit SkyLightSceneProxy(
            const SkyLightComponent*
            component);
    public:
        void UpdateFromComponent() override;

        // Environment texture and precompute dirty flag (copied from component)
        RenderCore::RenderTexture* GetEnvironmentMap() const { return EnvironmentMap; }
        RenderCore::RenderTexture* GetDiffuseIrradiance() const { return DiffuseIrradianceMap; }
        RenderCore::RenderTexture* GetSpecularPrefilter() const { return SpecularPrefilterMap; }
        bool IsPrecomputeDirty() const { return bPrecomputeDirty; }
        void ClearPrecomputeDirty() { bPrecomputeDirty = false; }

    public:
        ELightType GetLightType()
            const override;

    protected:
        RenderCore::RenderTexture* EnvironmentMap;
        RenderCore::RenderTexture* DiffuseIrradianceMap;
        RenderCore::RenderTexture* SpecularPrefilterMap;
        bool bPrecomputeDirty = true;
    };

}