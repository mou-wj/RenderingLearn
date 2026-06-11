#include "LightComponent.h"
#include "LightSceneProxy.h"
#include "RenderResource.h"
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

    LightSceneProxy* DirectionalLightComponent::CreateSceneProxy()
    {
        return new DirectionalLightSceneProxy(this);
    }

    ELightType
        DirectionalLightComponent::
        GetLightType() const
    {
        return ELightType::Directional;
    }

    void DirectionalLightComponent::
        SetDirection(
            const Core::Float3& direction)
    {
        Direction = direction;

        MarkRenderStateDirty();
    }

    const Core::Float3&
        DirectionalLightComponent::
        GetDirection() const
    {
        return Direction;
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

    LightSceneProxy* PointLightComponent::CreateSceneProxy()
    {
        return new PointLightSceneProxy(this);
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

    LightSceneProxy* SpotLightComponent::CreateSceneProxy()
    {
        return new SpotLightSceneProxy(this);
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

    void SpotLightComponent::
        SetDirection(
            const Core::Float3& direction)
    {
        Direction = direction;

        MarkRenderStateDirty();
    }

    const Core::Float3&
        SpotLightComponent::
        GetDirection() const
    {
        return Direction;
    }

    SkyLightComponent::
        SkyLightComponent()
    {
    }

    void SkyLightComponent::SetEnvironmentMap(RenderCore::RenderTexture* InEnvMap)
    {
        EnvironmentMap = InEnvMap;
        // 标记需要重新预计算
        bPrecomputeDirty = true;

        // 通知渲染线程需要更新
        MarkRenderStateDirty();
    }

    LightSceneProxy* SkyLightComponent::CreateSceneProxy()
    {
        return new SkyLightSceneProxy(this);
    }

    ELightType
        SkyLightComponent::
        GetLightType() const
    {
        return ELightType::Sky;
    }

}