#include "LightSceneProxy.h"
#include "Transform.hpp"

namespace Engine
{

    LightSceneProxy::
        LightSceneProxy(
            const LightComponent* component)
    {
        UpdateFromComponent(
            component);
    }

    void LightSceneProxy::
        UpdateFromComponent(
            const LightComponent* component)
    {
        
        Position =
            component->GetWorldLocation();

        Direction =
            component->GetForwardVector();

        Color =
            component->GetColor();

        Intensity =
            component->GetIntensity();

        bCastShadow =
            component->IsCastShadow();
    }

    const Core::Float3&
        LightSceneProxy::
        GetPosition() const
    {
        return Position;
    }

    const Core::Float3&
        LightSceneProxy::
        GetDirection() const
    {
        return Direction;
    }

    const Core::Float3&
        LightSceneProxy::
        GetColor() const
    {
        return Color;
    }

    float LightSceneProxy::
        GetIntensity() const
    {
        return Intensity;
    }

    bool LightSceneProxy::
        IsCastShadow() const
    {
        return bCastShadow;
    }

    DirectionalLightSceneProxy::
        DirectionalLightSceneProxy(
            const DirectionalLightComponent*
            component)
        :
        LightSceneProxy(component)
    {
    }

    ELightType
        DirectionalLightSceneProxy::
        GetLightType() const
    {
        return ELightType::Directional;
    }

    LocalLightSceneProxy::
        LocalLightSceneProxy(
            const LocalLightComponent*
            component)
        :
        LightSceneProxy(component)
    {
        AttenuationRadius =
            component->GetAttenuationRadius();
    }

    void LocalLightSceneProxy::
        UpdateFromComponent(
            const LightComponent*
            component)
    {
        LightSceneProxy::
            UpdateFromComponent(
                component);

        auto localLight =
            static_cast<
            const LocalLightComponent*>(
                component);

        AttenuationRadius =
            localLight->
            GetAttenuationRadius();
    }

    float LocalLightSceneProxy::
        GetAttenuationRadius()
        const
    {
        return AttenuationRadius;
    }

    PointLightSceneProxy::
        PointLightSceneProxy(
            const PointLightComponent*
            component)
        :
        LocalLightSceneProxy(
            component)
    {
    }

    ELightType
        PointLightSceneProxy::
        GetLightType() const
    {
        return ELightType::Point;
    }

    SpotLightSceneProxy::
        SpotLightSceneProxy(
            const SpotLightComponent*
            component)
        :
        LocalLightSceneProxy(
            component)
    {
        InnerConeAngle =
            component->
            GetInnerConeAngle();

        OuterConeAngle =
            component->
            GetOuterConeAngle();
    }

    void SpotLightSceneProxy::
        UpdateFromComponent(
            const LightComponent*
            component)
    {
        LocalLightSceneProxy::
            UpdateFromComponent(
                component);

        auto spot =
            static_cast<
            const SpotLightComponent*>(
                component);

        InnerConeAngle =
            spot->GetInnerConeAngle();

        OuterConeAngle =
            spot->GetOuterConeAngle();
    }

    ELightType
        SpotLightSceneProxy::
        GetLightType() const
    {
        return ELightType::Spot;
    }

    float SpotLightSceneProxy::
        GetInnerConeAngle()
        const
    {
        return InnerConeAngle;
    }

    float SpotLightSceneProxy::
        GetOuterConeAngle()
        const
    {
        return OuterConeAngle;
    }

    SkyLightSceneProxy::
        SkyLightSceneProxy(
            const SkyLightComponent*
            component)
        :
        LightSceneProxy(component)
    {
    }

    void SkyLightSceneProxy::UpdateFromComponent(const LightComponent* component)
    {
        LightSceneProxy::UpdateFromComponent(component);

        auto sky = static_cast<const SkyLightComponent*>(component);
        if (sky)
        {
            EnvironmentMap = sky->GetEnvironmentMap();
            DiffuseIrradianceMap = sky->GetDiffuseIrradiance();
            SpecularPrefilterMap = sky->GetSpecularPrefilter();
            bPrecomputeDirty = sky->IsPrecomputeDirty();
        }
    }

    ELightType
        SkyLightSceneProxy::
        GetLightType() const
    {
        return ELightType::Sky;
    }

}