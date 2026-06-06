#pragma once

#include "LightComponent.h"

namespace Engine
{

    class LightSceneProxy
    {
    public:
        explicit LightSceneProxy(
            const LightComponent* component);

        virtual ~LightSceneProxy() = default;

    public:
        virtual ELightType GetLightType()
            const = 0;

        virtual void UpdateFromComponent(
            const LightComponent* component);

    public:
        const Core::Float3&
            GetPosition() const;

        const Core::Float3&
            GetDirection() const;

        const Core::Float3&
            GetColor() const;

        float GetIntensity()
            const;

        bool IsCastShadow()
            const;

    protected:
        Core::Float3 Position =
            Core::Float3(
                0.0f,
                0.0f,
                0.0f);

        Core::Float3 Direction =
            Core::Float3(
                1.0f,
                0.0f,
                0.0f);

        Core::Float3 Color =
            Core::Float3(
                1.0f,
                1.0f,
                1.0f);

        float Intensity =
            1.0f;

        bool bCastShadow =
            true;
    };

    class DirectionalLightSceneProxy
        : public LightSceneProxy
    {
    public:
        explicit DirectionalLightSceneProxy(
            const DirectionalLightComponent*
            component);

    public:
        ELightType GetLightType()
            const override;
    };

    class LocalLightSceneProxy
        : public LightSceneProxy
    {
    public:
        explicit LocalLightSceneProxy(
            const LocalLightComponent*
            component);

    public:
        void UpdateFromComponent(
            const LightComponent*
            component)
            override;

    public:
        float GetAttenuationRadius()
            const;

    protected:
        float AttenuationRadius =
            1000.0f;
    };

    class PointLightSceneProxy
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

    class SpotLightSceneProxy
        : public LocalLightSceneProxy
    {
    public:
        explicit SpotLightSceneProxy(
            const SpotLightComponent*
            component);

    public:
        void UpdateFromComponent(
            const LightComponent*
            component)
            override;

        ELightType GetLightType()
            const override;

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
    };

    class SkyLightSceneProxy
        : public LightSceneProxy
    {
    public:
        explicit SkyLightSceneProxy(
            const SkyLightComponent*
            component);

    public:
        ELightType GetLightType()
            const override;
    };

}