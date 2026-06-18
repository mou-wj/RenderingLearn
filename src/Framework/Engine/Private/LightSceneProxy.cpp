#include "LightSceneProxy.h"
#include "Transform.hpp"

namespace Engine
{

    LightSceneProxy::
        LightSceneProxy(
            const LightComponent* component) : Component(component)
    {
        
    }

    void LightSceneProxy::
        UpdateFromComponent()
    {
        
        Position =
            Component->GetWorldLocation();
        Color =
            Component->GetColor();

        Intensity =
            Component->GetIntensity();

        bCastShadow =
            Component->IsCastShadow();
    }

    const Core::Float3&
        LightSceneProxy::
        GetPosition() const
    {
        return Position;
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
        UpdateFromComponent();
    }

    void DirectionalLightSceneProxy::
        UpdateFromComponent()
    {
        LightSceneProxy::UpdateFromComponent();

        auto dir = static_cast<const DirectionalLightComponent*>(Component);
        if (dir)
        {
            Direction = dir->GetDirection();
        }
    }

    const Core::Float3& DirectionalLightSceneProxy::GetDirection() const
    {
        return Direction;
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
        UpdateFromComponent()
    {
        LightSceneProxy::
            UpdateFromComponent();

        auto localLight =
            static_cast<
            const LocalLightComponent*>(
                Component);

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
        UpdateFromComponent();
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
        Direction = component->GetDirection();
        UpdateFromComponent();
    }

    void SpotLightSceneProxy::
        UpdateFromComponent()
    {
        LocalLightSceneProxy::
            UpdateFromComponent();

        auto spot =
            static_cast<
            const SpotLightComponent*>(
                Component);

        InnerConeAngle =
            spot->GetInnerConeAngle();

        OuterConeAngle =
            spot->GetOuterConeAngle();
        Direction = spot->GetDirection();
    }

    const Core::Float3& SpotLightSceneProxy::GetDirection() const
    {
        return Direction;
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
        UpdateFromComponent();
    }

    void SkyLightSceneProxy::UpdateFromComponent()
    {
        LightSceneProxy::UpdateFromComponent();

        auto sky = static_cast<const SkyLightComponent*>(Component);
        if (sky)
        {
            DiffuseIrradianceMap = sky->GetDiffuseIrradiance();
            SpecularPrefilterMap = sky->GetSpecularPrefilter();
        }
    }

    ELightType
        SkyLightSceneProxy::
        GetLightType() const
    {
        return ELightType::Sky;
    }

}