#pragma once
#include "ShaderParameter.h"
#include "Math.hpp"
namespace Engine {

    BEGIN_SHADER_PARAMETER_STRUCT(LightCommonData)
        SHADER_PARAMETER(Core::Float3,Color)
        SHADER_PARAMETER(float,Intensity)
    END_SHADER_PARAMETER_STRUCT(LightCommonData)

    BEGIN_SHADER_PARAMETER_STRUCT(DirectionalLightData)

        SHADER_PARAMETER_STRUCT_INCLUDE( LightCommonData, Common )

        SHADER_PARAMETER(Core::Float3,Direction)

        SHADER_PARAMETER(float,ShadowIndex)

    END_SHADER_PARAMETER_STRUCT(DirectionalLightData)


    BEGIN_SHADER_PARAMETER_STRUCT(PointLightData)
        SHADER_PARAMETER_STRUCT_INCLUDE(LightCommonData,Common)
        SHADER_PARAMETER(Core::Float3,Position)
        SHADER_PARAMETER(float,Radius)

    END_SHADER_PARAMETER_STRUCT(PointLightData)

    BEGIN_SHADER_PARAMETER_STRUCT(SpotLightData)
        SHADER_PARAMETER_STRUCT_INCLUDE(LightCommonData,Common)
        SHADER_PARAMETER(Core::Float3,Position)
        SHADER_PARAMETER(float,Radius)
        SHADER_PARAMETER(Core::Float3,Direction)
        SHADER_PARAMETER(float,InnerConeCos)
        SHADER_PARAMETER(float,OuterConeCos)
    END_SHADER_PARAMETER_STRUCT(SpotLightData)

    BEGIN_SHADER_PARAMETER_STRUCT(SceneLightParameters)
        SHADER_PARAMETER(uint32_t,DirectionalLightCount)
        SHADER_PARAMETER(uint32_t,PointLightCount)
        SHADER_PARAMETER(uint32_t,SpotLightCount)
        SHADER_PARAMETER_RDG_STRUCTURED_BUFFER(DirectionalLightData,DirectionalLights)
        SHADER_PARAMETER_RDG_STRUCTURED_BUFFER(PointLightData,PointLights)
        SHADER_PARAMETER_RDG_STRUCTURED_BUFFER(SpotLightData,SpotLights)
    END_SHADER_PARAMETER_STRUCT(SceneLightParameters)

	BEGIN_SHADER_PARAMETER_STRUCT(SceneShaderParameters)
        SHADER_PARAMETER_STRUCT_REFERENCE(SceneLightParameters, LightParameters)
    END_SHADER_PARAMETER_STRUCT(SceneShaderParameters)


}
