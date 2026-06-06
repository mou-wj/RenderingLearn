#pragma once

#include "SceneComponent.h"

namespace Engine
{

    enum class ELightType
    {
        Directional,
        Point,
        Spot,
        Sky
    };

    class LightComponent
        : public SceneComponent
    {
    public:
        LightComponent();
        virtual ~LightComponent() = default;

    public:
        virtual ELightType GetLightType()
            const = 0;

    public:
        void SetColor(
            const Core::Float3& color);

        void SetIntensity(
            float intensity);

        void SetCastShadow(
            bool castShadow);

    public:
        const Core::Float3&
            GetColor() const;

        float GetIntensity()
            const;

        bool IsCastShadow()
            const;

    protected:
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

    class DirectionalLightComponent
        : public LightComponent
    {
    public:
        DirectionalLightComponent();

    public:
        ELightType GetLightType()
            const override;
    };

    class LocalLightComponent
        : public LightComponent
    {
    public:
        void SetAttenuationRadius(
            float radius);

        float GetAttenuationRadius()
            const;

    protected:
        float AttenuationRadius =
            1000.0f;
    };

    class PointLightComponent
        : public LocalLightComponent
    {
    public:
        PointLightComponent();

    public:
        ELightType GetLightType()
            const override;
    };

    class SpotLightComponent
        : public LocalLightComponent
    {
    public:
        SpotLightComponent();

    public:
        ELightType GetLightType()
            const override;

    public:
        void SetInnerConeAngle(
            float angle);

        void SetOuterConeAngle(
            float angle);

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

    class SkyLightComponent
        : public LightComponent
    {
    public:
        SkyLightComponent();

    public:
        ELightType GetLightType()
            const override;
    };

}