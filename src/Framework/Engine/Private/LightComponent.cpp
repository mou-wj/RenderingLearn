#include "LightComponent.h"

namespace Engine
{

    LightComponent::LightComponent()
    {
    }

    void LightComponent::SetColor(
        const Core::Float3& color)
    {
        Color = color;

        MarkRenderStateDirty();
    }

    void LightComponent::SetIntensity(
        float intensity)
    {
        Intensity = intensity;

        MarkRenderStateDirty();
    }

    void LightComponent::SetCastShadow(
        bool castShadow)
    {
        bCastShadow = castShadow;

        MarkRenderStateDirty();
    }

    const Core::Float3&
        LightComponent::GetColor() const
    {
        return Color;
    }

    float LightComponent::GetIntensity() const
    {
        return Intensity;
    }

    bool LightComponent::IsCastShadow() const
    {
        return bCastShadow;
    }

    DirectionalLightComponent::
        DirectionalLightComponent()
    {
    }

    ELightType
        DirectionalLightComponent::
        GetLightType() const
    {
        return ELightType::Directional;
    }

    void LocalLightComponent::
        SetAttenuationRadius(
            float radius)
    {
        AttenuationRadius = radius;

        MarkRenderStateDirty();
    }

    float LocalLightComponent::
        GetAttenuationRadius() const
    {
        return AttenuationRadius;
    }

    PointLightComponent::
        PointLightComponent()
    {
    }

    ELightType
        PointLightComponent::
        GetLightType() const
    {
        return ELightType::Point;
    }

    SpotLightComponent::
        SpotLightComponent()
    {
    }

    ELightType
        SpotLightComponent::
        GetLightType() const
    {
        return ELightType::Spot;
    }

    void SpotLightComponent::
        SetInnerConeAngle(
            float angle)
    {
        InnerConeAngle = angle;

        MarkRenderStateDirty();
    }

    void SpotLightComponent::
        SetOuterConeAngle(
            float angle)
    {
        OuterConeAngle = angle;

        MarkRenderStateDirty();
    }

    float SpotLightComponent::
        GetInnerConeAngle() const
    {
        return InnerConeAngle;
    }

    float SpotLightComponent::
        GetOuterConeAngle() const
    {
        return OuterConeAngle;
    }

    SkyLightComponent::
        SkyLightComponent()
    {
    }

    ELightType
        SkyLightComponent::
        GetLightType() const
    {
        return ELightType::Sky;
    }

}