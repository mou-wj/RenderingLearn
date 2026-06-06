#include "ForwardSceneRenderer.h"
#include "ScreenPass.h"
#include "StaticMeshProcess.h"
#include "StaticMeshProxy.h"
#include "LocalVertexFactory.h"
#include "RHIPipelineStateCache.h"
#include "StaticMeshMaterialShader.h"
#include <iostream>
using namespace RenderCore;
using namespace Engine;
namespace Renderer {

    void ForwardSceneRenderer::Build(RenderCore::RenderGraphBuilder& builder)
    {
        auto SceneColorTargetTexture = SceneTextures.SceneColor;
        //本renderer绘制到DefferedOutputColor
        //获取所有static mesh primitive
		const auto& views = Views->GetViews();
        for (auto view : views) {
            MeshBatchList DrawMeshBatches;
            std::vector<Engine::StaticMeshProxy*> proxys;
            Scene->ForEachProxyInView(view, [this, &builder, &proxys](Engine::PrimitiveSceneProxy* proxy) {
                if (proxy->IsA<Engine::StaticMeshProxy>()) {
				proxys.push_back(static_cast<Engine::StaticMeshProxy*>(proxy));
			    }
            });
            StaticMeshDrawBuild(proxys, DrawMeshBatches);
            for (auto MeshBatch : DrawMeshBatches) {
                auto vsfShaderType = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::MeshMaterial]["StaticMeshMaterialShaderVS"];
                auto psfShaderType = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::MeshMaterial]["StaticMeshMaterialShaderPS"];
                auto vfType = VertexFactoryType::GetRegisterMap()["LocalVertexFactory"];
                auto vfFlags = MeshBatch.VertexFactory->GetVertexFactoryFlags();
                MeshMaterialShaderKey vsKey;
                vsKey.ShaderType = static_cast<MeshMaterialShaderType*>(vsfShaderType);
                vsKey.VF = vfType;
                vsKey.PermutationId = 0;
                vsKey.VertexFactoryFlags = vfFlags;
                vsKey.MaterialParameter.ShadingModel = EShadingModel::Unlit;

                auto vertexShader = GMeshMaterialShaderMap.GetShader(vsKey);

                //设置pixel参数
                MeshMaterialShaderKey psKey;
                psKey.ShaderType = static_cast<MeshMaterialShaderType*>(psfShaderType);
                psKey.VF = vfType;
                psKey.PermutationId = 0;
                psKey.VertexFactoryFlags = vfFlags;
                psKey.MaterialParameter.ShadingModel = EShadingModel::Unlit;
                auto pixelShader = GMeshMaterialShaderMap.GetShader(psKey);


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
                pipelineDesc.shaderStages.vertexShader = dynamic_cast<RHI::RHIVertexShader*>(vertexShader->GetRHIShader());
                pipelineDesc.shaderStages.fragmentShader = dynamic_cast<RHI::RHIFragmentShader*>(pixelShader->GetRHIShader());
                // 这里可以设置更多管线配置...
                pipelineDesc.rasterizerState = rasterizerState.get();

                pipelineDesc.depthStencilState = depthStencilState.get();
                pipelineDesc.vertexDescState = MeshBatch.VertexFactory->GetRHIVertexDescState().get();

                //rendertarget info
                pipelineDesc.attachmentDesc.colorAttachmentCount = 1;
                pipelineDesc.attachmentDesc.colorAttachments[0].format = RHI::ERHIFormat::R8G8B8A8_UNorm;
                pipelineDesc.attachmentDesc.colorAttachments[0].actions = ERenderTargetActions::Clear_Store;
                pipelineDesc.attachmentDesc.depthActions = ERenderTargetActions::Clear_Store;
                pipelineDesc.attachmentDesc.enableDepth = true;
                pipelineDesc.attachmentDesc.depthStencilFormat = RHI::ERHIFormat::D32_Float;

                for (auto& batch : DrawMeshBatches) {
                    auto materialOwner = batch.MaterialProxy->GetParent();

                    //颜色混合
                    if (materialOwner->GetBlendMode() == EBlendMode::Opaque)
                    {
                        RHI::RHIColorBlendStateDesc blendDesc;
                        // 创建颜色混合状态
                        RHI::RHIColorBlendAttachmentDesc blendAttachDesc;
                        blendAttachDesc.blendEnable = false;
                        std::vector<RHI::RHIColorBlendAttachmentDesc> attachments = { blendAttachDesc };
                        blendDesc.attachments = attachments;
                        auto colorBlendState = RHIPipelineStateCache::GetOrCreateColorBlendState(blendDesc);
                        pipelineDesc.colorBlendState = colorBlendState.get();
                    }
                    else {
                        RHI::RHIColorBlendStateDesc blendDesc;
                        // 创建颜色混合状态
                        RHI::RHIColorBlendAttachmentDesc blendAttachDesc;
                        blendAttachDesc.blendEnable = true;
                        std::vector<RHI::RHIColorBlendAttachmentDesc> attachments = { blendAttachDesc };
                        blendDesc.attachments = attachments;
                        auto colorBlendState = RHIPipelineStateCache::GetOrCreateColorBlendState(blendDesc);
                        pipelineDesc.colorBlendState = colorBlendState.get();
                    }
                    auto pipeline = RHIPipelineStateCache::GetOrCreateGraphicsPipelineState(pipelineDesc);
                    auto state = pipeline;
                    // 参数

                    BEGIN_SHADER_PARAMETER_STRUCT(PassParameters)
                        SHADER_PARAMETER_STRUCT_REFERENCE(StaticMeshMaterialShaderVSParameters, vertexParameters)
                        SHADER_PARAMETER_STRUCT_REFERENCE(StaticMeshMaterialShaderPSParameters, pixelParameters)
                    END_SHADER_PARAMETER_STRUCT(PassParameters)
                    auto* params = builder.AllocateParameter<PassParameters>();
                    params->vertexParameters.vertexFactoryParameters.CameraWorldPosition = view.CameraWorldPos;
                    params->vertexParameters.vertexFactoryParameters.ViewProjection = view.ViewProjectionMatrix;
                    //params->vertexFactoryParameters.LocalToWorld = meshSceneProxy->GetLocalToWorld();
                    params->vertexParameters.vertexFactoryParameters.LocalToWorld = Core::Float4x4::Identity();
                    //params->vertexFactoryParameters.WorldToLocal = meshSceneProxy->GetWorldToLocal();
                    params->vertexParameters.vertexFactoryParameters.WorldToLocal = Core::Float4x4::Identity();

                    params->pixelParameters.renderTargetSlots.NumColorRenderTargets = 1;
                    params->pixelParameters.renderTargetSlots[0].Texture = SceneTextures.SceneColor;
                    params->pixelParameters.renderTargetSlots.DepthStencil.Texture = SceneTextures.SceneDepth;



                    // -------------------------------------
                    // 添加 pass
                    // -------------------------------------
                    builder.AddPass<PassParameters>(
                        "StaticMeshDrawPass",
                        PassParameters::GetMetaData(),
                        params,
                        EPassFlag::Graphic,
                        [=](RHI::RHICommandListBase& RHICmdList)
                        {
                            auto& cmd = static_cast<RHI::RHIGraphicCommandList&>(RHICmdList);
                            PassParameters* materialParams = params;

                            cmd.SetGraphicPipelineState(state.get());
                            //绑定vertexfactory
                            batch.VertexFactory->Bind(cmd);
                            cmd.SetViewport(view.Viewport.x, view.Viewport.y, view.Viewport.width, view.Viewport.height, 0.0f, 1.0f);
                            cmd.SetScissor(view.Viewport.x, view.Viewport.y, view.Viewport.width, view.Viewport.height);
                            //设置vertex参数
                            SetShaderParameters(cmd, vertexShader, &params->vertexParameters);

                            //设置pixel参数
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
                            //绘制
                            for (auto element : batch.Elements) {
                                cmd.DrawIndexed(batch.IndexBuffer->GetRHI(), element.NumIndices, element.NumInstances, element.FirstIndex, element.BaseVertexIndex, element.StartInstance);
                            }
                            cmd.EndRenderPass();
                        }
                    );
                }


            }



        }
    }
}