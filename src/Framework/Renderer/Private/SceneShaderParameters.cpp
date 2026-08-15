#include "SceneShaderParameters.h"
#include "RenderGraphBuilder.h"
#include "Scene.h"
#include "IBLPrecomputeShader.h"
namespace Renderer {


    void BuildShaderParameters(
        Scene*
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
        Out.LightParameters.LinearClampSampler = RenderCore::GlobalSampler.get();
        //填充阴影参数
        Out.LightShadowParameters.NearestSampler = RenderCore::GlobalNearestSampler.get();
        auto altasTexture = Scene->GetShadowMapAllocator().GetShadowAtlas();
        auto pointLightShadowTextures = Scene->GetShadowMapAllocator().GetDedicatedPointLightShadowTextures();
        auto parallelLightShadowTextures = Scene->GetShadowMapAllocator().GetDedicatedParallelLightShadowTextures();
        int i = 0;
        for (auto& t : pointLightShadowTextures) {
            if (i > 7) break;
            auto rdgT = Builder.RegisterExternalTexture("PointLightShadowTexture", t);
            Out.LightShadowParameters.PointLightShadows[i] = rdgT;
            i++;
        }
        auto unused2D = Builder.RegisterExternalTexture("Unused2DTexture", RenderCore::GlobalTestTexture.get());
        auto unusedCube = Builder.RegisterExternalTexture("UnusedCubeTexture", RenderCore::GlobalEmptyCubeTexture.get());
        auto unused2DArray = Builder.RegisterExternalTexture("UnusedTexture2dArray", RenderCore::GlobalEmptyTexture2DArray.get());
        for (; i < 8; i++) {
            Out.LightShadowParameters.PointLightShadows[i] = unusedCube;
        }
        i = 0;
        for (auto& t : parallelLightShadowTextures) {
            if (i > 3) break;
            auto rdgT = Builder.RegisterExternalTexture("ParallelLightShadowTexture", t);
            Out.LightShadowParameters.ParrallelLightShadows[i] = rdgT;
            i++;
        }
        for (; i < 4; i++) {
            Out.LightShadowParameters.ParrallelLightShadows[i] = unused2DArray;
        }
        auto rdgT = Builder.RegisterExternalTexture("ShadowAtlasTexture", altasTexture);
        Out.LightShadowParameters.LightShadowAtlas = rdgT;
        auto rdgB = Builder.RegisterExternalBuffer("AtlasAccessTextureInfos", Scene->GetGPUResourceInfo().ShadowResourceInfo.AtlasAccessInfoBuffer.get());
        RenderCore::RenderGraphBufferSRVDesc SRVDesc;
        SRVDesc.NumElements = 1;
        SRVDesc.Stride = Scene->GetGPUResourceInfo().ShadowResourceInfo.AtlasAccessInfoBuffer->GetRHI()->GetDesc().Size;
        SRVDesc.Buffer = rdgB;
        Out.LightShadowParameters.AtlasTextureInfos = Builder.CreateBufferSRV("AtlasAccessTextureInfosSRV", SRVDesc);
        rdgB = Builder.RegisterExternalBuffer("LightShadowAccessInfos", Scene->GetGPUResourceInfo().ShadowResourceInfo.LightShadowInfoBuffer.get());
        SRVDesc.NumElements = 1;
        SRVDesc.Stride = Scene->GetGPUResourceInfo().ShadowResourceInfo.LightShadowInfoBuffer->GetRHI()->GetDesc().Size;
        SRVDesc.Buffer = rdgB;
        Out.LightShadowParameters.LightShadowInfos = Builder.CreateBufferSRV("LightShadowAccessInfosSRV", SRVDesc);
        rdgB = Builder.RegisterExternalBuffer("DirectionalLightShadowViewInfos", Scene->GetGPUResourceInfo().ShadowResourceInfo.DirectionalLightShadowViewInfoBuffer.get());
        SRVDesc.NumElements = 1;
        SRVDesc.Stride = Scene->GetGPUResourceInfo().ShadowResourceInfo.DirectionalLightShadowViewInfoBuffer->GetRHI()->GetDesc().Size;
        SRVDesc.Buffer = rdgB;
        Out.LightShadowParameters.DirectionalLightViewInfos = Builder.CreateBufferSRV("DirectionalLightShadowViewInfosSRV", SRVDesc);
        
        auto& SplitBuffer = Scene->GetGPUResourceInfo().ShadowResourceInfo.SplitBuffer;
        auto splitBuffer = Builder.RegisterExternalBuffer("SplitBuffer", SplitBuffer.get());
        
        RenderCore::RenderGraphBufferSRVDesc Desc;
        Desc.Buffer = splitBuffer;
        Desc.Format = ERHIFormat::R32_Float;
        Desc.NumElements = splitBuffer->GetRHIBuffer()->GetDesc().Size / sizeof(float);
        Desc.Stride = sizeof(float);
        auto splitBufferSRV = Builder.CreateBufferSRV("SplitBufferSRV", Desc);
		Out.LightShadowParameters.SplitBuffer = splitBufferSRV;

        //填充距离场参数
		

    }

    void BuildGlobalDistanceFieldParameters(Scene* Scene, RenderCore::RenderGraphBuilder& Builder, SceneGlobalDistanceFieldParameters& Out)
    {
        auto& DistanceFieldResourceInfo = Scene->GetGPUResourceInfo().DistanceFieldResourceInfo;
        Out.BlockSize = Core::Float3(5, 5, 5);
        Out.DistanceFieldVoxelSize = 5 / 64.0;
        Out.DistanceSampler = RenderCore::GlobalSampler.get();
        Out.GlobalDistanceFieldAtlasResolution = Core::UInt3(256, 256, 256);
        Out.GlobalDistanceFieldGridSize = DistanceFieldResourceInfo.StaticDistanceField.GetGridSize();
        Out.GlobalDistanceFieldOrigin = DistanceFieldResourceInfo.StaticDistanceField.GetOrigin();
        auto rdgGlobalDistenctAtlas = Builder.RegisterExternalTexture("GlobalDistanceFieldAtlas", DistanceFieldResourceInfo.StaticDistanceField.GetAtlas().GetAtlasTexture());
        Out.GlobalDistanceFieldAtlas = rdgGlobalDistenctAtlas;
        Out.AllocateBlockIndexInfos = DistanceFieldResourceInfo.StaticDistanceField.GetBlockIndexBufferSRV();
    }

    void BuildEvnIBLLightParameters(Scene* Scene, RenderCore::RenderGraphBuilder& Builder, SceneEvnIBLLightParameters& Out)
    {
        const auto& SceneLightResourceInfo = Scene->GetGPUResourceInfo().LightResourceInfo;
        if (SceneLightResourceInfo.IBLDiffuseTexture != nullptr && SceneLightResourceInfo.IBLSpecularTexture != nullptr) {


            auto rdgT = Builder.RegisterExternalTexture("IBLDiffuseTexture", SceneLightResourceInfo.IBLDiffuseTexture);
            Out.IBLDiffuseMap = rdgT;
            rdgT = Builder.RegisterExternalTexture("IBLSpecularTexture", SceneLightResourceInfo.IBLSpecularTexture);
            Out.IBLSpecularMap = rdgT;
            rdgT = Builder.RegisterExternalTexture("IBLLutTexture", GlobalIBLLutTexture.get());
            Out.LinearClampIBLSampler = RenderCore::GlobalSampler.get();
            Out.IBLLut = rdgT;
        }
        else {
            RenderCore::RenderGraphTextureDesc emptyCubeMapDesc;
            emptyCubeMapDesc.Type = RHI::ERHITextureType::TextureCube;
            emptyCubeMapDesc.Width = 1;
            emptyCubeMapDesc.Height = 1;
            emptyCubeMapDesc.ArraySize = 6;
            emptyCubeMapDesc.MipLevels = 1;
            emptyCubeMapDesc.Usage = RHI::ERHITextureCreateFlag::ShaderResource;
            emptyCubeMapDesc.Format = RHI::ERHIFormat::R8G8B8A8_UNorm;
            Out.LinearClampIBLSampler = RenderCore::GlobalSampler.get();
            auto rdgT = Builder.CreateTexture("EmptyCubeMap", emptyCubeMapDesc);
            Out.IBLDiffuseMap = rdgT;
            Out.IBLSpecularMap = rdgT;
            rdgT = Builder.RegisterExternalTexture("IBLLutTexture", GlobalIBLLutTexture.get());
            Out.IBLLut = rdgT;

        }
    }









}