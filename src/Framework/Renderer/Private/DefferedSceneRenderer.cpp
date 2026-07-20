#include "DefferedSceneRenderer.h"
#include "ScreenPass.h"
#include "StaticMeshProcess.h"
#include "StaticMeshProxy.h"
#include "LocalVertexFactory.h"
#include "RHIPipelineStateCache.h"
#include "StaticMeshMaterialShader.h"
#include "GBufferInfo.h"
#include "DrawSkyPass.h"
#include "Log.h"
#include <iostream>
using namespace RenderCore;
using namespace Engine;

namespace Renderer {

    void DefferedSceneRenderer::Build(RenderCore::RenderGraphBuilder& builder)
    {
        auto SceneColorTargetTexture = SceneTextures.SceneColor;
        RenderTargetBindingSlots renderTargetBindingSlots;
        renderTargetBindingSlots.NumColorRenderTargets = 3;
        renderTargetBindingSlots[0].Texture = SceneTextures.GBufferA;
        renderTargetBindingSlots[1].Texture = SceneTextures.GBufferB;
        renderTargetBindingSlots[2].Texture = SceneTextures.GBufferC;
        renderTargetBindingSlots.DepthStencil.Texture = SceneTextures.SceneDepth;
        AddClearRenderTargetsPass(builder, renderTargetBindingSlots);



        auto sceneEnvMap = Scene->GetGPUResourceInfo().LightResourceInfo.IBLSpecularTexture;
        //��renderer���Ƶ�DefferedOutputColor
        //��ȡ����static mesh primitive
        const auto& views = Views->GetViews();
        for (auto view : views) {
            UpdateCascadeShadowInfo(Scene, builder, view);
            MeshBatchList DrawMeshBatches;
            StaticMeshDrawBuild(Scene, view, DrawMeshBatches);
            for (auto MeshBatch : DrawMeshBatches) {
                auto vsfShaderType = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::MeshMaterial]["StaticMeshMaterialShaderVS"];
                auto psfShaderType = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::MeshMaterial]["StaticMeshMaterialGBufferShaderPS"];
                auto vfType = VertexFactoryType::GetRegisterMap()["LocalVertexFactory"];
                auto vfFlags = MeshBatch.VertexFactory->GetVertexFactoryFlags();
                bool supportInstance = MeshBatch.InstanceDataIds.size() > 1;
                if (supportInstance) {
                    LocalVertexFactoryFeatureFlags flags;
                    flags.PackedFlags = vfFlags;
                    flags.SupportsInstanceData = true;
                    vfFlags = flags.PackedFlags;
                }
                auto shdingMode = MeshBatch.MaterialProxy->GetParent()->GetShadingModel();
                shdingMode = EShadingModel::Lit;

                MeshMaterialShaderKey vsKey;
                vsKey.ShaderType = static_cast<MeshMaterialShaderType*>(vsfShaderType);
                vsKey.VF = vfType;
                vsKey.PermutationId = 0;
                vsKey.VertexFactoryFlags = vfFlags;
                vsKey.MaterialParameter.ShadingModel = shdingMode;

                auto vertexShader = GMeshMaterialShaderMap.GetShader(vsKey);

                //����pixel����
                MeshMaterialShaderKey psKey;
                psKey.ShaderType = static_cast<MeshMaterialShaderType*>(psfShaderType);
                psKey.VF = vfType;
                psKey.PermutationId = 0;
                psKey.VertexFactoryFlags = vfFlags;
                psKey.MaterialParameter.ShadingModel = shdingMode;
                auto pixelShader = GMeshMaterialShaderMap.GetShader(psKey);


                // ������դ��״̬
                RHI::RHIRasterizerStateDesc rasterizerDesc;
                rasterizerDesc.polygonMode = RHI::ERHIPolygonMode::Fill;
                rasterizerDesc.cullMode = RHI::ERHICullMode::Back;
                rasterizerDesc.frontFace = MeshBatch.FrontFace;
                rasterizerDesc.lineWidth = 1.0f;
                rasterizerDesc.depthBiasEnable = false;

                auto rasterizerState = RHIPipelineStateCache::GetOrCreateRasterizerState(rasterizerDesc);

                // �������ģ��״̬
                RHI::RHIDepthStencilStateDesc depthStencilDesc;
                depthStencilDesc.depthTestEnable = true;
                depthStencilDesc.depthWriteEnable = true;
                depthStencilDesc.depthCompareOp = RHI::ERHICompareOp::Less;

                auto depthStencilState = RHIPipelineStateCache::GetOrCreateDepthStencilState(depthStencilDesc);
                RHI::RHIGraphicsPipelineStateDesc pipelineDesc;
                // ����ͼ�ι���״̬
                pipelineDesc.shaderStages.vertexShader = dynamic_cast<RHI::RHIVertexShader*>(vertexShader->GetRHIShader());
                pipelineDesc.shaderStages.fragmentShader = dynamic_cast<RHI::RHIFragmentShader*>(pixelShader->GetRHIShader());
                // ����������ø����������...
                pipelineDesc.rasterizerState = rasterizerState;

                pipelineDesc.depthStencilState = depthStencilState;
                pipelineDesc.vertexDescState = MeshBatch.VertexFactory->GetRHIVertexDescState();
                if (supportInstance) {
                    LocalVertexFactory* vf = dynamic_cast<LocalVertexFactory*>(MeshBatch.VertexFactory);
                    pipelineDesc.vertexDescState = vf->GetRHIInstancedVertexDescState();
                }
                //rendertarget info
                auto gbufferInfo = CreateGBufferInfo({});


                pipelineDesc.attachmentDesc.colorAttachmentCount = gbufferInfo.NumTargets;
                for (int i = 0; i < gbufferInfo.NumTargets; i++) {
                    pipelineDesc.attachmentDesc.colorAttachments[i].format = gbufferInfo.Targets[i].TargetFormat;
                    pipelineDesc.attachmentDesc.colorAttachments[i].actions = ERenderTargetActions::Load_Store;
                }
                pipelineDesc.attachmentDesc.depthActions = ERenderTargetActions::Load_Store;
                pipelineDesc.attachmentDesc.enableDepth = true;
                pipelineDesc.attachmentDesc.depthStencilFormat = RHI::ERHIFormat::D32_Float;

                BEGIN_SHADER_PARAMETER_STRUCT(PassParameters)
                    SHADER_PARAMETER_STRUCT_REFERENCE(StaticMeshMaterialShaderVSParameters, vertexParameters)
                    SHADER_PARAMETER_STRUCT_REFERENCE(StaticMeshMaterialGBufferShaderPSParameters, pixelParameters)
                END_SHADER_PARAMETER_STRUCT(PassParameters)
                auto* params = builder.AllocateParameter<PassParameters>();
                if (shdingMode == EShadingModel::Lit) {
                    //�������Լ�������Ϣ
                    BuildShaderParameters(MeshBatch.MaterialProxy, builder, params->pixelParameters.Material);
                }
                auto& batch = MeshBatch;
                
                auto materialOwner = batch.MaterialProxy->GetParent();

                //��ɫ���
                if (materialOwner->GetBlendMode() == EBlendMode::Opaque)
                {
                    RHI::RHIColorBlendStateDesc blendDesc;
                    // ������ɫ���״̬
                    RHI::RHIColorBlendAttachmentDesc blendAttachDesc;
                    blendAttachDesc.blendEnable = false;
                    std::vector<RHI::RHIColorBlendAttachmentDesc> attachments = { blendAttachDesc,blendAttachDesc,blendAttachDesc };
                    blendDesc.attachments = attachments;
                    auto colorBlendState = RHIPipelineStateCache::GetOrCreateColorBlendState(blendDesc);
                    pipelineDesc.colorBlendState = colorBlendState;
                }
                else {
                    LOG_ERROR("Blend mode not supported");
                    return;
                }
                auto pipeline = RHIPipelineStateCache::GetOrCreateGraphicsPipelineState(pipelineDesc);

                // ����


                params->vertexParameters.vertexFactoryParameters.CameraWorldPosition = view.CameraWorldPos;
                params->vertexParameters.vertexFactoryParameters.ViewProjection = view.ViewProjectionMatrix;
                params->vertexParameters.vertexFactoryParameters.LocalToWorld = batch.LocalToWorld;
                params->pixelParameters.vertexFactoryParameters.CameraWorldPosition = view.CameraWorldPos;
                params->pixelParameters.vertexFactoryParameters.ViewProjection = view.ViewProjectionMatrix;
                params->pixelParameters.vertexFactoryParameters.LocalToWorld = batch.LocalToWorld;
                params->pixelParameters.renderTargetSlots.NumColorRenderTargets = gbufferInfo.NumTargets;
                params->pixelParameters.renderTargetSlots[0].Texture = SceneTextures.GBufferA;
                params->pixelParameters.renderTargetSlots[0].Action = ERenderTargetActions::Load_Store;
                params->pixelParameters.renderTargetSlots[1].Texture = SceneTextures.GBufferB;
                params->pixelParameters.renderTargetSlots[1].Action = ERenderTargetActions::Load_Store;
                params->pixelParameters.renderTargetSlots[2].Texture = SceneTextures.GBufferC;
                params->pixelParameters.renderTargetSlots[2].Action = ERenderTargetActions::Load_Store;
                params->pixelParameters.renderTargetSlots.DepthStencil.Texture = SceneTextures.SceneDepth;
                params->pixelParameters.renderTargetSlots.DepthStencil.DepthAction = ERenderTargetActions::Load_Store;
                if (supportInstance) {
                    params->vertexParameters.vertexFactoryParameters.LocalVFInstanceInfo.InstanceData = batch.InstanceDataBufferSRV;
                    params->pixelParameters.vertexFactoryParameters.LocalVFInstanceInfo.InstanceData = batch.InstanceDataBufferSRV;
                }
                // -------------------------------------
                // ���� pass
                // -------------------------------------
                builder.AddPass<PassParameters>(
                    "StaticMeshGBufferPass",
                    PassParameters::GetMetaData(),
                    params,
                    EPassFlag::Graphic,
                    [=](RHI::RHICommandListBase& RHICmdList)
                    {
                        auto& cmd = static_cast<RHI::RHIGraphicCommandList&>(RHICmdList);
                        PassParameters* materialParams = params;

                        cmd.SetGraphicPipelineState(pipeline);
                        //��vertexfactory
                        batch.VertexFactory->Bind(cmd);
                        if (supportInstance) {
                            LocalVertexFactory* vf = dynamic_cast<LocalVertexFactory*>(MeshBatch.VertexFactory);
                            vf->BindInstanceBuffer(cmd, MeshBatch.InstanceDataBufferAccessor->GetInstanceIdBuffer()->GetRHI(), 0);
                        }
                        cmd.SetViewport(view.Viewport.x, view.Viewport.y, view.Viewport.width, view.Viewport.height, 0.0f, 1.0f);
                        cmd.SetScissor(view.Viewport.x, view.Viewport.y, view.Viewport.width, view.Viewport.height);
                        //����vertex����
                        SetShaderParameters(cmd, vertexShader, &params->vertexParameters);

                        //����pixel����
                        SetShaderParameters(cmd, pixelShader, &params->pixelParameters);
                        auto boundRenderTarget = params->pixelParameters.renderTargetSlots.GetBoundRenderTarget();
                        RHIRenderPassInfo passInfo;
                        passInfo.RenderTargets = boundRenderTarget;
                        passInfo.RenderTargets.DepthStencil.ClearBinding.Depth = 1.0f;
                        passInfo.RenderArea.X = view.Viewport.x;
                        passInfo.RenderArea.Y = view.Viewport.y;
                        passInfo.RenderArea.Width = view.Viewport.width;
                        passInfo.RenderArea.Height = view.Viewport.height;
                        cmd.BeginRenderPass(passInfo);
                        //����
                        for (auto element : batch.Elements) {
                            cmd.DrawIndexed(batch.IndexBuffer->GetRHI(), element.NumIndices, batch.InstanceDataIds.size(), element.FirstIndex, element.BaseVertexIndex, batch.StartInstance);
                        }
                        cmd.EndRenderPass();
                    }
                );
                

                

            }
        
            //����һ���ӳٻ���
            auto width = view.Viewport.width;
            auto height = view.Viewport.height;
            auto* paramss = builder.AllocateParameter<StaticMeshMaterialDefferedShadingCSParameters>();
            paramss->InvViewProj = view.InvViewProjectionMatrix;
            paramss->CameraPos = view.CameraWorldPos;
            paramss->ScreenSize.x = width;
            paramss->ScreenSize.y = height;
            paramss->GBuffer.GBufferA = SceneTextures.GBufferA;
            paramss->GBuffer.GBufferB = SceneTextures.GBufferB;
            paramss->GBuffer.GBufferC = SceneTextures.GBufferC;
            paramss->GBuffer.Depth = SceneTextures.SceneDepth;
            RenderGraphTextureUAVDesc uavDesc;
            uavDesc.Format = SceneTextures.SceneColor->GetDesc().Format;
            uavDesc.Texture = SceneTextures.SceneColor;
            
            auto sceneColorUAV = builder.CreateTextureUAV("sceneColorUAV", uavDesc);
            paramss->GBuffer.OutputColor = sceneColorUAV;
            paramss->GBuffer.PointSampler = GlobalSampler.get();
            BuildShaderParameters(Scene, builder, paramss->Scene);
            RHIComputePipelineStateDesc computePipelineDesc;
            auto defferedShaderType = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::Material]["StaticMeshMaterialDefferedShadingCS"];
            MaterialShaderKey key;
            key.ShaderType = static_cast<MaterialShaderType*>(defferedShaderType); ;
            key.PermutationId = 0;
            key.MaterialParameter.ShadingModel = EShadingModel::Lit;
            auto cshader = GMaterialShaderMap.GetShader(key);
            computePipelineDesc.computeShader = dynamic_cast<RHI::RHIComputeShader*>(cshader->GetRHIShader());
            auto defferedDrawPipeline = RHIPipelineStateCache::GetOrCreateComputePipelineState(computePipelineDesc);
            builder.AddPass<StaticMeshMaterialDefferedShadingCSParameters>(
                "StaticMeshDefferedDrawPass",
                StaticMeshMaterialDefferedShadingCSParameters::GetMetaData(),
                paramss,
                EPassFlag::Compute,
                [=](RHI::RHICommandListBase& RHICmdList)
                {
                    auto& cmd = static_cast<RHI::RHIComputeCommandList&>(RHICmdList);
                    StaticMeshMaterialDefferedShadingCSParameters* materialParams = paramss;
            
                    cmd.SetComputePipelineState(defferedDrawPipeline);
            
                    //����pixel����
                    SetShaderParameters(cmd, cshader, paramss);
            
                    cmd.Dispatch(width / 8, height / 8, 1);
                }
            );
            if (sceneEnvMap != nullptr) {
                AddDrawSkyBoxPass(builder, sceneEnvMap, SceneTextures.SceneColor, SceneTextures.SceneDepth, view);
            }
        }
    }
}