#include "SceneShaderParameters.h"
#include "RenderGraphBuilder.h"
#include "Scene.h"
#include "IBLPrecomputeShader.h"
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
        //Out.LightParameters.PointLightCount = 0;
        //Out.LightParameters.SpotLightCount = 0;
        //Out.LightParameters.DirectionalLightCount = 0;
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
        if (SceneLightResourceInfo.IBLDiffuseTexture != nullptr && SceneLightResourceInfo.IBLSpecularTexture != nullptr) {


            auto rdgT = Builder.RegisterExternalTexture("IBLDiffuseTexture", SceneLightResourceInfo.IBLDiffuseTexture);
            Out.LightParameters.IBLDiffuseMap = rdgT;
            rdgT = Builder.RegisterExternalTexture("IBLSpecularTexture", SceneLightResourceInfo.IBLSpecularTexture);
            Out.LightParameters.IBLSpecularMap = rdgT;
            rdgT = Builder.RegisterExternalTexture("IBLLutTexture", GlobalIBLLutTexture.get());
            Out.LightParameters.LinearClampSampler = RenderCore::GlobalSampler.get();
            Out.LightParameters.IBLLut = rdgT;
            Out.LightParameters.EnableIBLMap = 1;
        }else{
            RenderCore::RenderGraphTextureDesc emptyCubeMapDesc;
            emptyCubeMapDesc.Type = RHI::ERHITextureType::TextureCube;
            emptyCubeMapDesc.Width = 1;
            emptyCubeMapDesc.Height = 1;
            emptyCubeMapDesc.ArraySize = 6;
            emptyCubeMapDesc.MipLevels = 1;
            emptyCubeMapDesc.Usage = RHI::ERHITextureCreateFlag::ShaderResource;
            emptyCubeMapDesc.Format = RHI::ERHIFormat::R8G8B8A8_UNorm;
            Out.LightParameters.LinearClampSampler = RenderCore::GlobalSampler.get();
            auto rdgT = Builder.CreateTexture("EmptyCubeMap", emptyCubeMapDesc);
            Out.LightParameters.IBLDiffuseMap = rdgT;
            Out.LightParameters.IBLSpecularMap = rdgT;
            rdgT = Builder.RegisterExternalTexture("IBLLutTexture", GlobalIBLLutTexture.get());
            Out.LightParameters.IBLLut = rdgT;
            Out.LightParameters.EnableIBLMap = 0;

        }
    }









}