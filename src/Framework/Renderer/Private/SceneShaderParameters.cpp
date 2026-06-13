#include "SceneShaderParameters.h"
#include "RenderGraphBuilder.h"
#include "Scene.h"
namespace Renderer {


    void BuildShaderParameters(
        const Scene*
        Scene,
        RenderCore::RenderGraphBuilder& Builder,
        SceneShaderParameters&
        Out) {
        const auto& SceneLightResourceInfo = Scene->GetGPUResourceInfo().LightResourceInfo;
        Out.LightParameters.PointLightCount = SceneLightResourceInfo.PointLightCount;
		Out.LightParameters.SpotLightCount = SceneLightResourceInfo.SpotLightCount;
		Out.LightParameters.DirectionalLightCount = SceneLightResourceInfo.DirectionalLightCount;
        if (SceneLightResourceInfo.PointLightBuffer) {
            auto rdgB = Builder.RegisterExternalBuffer(
                "PointLights",
                SceneLightResourceInfo.PointLightBuffer.get());
            RenderCore::RenderGraphBufferSRVDesc SRVDesc;
            SRVDesc.NumElements = 1;
            SRVDesc.Stride = SceneLightResourceInfo.PointLightBuffer->GetRHI()->GetDesc().Size;
            SRVDesc.Buffer = rdgB;
            Out.LightParameters.PointLights = Builder.CreateBufferSRV("PointLightsSRV", SRVDesc);
        }
        if (SceneLightResourceInfo.SpotLightBuffer) {
            auto rdgB = Builder.RegisterExternalBuffer(
                "SpotLights",
                SceneLightResourceInfo.SpotLightBuffer.get());
            RenderCore::RenderGraphBufferSRVDesc SRVDesc;
            SRVDesc.NumElements = 1;
            SRVDesc.Stride = SceneLightResourceInfo.SpotLightBuffer->GetRHI()->GetDesc().Size;
            SRVDesc.Buffer = rdgB;
            Out.LightParameters.SpotLights = Builder.CreateBufferSRV("SpotLightsSRV", SRVDesc);
        }
        if (SceneLightResourceInfo.DirectionalLightBuffer) {
            auto rdgB = Builder.RegisterExternalBuffer(
                "DirectionalLights",
                SceneLightResourceInfo.DirectionalLightBuffer.get());
            RenderCore::RenderGraphBufferSRVDesc SRVDesc;
            SRVDesc.NumElements = 1;
            SRVDesc.Stride = SceneLightResourceInfo.DirectionalLightBuffer->GetRHI()->GetDesc().Size;
            SRVDesc.Buffer = rdgB;
            Out.LightParameters.DirectionalLights = Builder.CreateBufferSRV("DirectionalLightsSRV", SRVDesc);
        }

    }









}