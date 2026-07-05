#include "SceneRenderer.h"
#include "StaticMeshMaterialShader.h"
#include "LightSceneProxy.h"
#include "Transform.hpp"
#include "StaticMeshProcess.h"
#include "StaticMeshProxy.h"
#include "MaterialCore.h"
#include "RHIPipelineStateCache.h"
using namespace RenderCore;
using namespace Engine;
namespace Renderer {


    void SceneRenderer::BuildSceneLightShadowMap(RenderCore::RenderGraphBuilder& builder)
    {
        std::vector<ShadowRenderView> shadowRenderViews;

        Scene->ForEachLight(
            [this, &shadowRenderViews](Engine::LightSceneProxy* light)
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
                    view.Allocation = allocation.Slices[0];
                    shadowInfo.ShadowMatrices.resize(1);
                    auto proj =
                        Core::PerspectiveRH(
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
                    view.TargetHeight = allocator.GetDesc().SpotShadowAtlas.Width;
                    view.TargetWidth = allocator.GetDesc().SpotShadowAtlas.Height;
                    shadowRenderViews.push_back(view);
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

                    static Core::Float3 ups[6] =
                    {
                        {0,-1,0},
                        {0,-1,0},
                        {0,0,1},
                        {0,0,-1},
                        {0,-1,0},
                        {0,-1,0}
                    };

                    auto proj =
                        Core::PerspectiveRH(
                            90.0f * 3.1415926f / 180.0f,
                            1.0f,
                            0.1f,
                            pointLight->GetAttenuationRadius());
                    shadowInfo.ShadowMatrices.resize(6);
                    for (uint32_t i = 0; i < 6; i++)
                    {
                        ShadowRenderView view;
                        view.Allocation =
                            allocation.Slices[i];

                        auto viewMat =
                            Core::LookAtRH(
                                pointLight->GetPosition(),
                                pointLight->GetPosition() +
                                directions[i],
                                ups[i]);

                        view.ViewProjection =
                            proj * viewMat;
                        shadowInfo.ShadowMatrices[i] = view.ViewProjection;
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
            [this, &shadowRenderViews,&sceneView](Engine::LightSceneProxy* light)
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
                    std::array<float, 5> splitDepths;
                    splitDepths[0] = sceneView.NearClip;
                    splitDepths[4] = sceneView.FarClip;

                    float lambda = 0.7f;
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
                        view.Allocation =
                            allocation.Slices[i];
                        //构建每个cascade深度
                        float p = float(i) / 4.0f;

                        float logSplit =
                            sceneView.NearClip *
                            std::pow(sceneView.FarClip / sceneView.NearClip, p);

                        float linearSplit =
                            sceneView.NearClip +
                            (sceneView.FarClip - sceneView.NearClip) * p;

                        splitDepths[i] =
                            Core::Lerp(linearSplit, logSplit, lambda);

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

                        auto proj =
                            Core::OrthoRH(
                                bounds.Min.x,
                                bounds.Max.x,
                                bounds.Min.y,
                                bounds.Max.y,
                                bounds.Min.z,
                                bounds.Max.z);


                        view.ViewProjection =
                            proj * lightView;
                        shadowInfo.ShadowMatrices[i] = view.ViewProjection;
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
        rasterizerDesc.cullMode = RHI::ERHICullMode::None;
        rasterizerDesc.frontFace = RHI::ERHIFrontFace::Clockwise;
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
        pipelineDesc.attachmentDesc.colorAttachments[0].actions = ERenderTargetActions::Clear_Store;
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

            MeshBatchList drawMeshBatches;
            std::vector<Engine::StaticMeshProxy*> proxys;
            Scene->ForEachPrimitive([this, &graphBuilder, &proxys](Engine::PrimitiveSceneProxy* proxy) {
                if (proxy->IsA<Engine::StaticMeshProxy>()) {
                    proxys.push_back(static_cast<Engine::StaticMeshProxy*>(proxy));
                }
                });

            StaticMeshDrawBuild(proxys, drawMeshBatches);

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
                    SHADER_PARAMETER_STRUCT_REFERENCE(StaticMeshMaterialShaderVSParameters,vertexParameters)
                    SHADER_PARAMETER_STRUCT_REFERENCE(StaticMeshMaterialLightShadowPassPSParameters, pixelParameters)
                END_SHADER_PARAMETER_STRUCT(PassParameters)

                auto* params =
                    graphBuilder.AllocateParameter<PassParameters>();
                //params->vertexParameters.vertexFactoryParameters.CameraWorldPosition = view.CameraWorldPos;
                params->vertexParameters.vertexFactoryParameters.ViewProjection = shadowView.ViewProjection;
                //params->vertexFactoryParameters.LocalToWorld = meshSceneProxy->GetLocalToWorld();
                params->vertexParameters.vertexFactoryParameters.LocalToWorld = Core::Float4x4::Identity();
                //params->vertexFactoryParameters.WorldToLocal = meshSceneProxy->GetWorldToLocal();
                params->vertexParameters.vertexFactoryParameters.WorldToLocal = Core::Float4x4::Identity();
                params->pixelParameters.vertexFactoryParameters.ViewProjection = shadowView.ViewProjection;
                //params->pixelParameters.LocalToWorld = meshSceneProxy->GetLocalToWorld();
                params->pixelParameters.vertexFactoryParameters.LocalToWorld = Core::Float4x4::Identity();
                //params->pixelParameters.WorldToLocal = meshSceneProxy->GetWorldToLocal();
                params->pixelParameters.vertexFactoryParameters.WorldToLocal = Core::Float4x4::Identity();
                params->pixelParameters.renderTargetSlots.NumColorRenderTargets = 1;
                auto colorTarget = graphBuilder.RegisterExternalTexture(std::string("ShadowPassTarget") + std::to_string((uint64_t)shadowView.Allocation.Texture), shadowView.Allocation.Texture);
                params->pixelParameters.renderTargetSlots[0].Texture = colorTarget;
                params->pixelParameters.renderTargetSlots[0].ArraySlice = shadowView.Allocation.Layer;
                params->pixelParameters.renderTargetSlots[0].MipIndex = shadowView.Allocation.Mip;
                params->pixelParameters.renderTargetSlots.DepthStencil.Texture = shadowMapPassDepthTarget;


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
