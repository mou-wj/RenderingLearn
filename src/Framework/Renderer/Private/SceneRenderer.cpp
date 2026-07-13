#include "SceneRenderer.h"
#include "StaticMeshMaterialShader.h"
#include "LightSceneProxy.h"
#include "Transform.hpp"
#include "StaticMeshProcess.h"
#include "StaticMeshProxy.h"
#include "MaterialCore.h"
#include "RHIPipelineStateCache.h"
#include "SceneShaderParameters.h"
using namespace RenderCore;
using namespace Engine;
namespace Renderer {
    
    void SceneRenderer::AddClearRenderTargetsPass(RenderCore::RenderGraphBuilder& bulder, RenderCore::RenderTargetBindingSlots& renderTargetBindingSlots)
    {
        BEGIN_SHADER_PARAMETER_STRUCT(ClearTargetParameters)
            SHADER_PARAMETER_RENDER_TARGET_BINDING_SLOTS(renderTargetSlots)
        END_SHADER_PARAMETER_STRUCT(ClearTargetParameters)
        auto params = bulder.AllocateParameter<ClearTargetParameters>();
        params->renderTargetSlots = renderTargetBindingSlots;
        bulder.AddPass<ClearTargetParameters>(
            "ClearTargetPass",
            ClearTargetParameters::GetMetaData(),
            params,
            EPassFlag::Graphic,
            [=](RHI::RHICommandListBase& RHICmdList)
            {
                auto& cmd = static_cast<RHI::RHIGraphicCommandList&>(RHICmdList);
                auto boundRenderTarget = params->renderTargetSlots.GetBoundRenderTarget();
                RHIRenderPassInfo passInfo;
                passInfo.RenderTargets = boundRenderTarget;
                passInfo.RenderTargets.DepthStencil.ClearBinding.Depth = 1.0f;
                passInfo.RenderArea.Width = boundRenderTarget.Dimensions.x;
                passInfo.RenderArea.Height = boundRenderTarget.Dimensions.y;

                cmd.BeginRenderPass(passInfo);
                cmd.SetViewport(
                    0,
                    0,
                    boundRenderTarget.Dimensions.x,
                    boundRenderTarget.Dimensions.y,
                    0.0f,
                    1.0f);
                cmd.SetScissor(0, 0, boundRenderTarget.Dimensions.x, boundRenderTarget.Dimensions.y);
                cmd.EndRenderPass();
            });

    }
    void SceneRenderer::BuildSceneLightShadowMap(RenderCore::RenderGraphBuilder& builder)
    {
        std::vector<ShadowRenderView> shadowRenderViews;

        Scene->ForEachLight(
            [this, &shadowRenderViews,&builder](Engine::LightSceneProxy* light)
            {
                if (!light)
                    return;

                if (!light->IsCastShadow())
                    return;
                if (!light->bUpdateCastShadow)
                    return;
                light->bUpdateCastShadow = false;
                auto lightType = light->GetLightType();
                auto& allocator = Scene->GetShadowMapAllocator();

                switch (lightType)
                {
                    //---------------------------------------------------
                    // Spot Light
                    //---------------------------------------------------
                case Engine::ELightType::Spot:
                {
                    
                    auto* spotLight =
                        dynamic_cast<Engine::SpotLightSceneProxy*>(light);

                    if (!spotLight)
                        return;
                    ShadowAllocation allocation;

                    LightShadowInfo& shadowInfo = Scene->GetLightShadowInfo(light);
                    if (!shadowInfo.Allocation.IsValid()) {
                        allocation = allocator.AllocateSpotShadow(1024);
                        shadowInfo.Allocation = allocation;
                        if (!allocation.IsValid())
                            return;
                    }
                    else {
                        allocation = shadowInfo.Allocation;
                    }
                    ShadowRenderView view;
                    auto rdgT = builder.RegisterExternalTexture(std::string("ShadowPassTarget") + std::to_string((uint64_t)allocation.Texture), allocation.Texture);
                    view.Allocation.Texture = rdgT;
                    view.Allocation.X = allocation.Slices[0].X;
                    view.Allocation.Y = allocation.Slices[0].Y;
                    view.Allocation.Width = allocation.Slices[0].Width;
					view.Allocation.Height = allocation.Slices[0].Height;


                    shadowInfo.ShadowMatrices.resize(1);
                    auto proj =
                        Core::PerspectiveRH_ZO(
                            spotLight->GetOuterConeAngle() * 2.0f,
                            1.0f,
                            0.1f,
                            spotLight->GetAttenuationRadius());

                    auto viewMat =
                        Core::LookAtRH(
                            spotLight->GetPosition(),
                            spotLight->GetPosition() +
                            spotLight->GetDirection(),
                            Core::Float3(0, 1, 0));

                    view.ViewProjection =
                        proj * viewMat;
                    shadowInfo.ShadowMatrices[0] = view.ViewProjection;
                    Core::Float4 test = viewMat * Core::Float4(3, 3, 3, 1).Data;
                    Core::Float4 restest = proj * test.Data;
                    float z = restest.z / restest.w;
                    view.TargetHeight = allocator.GetDesc().SpotShadowAtlas.Width;
                    view.TargetWidth = allocator.GetDesc().SpotShadowAtlas.Height;
                    shadowRenderViews.push_back(view);
                    view.CameraPos = spotLight->GetPosition();
                    view.wantRawDepth = false;
                    break;
                }

                //---------------------------------------------------
                // Point Light (6 faces)
                //---------------------------------------------------
                case Engine::ELightType::Point:
                {
                    
                    auto* pointLight =
                        dynamic_cast<Engine::PointLightSceneProxy*>(light);

                    if (!pointLight)
                        return;

                    ShadowAllocation allocation; 


                    LightShadowInfo& shadowInfo = Scene->GetLightShadowInfo(light);
                    if (!shadowInfo.Allocation.IsValid()) {
                        allocation = allocator.AllocatePointShadow(1024);
                        shadowInfo.Allocation = allocation;
                        if (!allocation.IsValid())
                            return;
                    }
                    else {
                        allocation = shadowInfo.Allocation;
                    }
                    
                    
                    static Core::Float3 directions[6] =
                    {
                        { 1, 0, 0 },
                        {-1, 0, 0 },
                        { 0, 1, 0 },
                        { 0,-1, 0 },
                        { 0, 0, 1 },
                        { 0, 0,-1 }
                    };

                    //static Core::Float3 ups[6] =
                    //{
                    //    {0,1,0},
                    //    {0,1,0},
                    //    {0,0,1},
                    //    {0,0,-1},
                    //    {0,1,0},
                    //    {0,1,0}
                    //};
                    static Core::Float3 ups[6] =
                    {
                        { 0,-1,0 }, // +X
                        { 0,-1,0 }, // -X

                        { 0,0,1 },  // +Y
                        { 0,0,-1 }, // -Y

                        { 0,-1,0 }, // +Z
                        { 0,-1,0 }  // -Z
                    };
                    float radius = pointLight->GetAttenuationRadius();
                    auto proj =
                        Core::PerspectiveRH_ZO(
                            Core::DegToRad(90.0f),
                            1.0f,
                            0.1f,
                            radius);
                    
                    shadowInfo.ShadowMatrices.resize(6);
                    for (uint32_t i = 0; i < 6; i++)
                    {
                        ShadowRenderView view;
                        auto rdgT = builder.RegisterExternalTexture(std::string("ShadowPassTarget") + std::to_string((uint64_t)allocation.Texture), allocation.Texture);
                        view.Allocation.Texture = rdgT;
                        view.Allocation.X = allocation.Slices[i].X;
                        view.Allocation.Y = allocation.Slices[i].Y;
                        view.Allocation.Width = allocation.Slices[i].Width;
                        view.Allocation.Height = allocation.Slices[i].Height;
						view.Allocation.Layer = allocation.Slices[i].Layer;
						view.Allocation.Mip = allocation.Slices[i].Mip;
                        auto viewMat =
                            Core::LookAtRH(
                                pointLight->GetPosition(),
                                pointLight->GetPosition() + directions[i],
                                ups[i]);

                        view.ViewProjection =
                            proj * viewMat;
                        if (RHI::GRHIApi->GetPlatformInfo().NDCToUVScaleBias.y < 0) {
                            ;
                            view.ViewProjection = Core::MakeScaleMatrix(Core::Float3(1, -1, 1)) * view.ViewProjection;
                        }
                        Core::Float4 test = viewMat * Core::Float4(3, 3, 3, 1).Data;
                        Core::Float4 restest = proj * test.Data;

                        float z = restest.z / restest.w;
                        shadowInfo.ShadowMatrices[i] = view.ViewProjection;
                        view.CameraPos = pointLight->GetPosition();
                        view.wantRawDepth = true;
                        view.RawDepthClearValue = Core::Float4(radius, radius,0,0);
                        shadowRenderViews.push_back(view);
                    }
                    
                    break;
                }

                default:
                    break;
                }
            });

        if (!shadowRenderViews.empty())
        {
            BuildLightShadow(builder, shadowRenderViews);
        }
    }
    void SceneRenderer::BuildSceneLightCascadeShadowMap(RenderCore::RenderGraphBuilder& builder, const SceneView& sceneView)
    {
        std::vector<ShadowRenderView> shadowRenderViews;

        Scene->ForEachLight(
            [this, &shadowRenderViews,&sceneView,&builder](Engine::LightSceneProxy* light)
            {
                if (!light)
                    return;
                
                if (!light->IsCastShadow())
                    return;
                auto lightType = light->GetLightType();
                auto& allocator = Scene->GetShadowMapAllocator();

                switch (lightType)
                {
                //---------------------------------------------------
                // Directional Light (CSM)
                //---------------------------------------------------
                case Engine::ELightType::Directional:
                {
                    auto* dirLight =
                        dynamic_cast<Engine::DirectionalLightSceneProxy*>(light);

                    if (!dirLight)
                        return;

                    constexpr uint32_t CascadeCount = 4;
                    const std::array<float, 5>& splitDepths = sceneView.splitDepths;
                    
					LightShadowInfo& shadowInfo = Scene->GetLightShadowInfo(light);
                    ShadowAllocation allocation;
                    if (!shadowInfo.Allocation.IsValid()) {
                        allocation = allocator.AllocateDirectionalCSMShadow(
                            2048,
                            CascadeCount);
						shadowInfo.Allocation = allocation;
                        if (!allocation.IsValid())
                            return;
                    }
                    else {
                        allocation = shadowInfo.Allocation;
                    }

                    shadowInfo.ShadowMatrices.resize(CascadeCount);
                    
                    auto& sceneBound = Scene->GetSceneBounds();

                    for (uint32_t i = 0; i < CascadeCount; i++)
                    {

                        ShadowRenderView view;
                        auto rdgT = builder.RegisterExternalTexture(std::string("ShadowPassTarget") + std::to_string((uint64_t)allocation.Texture), allocation.Texture);
                        view.Allocation.Texture = rdgT;
                        view.Allocation.X = allocation.Slices[i].X;
                        view.Allocation.Y = allocation.Slices[i].Y;
                        view.Allocation.Width = allocation.Slices[i].Width;
                        view.Allocation.Height = allocation.Slices[i].Height;
                        view.Allocation.Layer = allocation.Slices[i].Layer;
                        view.Allocation.Mip = allocation.Slices[i].Mip;

                        auto corners =
                            sceneView.GetFrustumCornersWS(
                                splitDepths[i],
                                splitDepths[i + 1]);

                        Core::Float3 center(0, 0, 0);
                        for (auto& c : corners)
                            center += c;
                        center /= 8.0f;


                        auto lightDir =
                            dirLight->GetDirection();

                        auto lightPos =
                            center - lightDir * 200.0f;

                        auto lightView =
                            Core::LookAtRH(
                                lightPos,
                                center,
                                Core::Float3(0, 1, 0));

                        Core::AABB bounds;
                        
                        for (auto& corner : corners)
                        {
                            Core::Float4 p =
                                lightView *
                                Core::Float4(corner, 1.0f).Data;

                            bounds.ExpandBy(
                                Core::Float3(p.x, p.y, p.z));
                        }
                        //bounds.Min.z -= ExtendNear;
                        //bounds.Min.z -= 10;
                        auto minz = CORE_MIN(bounds.Min.z, 0);
                        auto maxz = CORE_MAX(bounds.Max.z, 0);
                        assert(minz < 0);
                        
                        auto proj =
                            Core::OrthoRHBounds_ZO(
                                bounds.Min.x,
                                bounds.Max.x,
                                bounds.Min.y,
                                bounds.Max.y,
                                maxz,
                                minz);


                        view.ViewProjection =
                            proj * lightView;
                        shadowInfo.ShadowMatrices[i] = view.ViewProjection;
                        view.CameraPos = lightPos;
                        view.wantRawDepth = false;
                        shadowRenderViews.push_back(view);
                    }
                    

                    break;
                }
                default:
                    break;
                }
            });

        if (!shadowRenderViews.empty())
        {
            BuildLightShadow(builder, shadowRenderViews);
        }
    }
    void SceneRenderer::UploadShadowMapInfo(RenderCore::RenderGraphBuilder& graphBuilder)
    {
		auto lightCount = Scene->LightIndexs.size();
        std::vector<LightShadowAccessInfo> lightShadowAccessInfos(lightCount);
        lightShadowAccessInfos.reserve(lightCount);
        std::vector<AtlasShadowTextureAccessInfo> spotShadowAccessInfos;
        std::vector<DirectionalLightCascadeShadowViewInfo> directionalLightShadowViewInfos;
        //
        int atlasIndex = 0, directionalLightShadowViewInfoIndex = 0;
        for (auto& shadowInfo : Scene->GPUResourceInfo.ShadowResourceInfo.LightShadowInfos) {
			auto index = Scene->LightIndexs[shadowInfo.first];
            LightShadowAccessInfo& lightShadowAccessInfo = lightShadowAccessInfos[index];
            lightShadowAccessInfo.ShadowType = (uint32_t)shadowInfo.second.Allocation.ShadowType;
            AtlasShadowTextureAccessInfo spotShadowAccessInfo;
            DirectionalLightCascadeShadowViewInfo directionalLightShadowViewInfo;
            if (shadowInfo.second.Allocation.ShadowType == EShadowType::Spot) {
                lightShadowAccessInfo.ShadowInfoIndex = atlasIndex;
				spotShadowAccessInfo.Layer = shadowInfo.second.Allocation.Slices[0].Layer;
				spotShadowAccessInfo.mip = shadowInfo.second.Allocation.Slices[0].Mip;
				spotShadowAccessInfo.UVScale = shadowInfo.second.Allocation.Slices[0].UVScale;
				spotShadowAccessInfo.UVBias = shadowInfo.second.Allocation.Slices[0].UVOffset;
				spotShadowAccessInfo.ViewProj = shadowInfo.second.ShadowMatrices[0];
                
				spotShadowAccessInfos.push_back(spotShadowAccessInfo);
				atlasIndex++; 
                
			}
			else if (shadowInfo.second.Allocation.ShadowType == EShadowType::Directional) {
				lightShadowAccessInfo.ShadowInfoIndex = directionalLightShadowViewInfoIndex;
				for (uint32_t i = 0; i < shadowInfo.second.ShadowMatrices.size(); i++) {
					directionalLightShadowViewInfo.ViewProj = shadowInfo.second.ShadowMatrices[i];
					directionalLightShadowViewInfos.push_back(directionalLightShadowViewInfo);
				}
				directionalLightShadowViewInfoIndex += shadowInfo.second.ShadowMatrices.size();
                lightShadowAccessInfo.CascadeCount = shadowInfo.second.ShadowMatrices.size();
			}
            
            lightShadowAccessInfo.ShadowTextureIndex = shadowInfo.second.Allocation.TextureIndex;

        }
        auto& LightShadowInfoBuffer = Scene->GPUResourceInfo.ShadowResourceInfo.LightShadowInfoBuffer;
        auto& AtlasAccessInfoBuffer = Scene->GPUResourceInfo.ShadowResourceInfo.AtlasAccessInfoBuffer;
        auto& directionalLightShadowViewInfoBuffer = Scene->GPUResourceInfo.ShadowResourceInfo.DirectionalLightShadowViewInfoBuffer;
        if (LightShadowInfoBuffer == nullptr || LightShadowInfoBuffer->GetRHI()->GetDesc().Size < lightCount * sizeof(LightShadowAccessInfo)) {
            RHI::RHIBufferDesc Desc;
            Desc.Size = lightCount * sizeof(LightShadowAccessInfo);
            if (Desc.Size == 0) {
                Desc.Size = 1;
            }
            Desc.Usage = RHI::ERHIBufferUsageFlag::ShaderResource | RHI::ERHIBufferUsageFlag::TransferDst;
            LightShadowInfoBuffer = std::make_shared<RenderCore::RenderBuffer>(Desc);
            LightShadowInfoBuffer->InitRHIResource();
            auto bufferUpload = graphBuilder.RegisterExternalBuffer("LightShadowInfoBuffer",LightShadowInfoBuffer.get());
			graphBuilder.AddUploadBuffer(bufferUpload, lightShadowAccessInfos.data(), lightCount * sizeof(LightShadowAccessInfo));
        }
        if (AtlasAccessInfoBuffer == nullptr || AtlasAccessInfoBuffer->GetRHI()->GetDesc().Size < spotShadowAccessInfos.size() * sizeof(AtlasShadowTextureAccessInfo)) {
            RHI::RHIBufferDesc Desc;
            Desc.Size = spotShadowAccessInfos.size() * sizeof(AtlasShadowTextureAccessInfo);
            if (Desc.Size == 0) {
                Desc.Size = 1;
            }
            Desc.Usage = RHI::ERHIBufferUsageFlag::ShaderResource | RHI::ERHIBufferUsageFlag::TransferDst;
            AtlasAccessInfoBuffer = std::make_shared<RenderCore::RenderBuffer>(Desc);
            AtlasAccessInfoBuffer->InitRHIResource();
            auto bufferUpload = graphBuilder.RegisterExternalBuffer("AtlasAccessInfoBuffer", AtlasAccessInfoBuffer.get());
            graphBuilder.AddUploadBuffer(bufferUpload, spotShadowAccessInfos.data(), spotShadowAccessInfos.size() * sizeof(AtlasShadowTextureAccessInfo));
        }
		if (directionalLightShadowViewInfoBuffer == nullptr || directionalLightShadowViewInfoBuffer->GetRHI()->GetDesc().Size < directionalLightShadowViewInfos.size() * sizeof(DirectionalLightCascadeShadowViewInfo)) {
			RHI::RHIBufferDesc Desc;
			Desc.Size = directionalLightShadowViewInfos.size() * sizeof(DirectionalLightCascadeShadowViewInfo);
			if (Desc.Size == 0) {
				Desc.Size = 1;
			}
			Desc.Usage = RHI::ERHIBufferUsageFlag::ShaderResource | RHI::ERHIBufferUsageFlag::TransferDst;
			directionalLightShadowViewInfoBuffer = std::make_shared<RenderCore::RenderBuffer>(Desc);
			directionalLightShadowViewInfoBuffer->InitRHIResource();
			auto bufferUpload = graphBuilder.RegisterExternalBuffer("DirectionalLightShadowViewInfoBuffer", directionalLightShadowViewInfoBuffer.get());
			graphBuilder.AddUploadBuffer(bufferUpload, directionalLightShadowViewInfos.data(), directionalLightShadowViewInfos.size() * sizeof(DirectionalLightCascadeShadowViewInfo));
		}
        
    }
    void SceneRenderer::UploadSplits(RenderCore::RenderGraphBuilder& graphBuilder, const Engine::SceneView& view,RenderGraphBufferSRV** out)
	{
		auto& SplitBuffer = Scene->GPUResourceInfo.ShadowResourceInfo.SplitBuffer;
		if (SplitBuffer == nullptr || SplitBuffer->GetRHI()->GetDesc().Size < (view.splitDepths.size() * sizeof(float))) {
			RHI::RHIBufferDesc Desc;
            Desc.DebugName = "SceneSplitBuffer";
			Desc.Size = view.splitDepths.size() * sizeof(float);
			Desc.Usage = RHI::ERHIBufferUsageFlag::ShaderResource | RHI::ERHIBufferUsageFlag::TransferDst;
			SplitBuffer = std::make_shared<RenderCore::RenderBuffer>(Desc);
			SplitBuffer->InitRHIResource();
			
		}
        auto bufferUpload = graphBuilder.RegisterExternalBuffer("SplitBuffer", SplitBuffer.get());
        graphBuilder.AddUploadBuffer(bufferUpload, view.splitDepths.data(), view.splitDepths.size() * sizeof(float), RenderCore::RenderGraphBuilder::EUploadPolicy::Immediate);
        RenderGraphBufferSRVDesc Desc;
		Desc.Buffer = bufferUpload;
		Desc.Format = ERHIFormat::R32_Float;
		Desc.NumElements = view.splitDepths.size();
		Desc.Stride = sizeof(float);
        auto bufferSRV = graphBuilder.CreateBufferSRV("SplitBufferSRV",Desc);
        *out = bufferSRV;
    }
    void SceneRenderer::AddClearShadowMapPass(RenderCore::RenderGraphBuilder& graphBuilder, std::vector<ShadowRenderView>& shadowRenderViews)
    {
        uint16_t groupCount = shadowRenderViews.size() / 8 + 1;
        uint32_t shadowId = 0;
        for (uint32_t i = 0; i < groupCount ; i++) {
            RenderCore::RenderTargetBindingSlots renderTargetBindingSlots;
            uint32_t j = 0;
            for (; j < 8 && shadowId < shadowRenderViews.size(); j++, shadowId++) {
                renderTargetBindingSlots.ColorRenderTargets[j].Action = ERenderTargetActions::Clear_Store;
                renderTargetBindingSlots.ColorRenderTargets[j].ArraySlice = shadowRenderViews[shadowId].Allocation.Layer;
                renderTargetBindingSlots.ColorRenderTargets[j].MipIndex = shadowRenderViews[shadowId].Allocation.Mip;
                renderTargetBindingSlots.ColorRenderTargets[j].Texture = shadowRenderViews[shadowId].Allocation.Texture;
                renderTargetBindingSlots.ColorRenderTargets[j].ClearValue = Core::Float4(1, 1, 1, 1);

            }
            renderTargetBindingSlots.NumColorRenderTargets = j;
            AddClearRenderTargetsPass(graphBuilder, renderTargetBindingSlots);
        }

    }
    void SceneRenderer::BuildLightShadow(RenderCore::RenderGraphBuilder& graphBuilder, std::vector<ShadowRenderView>& shadowRenderViews)
    {

        auto vsShaderType =
            ShaderType::GetRegisterMap()
            [ShaderType::EShaderTypeFlag::MeshMaterial]
            ["StaticMeshMaterialShaderVS"];

        auto psShaderType =
            ShaderType::GetRegisterMap()
            [ShaderType::EShaderTypeFlag::MeshMaterial]
            ["StaticMeshMaterialLightShadowPassPS"];

        auto vfType =
            VertexFactoryType::GetRegisterMap()
            ["LocalVertexFactory"];

        //获取pipeline
        // 创建光栅化状态
        RHI::RHIRasterizerStateDesc rasterizerDesc;
        rasterizerDesc.polygonMode = RHI::ERHIPolygonMode::Fill;
        rasterizerDesc.cullMode = RHI::ERHICullMode::Back;
        rasterizerDesc.frontFace = RHI::ERHIFrontFace::CounterClockwise;
        rasterizerDesc.lineWidth = 1.0f;
        rasterizerDesc.depthBiasEnable = false;

        auto rasterizerState = RHIPipelineStateCache::GetOrCreateRasterizerState(rasterizerDesc);

        // 创建深度模板状态
        RHI::RHIDepthStencilStateDesc depthStencilDesc;
        depthStencilDesc.depthTestEnable = true;
        depthStencilDesc.depthWriteEnable = true;
        depthStencilDesc.depthCompareOp = RHI::ERHICompareOp::Less;

        auto depthStencilState = RHIPipelineStateCache::GetOrCreateDepthStencilState(depthStencilDesc);
        RHI::RHIGraphicsPipelineStateDesc pipelineDesc;
        // 创建图形管线状态
        

        // 这里可以设置更多管线配置...
        pipelineDesc.rasterizerState = rasterizerState;

        pipelineDesc.depthStencilState = depthStencilState;

        //rendertarget info
        pipelineDesc.attachmentDesc.colorAttachmentCount = 1;
        pipelineDesc.attachmentDesc.colorAttachments[0].format = RHI::ERHIFormat::R16G16_Float;
        pipelineDesc.attachmentDesc.colorAttachments[0].actions = ERenderTargetActions::Load_Store;
        pipelineDesc.attachmentDesc.depthActions = ERenderTargetActions::Clear_Store;
        pipelineDesc.attachmentDesc.enableDepth = true;
        pipelineDesc.attachmentDesc.depthStencilFormat = RHI::ERHIFormat::D32_Float;


        /*
        ============================================================
            Shared Shadow Pass
        ============================================================
        */
        uint32_t i = 0;
        for (auto& shadowView : shadowRenderViews)
        {
            //创建临时深度纹理
            RenderGraphTextureDesc desc;
            desc.Width = shadowView.Allocation.Width;
            desc.Height = shadowView.Allocation.Height;
            if (shadowView.TargetHeight != 0 && shadowView.TargetWidth != 0) {
                //重新设置dimention
                desc.Width = shadowView.TargetWidth;
                desc.Height = shadowView.TargetHeight;
            }
            desc.Depth = 1;
            desc.MipLevels = 1;
            desc.ArraySize = 1;

            desc.Format = ERHIFormat::D32_Float;
            desc.Type = ERHITextureType::Texture2D;
            desc.SampleCount = 1;

            desc.Usage =
                ERHITextureCreateFlag::DepthStencil |
                ERHITextureCreateFlag::ShaderResource;
			std::string depthTargetName = std::string("ShadowMapPassDepthTarget") + std::to_string((uint64_t)shadowView.Allocation.Texture) + " curid: " + std::to_string(i);
            desc.bGenerateMips = false;
            desc.DebugName = depthTargetName.c_str();
            auto shadowMapPassDepthTarget = graphBuilder.CreateTexture(depthTargetName,desc);
            i++;

            auto& slice =
                shadowView.Allocation;
            uint32_t psPermutationId = 0;
            if (shadowView.wantRawDepth) {
                psPermutationId = 1;
            }
            MeshBatchList drawMeshBatches;
            std::vector<Engine::StaticMeshProxy*> proxys;
            Scene->ForEachPrimitive([this, &graphBuilder, &proxys](Engine::PrimitiveSceneProxy* proxy) {
                if (proxy->IsA<Engine::StaticMeshProxy>()) {
                    proxys.push_back(static_cast<Engine::StaticMeshProxy*>(proxy));
                }
                });

            StaticMeshDrawBuild(proxys, drawMeshBatches);
            bool isFirst = true;
            for (auto& batch : drawMeshBatches)
            {
                auto vfFlags = batch.VertexFactory->GetVertexFactoryFlags();
                MeshMaterialShaderKey vsKey;
                vsKey.ShaderType =
                    static_cast<MeshMaterialShaderType*>(vsShaderType);
                vsKey.VF = vfType;
                vsKey.VertexFactoryFlags = vfFlags;
                auto vertexShader =
                    GMeshMaterialShaderMap.GetShader(vsKey);
                pipelineDesc.shaderStages.vertexShader = dynamic_cast<RHI::RHIVertexShader*>(vertexShader->GetRHIShader());

                MeshMaterialShaderKey psKey;
                psKey.ShaderType =
                    static_cast<MeshMaterialShaderType*>(psShaderType);
                psKey.VF = vfType;
                psKey.PermutationId = psPermutationId;
                psKey.VertexFactoryFlags = vfFlags;

                auto pixelShader =
                    GMeshMaterialShaderMap.GetShader(psKey);
                pipelineDesc.shaderStages.fragmentShader = dynamic_cast<RHI::RHIFragmentShader*>(pixelShader->GetRHIShader());

                pipelineDesc.vertexDescState = batch.VertexFactory->GetRHIVertexDescState();
                RHI::RHIColorBlendStateDesc blendDesc;
                // 创建颜色混合状态
                RHI::RHIColorBlendAttachmentDesc blendAttachDesc;
                blendAttachDesc.blendEnable = false;
                std::vector<RHI::RHIColorBlendAttachmentDesc> attachments = { blendAttachDesc };
                blendDesc.attachments = attachments;
                auto colorBlendState = RHIPipelineStateCache::GetOrCreateColorBlendState(blendDesc);
                pipelineDesc.colorBlendState = colorBlendState;
                auto pipeline = RHIPipelineStateCache::GetOrCreateGraphicsPipelineState(pipelineDesc);


                // 这里 pipeline 创建建议提取缓存
                // 逻辑与你当前 StaticMeshDrawPass 一样

                BEGIN_SHADER_PARAMETER_STRUCT(PassParameters)
                    SHADER_PARAMETER_STRUCT_REFERENCE(StaticMeshMaterialShaderVSParameters, vertexParameters)
                    SHADER_PARAMETER_STRUCT_REFERENCE(StaticMeshMaterialLightShadowPassPSParameters, pixelParameters)
                END_SHADER_PARAMETER_STRUCT(PassParameters)

                auto* params = graphBuilder.AllocateParameter<PassParameters>();
                params->vertexParameters.vertexFactoryParameters.CameraWorldPosition = shadowView.CameraPos;
                params->vertexParameters.vertexFactoryParameters.ViewProjection = shadowView.ViewProjection;
                params->vertexParameters.vertexFactoryParameters.LocalToWorld = batch.LocalToWorld;
                params->vertexParameters.vertexFactoryParameters.WorldToLocal = batch.WorldToLocal;
                params->pixelParameters.vertexFactoryParameters.CameraWorldPosition = shadowView.CameraPos;
                params->pixelParameters.vertexFactoryParameters.ViewProjection = shadowView.ViewProjection;
                params->pixelParameters.vertexFactoryParameters.LocalToWorld = batch.LocalToWorld;
                params->pixelParameters.vertexFactoryParameters.WorldToLocal = batch.WorldToLocal;
                params->pixelParameters.renderTargetSlots.NumColorRenderTargets = 1;

                params->pixelParameters.renderTargetSlots[0].Texture = shadowView.Allocation.Texture;
                params->pixelParameters.renderTargetSlots[0].ArraySlice = shadowView.Allocation.Layer;
                params->pixelParameters.renderTargetSlots[0].MipIndex = shadowView.Allocation.Mip;
                params->pixelParameters.renderTargetSlots[0].Action = ERenderTargetActions::Load_Store;
                params->pixelParameters.renderTargetSlots.DepthStencil.Texture = shadowMapPassDepthTarget;
                params->pixelParameters.renderTargetSlots.DepthStencil.DepthAction = ERenderTargetActions::Load_Store;
                if (isFirst) {
                    isFirst = false;
                    params->pixelParameters.renderTargetSlots[0].Action = ERenderTargetActions::Clear_Store;
					params->pixelParameters.renderTargetSlots[0].ClearValue = Core::Float4(1, 1, 1, 1);
                    if (shadowView.wantRawDepth) {
                        params->pixelParameters.renderTargetSlots[0].ClearValue = shadowView.RawDepthClearValue;
                    }
                    params->pixelParameters.renderTargetSlots.DepthStencil.DepthAction = ERenderTargetActions::Clear_Store;
                }

                graphBuilder.AddPass<PassParameters>(
                    "ShadowMapPass",
                    PassParameters::GetMetaData(),
                    params,
                    EPassFlag::Graphic,
                    [=](RHI::RHICommandListBase& RHICmdList)
                    {
                        auto& cmd =
                            static_cast<RHI::RHIGraphicCommandList&>(RHICmdList);
                        cmd.SetGraphicPipelineState(pipeline);
                        //绑定vertexfactory
                        batch.VertexFactory->Bind(cmd);
                        // Bind Pipeline
                        //设置vertex参数
                        SetShaderParameters(cmd, vertexShader, &params->vertexParameters);

                        //设置pixel参数
                        SetShaderParameters(cmd, pixelShader, &params->pixelParameters);
                        auto boundRenderTarget = params->pixelParameters.renderTargetSlots.GetBoundRenderTarget();
                        RHIRenderPassInfo passInfo;
                        passInfo.RenderTargets = boundRenderTarget;
                        passInfo.RenderTargets.DepthStencil.ClearBinding.Depth = 1.0f;
                        passInfo.RenderArea.X = slice.X;
                        passInfo.RenderArea.Y = slice.Y;
                        passInfo.RenderArea.Width = slice.Width;
                        passInfo.RenderArea.Height = slice.Height;
                        if (shadowView.TargetHeight != 0 && shadowView.TargetWidth != 0) {
                            //重新设置dimention
                            passInfo.RenderTargets.Dimensions.x = shadowView.TargetWidth;
                            passInfo.RenderTargets.Dimensions.y = shadowView.TargetHeight;
                        }
                        cmd.BeginRenderPass(passInfo);
                        cmd.SetViewport(
                            slice.X,
                            slice.Y,
                            slice.Width,
                            slice.Height,
                            0.0f,
                            1.0f);
                        cmd.SetScissor(slice.X, slice.Y, slice.Width, slice.Height);
                        for (auto& element : batch.Elements)
                        {
                            cmd.DrawIndexed(
                                batch.IndexBuffer->GetRHI(),
                                element.NumIndices,
                                element.NumInstances,
                                element.FirstIndex,
                                element.BaseVertexIndex,
                                element.StartInstance);
                        }
                        cmd.EndRenderPass();
                    });
            }
        }
    }
}
