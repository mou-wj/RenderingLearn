// StaticMeshRenderTest.cpp
// 测试StaticMesh的构建和执行：实现一个简单的拷贝纹理pass
#include "RenderGraphBuilder.h"
#include "RenderGraphPass.h"
#include "RenderGraphResource.h"
#include "ShaderParameter.h"
#include "GlobalShader.h"
#include "RenderResource.h"
#include "ShaderCompiler.h"
#include "RHIApi.h"
#include "VulkanRHIApi.h"
#include "ToolShaders.h"
#include "Window.h"
#include "AssetManager.h"
#include <unordered_map>
#include "TestBase.h"
#include "PathInfo.h"
#include "StaticMeshComponent.h"
#include "RenderModule.h"
#include "SceneInterface.h"
#include "Viewport.h"
#include "PrimitiveComponent.h"
#include "PrimitiveSceneProxy.h"
#include "LocalVertexFactory.h"
#include "RHIPipelineStateCache.h"
#include "ShaderParameter.h"
using namespace RenderCore;
using namespace Engine;
using namespace Renderer;
using namespace RHI;
namespace Test {

    namespace {
        std::unordered_map<const RHI::RHIViewableResource*, RHI::ERHIResourceAccess> GTrackedResourceAccess;

        void TransitionResource(
            RHI::RHIApi* api,
            RHI::RHICommandListBase& cmdList,
            RHI::RHIViewableResource* resource,
            RHI::ERHIResourceAccess currentAccess,
            RHI::ERHIResourceAccess targetAccess)
        {
            if (!api || !resource)
            {
                return;
            }

            if (currentAccess == targetAccess)
            {
                return;
            }

            std::vector<RHI::RHITransitionInfo> infos;
            if (auto* texture = dynamic_cast<RHI::RHITexture*>(resource))
            {
                infos.emplace_back(texture, currentAccess, targetAccess);
            }
            else if (auto* buffer = dynamic_cast<RHI::RHIBuffer*>(resource))
            {
                infos.emplace_back(buffer, currentAccess, targetAccess);
            }
            else
            {
                return;
            }

            char* transitionMem = new char[RHI::G_RHITransition_TotalSize];
            auto* transition = new(transitionMem) RHI::RHITransition();
            api->RHICreateTransition(transition, RHI::RHITransitionCreateInfo(RHI::ERHITransitionCreateFlags::None, std::move(infos)));

            cmdList.BeginTransitions({ transition });
            cmdList.EndTransitions({ transition });

            GTrackedResourceAccess[resource] = targetAccess;
            api->RHIReleaseTransition(transition);
            delete[] transitionMem;
        }


        void TransitionResource(
            RHI::RHIApi* api,
            RHI::RHICommandListBase& cmdList,
            RHI::RHIViewableResource* resource,
            RHI::ERHIResourceAccess targetAccess)
        {
            if (!api || !resource)
            {
                return;
            }

            const auto it = GTrackedResourceAccess.find(resource);
            const RHI::ERHIResourceAccess currentAccess =
                it != GTrackedResourceAccess.end() ? it->second : RHI::ERHIResourceAccess::Unknown;
            TransitionResource(api, cmdList, resource, currentAccess, targetAccess);
        }
    }






    class StaticMeshRenderTest : public TestBase {
    public:
        int FrameWidth = 0;
        int FrameHeight = 0;
        int WindiwWidth = 512;
        int WindowHeight = 512;
        RHI::RHISamplerSP samplerSP;
        void Setup() override {
            RHI::GRHIApi = new RHIVulkan::VulkanRHIApi();
            RHI::GRHIApi->Init();
            RenderCore::GShaderCompilationCache = new RenderCore::ShaderCompilationCache();
            auto* api = RHI::GRHIApi;
            if (!api)
            {
                return;
            }
            GetRenderModuleInstance();
            InitSwapchain();
            InitGlobalResources();
            InitStaticMesh();
            InitPipelineState();


            // 只保存静态描述
            texDesc.Width = FrameWidth;
            texDesc.Height = FrameHeight;
            texDesc.Format = ERHIFormat::R8G8B8A8_UNorm;
            texDesc.Usage = ERHITextureCreateFlag::ShaderResource | ERHITextureCreateFlag::TransferSrc | ERHITextureCreateFlag::TransferDest;
        }

        void InitSwapchain()
        {
            // 
            Window = Slate::WindowFactory::CreateWindowSP(WindiwWidth, WindowHeight, "RHIRenderTriangleTest");
            Window->Show();
            // 创建交换链
            void* windowHandle = Window->GetNativeHandle(); // 获取窗口句柄的函数
            uint32_t width = WindiwWidth;
            uint32_t height = WindowHeight;
            ERHIFormat format = ERHIFormat::B8G8R8A8_UNorm;
            Swapchain = RHI::GRHIApi->CreateSwapchain(windowHandle, width, height, format);
            auto frameSize = Window->GetFramebufferSize();
            FrameWidth = frameSize.x;
            FrameHeight = frameSize.y;
        }

        void InitGlobalResources()
        {
            GRenderTargetPool = new RenderTargetPool();
            GTransientResourceAllocator = new TransientResourceAllocator();
            GMeshMaterialShaderMap.Initialize();
        }

        void InitStaticMesh() {
			AssetManager::Get().LoadSync<MaterialAsset>(Core::GetProjectDir() + "/resources/material/DefaultWhite/material.json");

            staticMeshAsset = AssetManager::Get().LoadSync<StaticMeshAsset>(Core::GetProjectDir() + "/resources/glb/sphere.glb");
            staticMeshComponent = new StaticMeshComponent();
            staticMeshComponent->SetStaticMesh(staticMeshAsset->GetMesh());
            scene = GetRenderModuleInstance()->AllocateScene();
            scene->AddPrimitive(staticMeshComponent);
        }

        void InitPipelineState() {
            auto vsfShaderType = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::MeshMaterial]["StaticMeshMaterialShaderVS"];
            auto psfShaderType = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::MeshMaterial]["StaticMeshMaterialShaderPS"];
            auto vfType = VertexFactoryType::GetRegisterMap()["LocalVertexFactory"];
            auto vfFlags = staticMeshComponent->GetStaticMesh()->GetRenderData()->GetLODResource(0).VertexFactory->GetVertexFactoryFlags();
            MeshMaterialShaderKey vsKey;
            vsKey.ShaderType = static_cast<MeshMaterialShaderType*>(vsfShaderType);
            vsKey.VF = vfType;
            vsKey.PermutationId = 0;
            vsKey.VertexFactoryFlags = vfFlags;
            vsKey.MaterialParameter.ShadingModel = EShadingModel::Unlit;

            vertexShader = GMeshMaterialShaderMap.GetShader(vsKey);

            //设置pixel参数
            MeshMaterialShaderKey psKey;
            psKey.ShaderType = static_cast<MeshMaterialShaderType*>(psfShaderType);
            psKey.VF = vfType;
            psKey.PermutationId = 0;
            psKey.VertexFactoryFlags = vfFlags;
            psKey.MaterialParameter.ShadingModel = EShadingModel::Unlit;
            pixelShader = GMeshMaterialShaderMap.GetShader(psKey);


            // 创建光栅化状态
            RHI::RHIRasterizerStateDesc rasterizerDesc;
            rasterizerDesc.polygonMode = RHI::ERHIPolygonMode::Fill;
            rasterizerDesc.cullMode = RHI::ERHICullMode::Back;
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

            // 创建图形管线状态
            pipelineDesc.shaderStages.vertexShader = dynamic_cast<RHI::RHIVertexShader*>(vertexShader->GetRHIShader());
            pipelineDesc.shaderStages.fragmentShader = dynamic_cast<RHI::RHIFragmentShader*>(pixelShader->GetRHIShader());
            // 这里可以设置更多管线配置...
            pipelineDesc.rasterizerState = rasterizerState.get();

            pipelineDesc.depthStencilState = depthStencilState.get();
            pipelineDesc.vertexDescState = staticMeshAsset->GetMesh()->GetRenderData()->LODResources[0].VertexFactory->GetRHIVertexDescState().get();

            //rendertarget info
            pipelineDesc.attachmentDesc.colorAttachmentCount = 1;
            pipelineDesc.attachmentDesc.colorAttachments[0].format = RHI::ERHIFormat::B8G8R8A8_UNorm;
            pipelineDesc.attachmentDesc.colorAttachments[0].actions = ERenderTargetActions::Clear_Store;
            pipelineDesc.attachmentDesc.depthActions = ERenderTargetActions::Clear_Store;
            pipelineDesc.attachmentDesc.enableDepth = true;
            pipelineDesc.attachmentDesc.depthStencilFormat = RHI::ERHIFormat::D32_Float;



        }

        void Run() override {
            using namespace RHI;
            using namespace RenderCore;

            auto* api = GRHIApi;
            constexpr int kFrameCount = 10;

            RHI::RHISamplerDesc samplerDesc{};
            samplerSP = api->CreateSampler(samplerDesc);


            // =========================================
            // 2️⃣ 每帧执行 RDG
            // =========================================
            for (int frame = 0; frame < kFrameCount; ++frame)
            {

                // -------------------------------------
                // 从 pool 取 RT
                // -------------------------------------
                PoolRenderTargetDesc rtDesc{};
                rtDesc.Width = texDesc.Width;
                rtDesc.Height = texDesc.Height;
                rtDesc.Format = texDesc.Format;
                rtDesc.Usage = ERHITextureCreateFlag::RenderTarget |
                    ERHITextureCreateFlag::TransferSrc |
                    ERHITextureCreateFlag::TransferDest;

                auto renderTarget = GRenderTargetPool->GetFreeRenderTarget(rtDesc);

                PoolRenderTargetDesc depthTargetDesc{};
                depthTargetDesc.Width = texDesc.Width;
                depthTargetDesc.Height = texDesc.Height;
                depthTargetDesc.Format = RHI::ERHIFormat::D32_Float;
                depthTargetDesc.Usage = ERHITextureCreateFlag::DepthStencil;

                auto depthRenderTarget = GRenderTargetPool->GetFreeRenderTarget(depthTargetDesc);

                //绘制场景
                SceneView sceneView;
                SceneViewFamily sceneViewFamily;
                sceneViewFamily.Scene = scene;
                sceneViewFamily.AddView(sceneView);
                RenderTarget target;
                target.RenderTarget = renderTarget->GetRHI();
                sceneViewFamily.RenderTarget = &target;
                auto meshSceneProxy = staticMeshComponent->CreateSceneProxy();
                MeshBatchList list;
                meshSceneProxy->GetMeshBatches(sceneView,list);

                // -------------------------------------
                // RDG 构建
                // -------------------------------------
                RenderGraphBuilder builder;

                for (auto& batch : list) {
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
                    
                    BEGIN_SHADER_PARAMETER_STRUCT(PixelMaterialParameters)
                        SHADER_PARAMETER_RENDER_TARGET_BINDING_SLOTS(renderTargetSlots)
                    END_SHADER_PARAMETER_STRUCT(PixelMaterialParameters)

                    BEGIN_SHADER_PARAMETER_STRUCT(MaterialParameters)
                        SHADER_PARAMETER_STRUCT(LocalVertexFactoryParameters, vertexFactoryParameters)
                        SHADER_PARAMETER_STRUCT(PixelMaterialParameters, pixelParameters)
                    END_SHADER_PARAMETER_STRUCT(MaterialParameters)
                    auto* params = builder.AllocateParameter<MaterialParameters>();
                    params->vertexFactoryParameters.CameraWorldPosition = sceneView.CameraWorldPos;
                    params->vertexFactoryParameters.ViewProjection = sceneView.ViewProjectionMatrix;
                    params->vertexFactoryParameters.ViewProjection = Core::Float4x4::Identity();
					params->vertexFactoryParameters.LocalToWorld = meshSceneProxy->GetLocalToWorld();
                    params->vertexFactoryParameters.LocalToWorld = Core::Float4x4::Identity();
					params->vertexFactoryParameters.WorldToLocal = meshSceneProxy->GetWorldToLocal();
                    params->vertexFactoryParameters.WorldToLocal = Core::Float4x4::Identity();

                    auto rdgDst = builder.RegisterExternalTexture("DstTex", renderTarget.get());
                    auto depthRdgDst = builder.RegisterExternalTexture("DstDepthTex", depthRenderTarget.get());
                    params->pixelParameters.renderTargetSlots.NumColorRenderTargets = 1;
                    params->pixelParameters.renderTargetSlots[0].Texture = rdgDst;
                    params->pixelParameters.renderTargetSlots.DepthStencil.Texture = depthRdgDst;



                    // -------------------------------------
                    // 添加 pass
                    // -------------------------------------
                    builder.AddPass<MaterialParameters>(
                        "BlitPass",
                        MaterialParameters::GetMetaData(),
                        params,
                        EPassFlag::Graphic,
                        [=](RHI::RHICommandListBase& RHICmdList)
                        {
                            auto& cmd = static_cast<RHI::RHIGraphicCommandList&>(RHICmdList);
							MaterialParameters* materialParams = params;



                            cmd.SetGraphicPipelineState(state.get());
                            //绑定vertexfactory
                            batch.VertexFactory->Bind(cmd);
                            
                            //设置vertex参数
                            SetShaderParameters(cmd, vertexShader, params);

                            //设置pixel参数
                            SetShaderParameters(cmd, pixelShader, params);

                            //绘制
                            for (auto element : batch.Elements) {
                                cmd.DrawIndexed(batch.IndexBuffer->GetRHI(), element.NumPrimitives, element.NumInstances, element.FirstIndex, element.BaseVertexIndex, element.StartInstance);
                            }
                        }
                    );
                }
                // -------------------------------------
                // 执行 RDG
                // -------------------------------------
                builder.Execute();

                // =========================================
                // 3️⃣ Blit 到 swapchain
                // =========================================
                auto slot = Swapchain->AcquireNextSlot();
                auto* backTexture = slot.Texture;

                auto* computeQueue = api->GetQueue(EQueueType::Compute);
                auto* ctx = computeQueue->AcquireCommandContext();
                RHIComputeCommandList cmd(dynamic_cast<RHIComputeContex*>(ctx));
                cmd.SetImmediate(true);
                cmd.Begin();

                TransitionResource(api, cmd, renderTarget->GetRHI(), renderTarget->GetTracker().GetSubresourceAccess(RHI::RHISubresourceRange{}), ERHIResourceAccess::TransferSrc);


                TransitionResource(api, cmd, backTexture, ERHIResourceAccess::TransferDest);
                RHI::RHIBlitTextureDesc blit{};
                blit.SrcRegion.Width = texDesc.Width;
                blit.SrcRegion.Height = texDesc.Height;
                blit.DstRegion.Width = FrameWidth;
                blit.DstRegion.Height = FrameHeight;
                cmd.BlitTexture(renderTarget->GetRHI(), backTexture, blit);

                TransitionResource(api, cmd, backTexture, ERHIResourceAccess::Present);

                cmd.End();
                std::vector<RHI::RHIWaitInfo> waitInfos;

                RHI::RHIWaitInfo renderTargetFinish;
                renderTargetFinish.QueueType = renderTarget->GetTracker().GetLastAccessFence().QueueType;
                renderTargetFinish.Value = renderTarget->GetTracker().GetLastAccessFence().Value;
                waitInfos.push_back(renderTargetFinish);
                if (slot.ReadySync)
                {
                    waitInfos.push_back({ slot.ReadySync, EQueueType::Graphics, 0,RHI::ERHIPipelineStage::ColorAttachmentOutput });
                }
                auto fence = computeQueue->ExecuteContext({ ctx }, waitInfos);

                // 更新 tracker（关键！）
                renderTarget->GetTracker().UpdateLastAccessFence(fence);
                renderTarget->GetTracker().UpdateSubresourceAccess(RHI::RHISubresourceRange{}, ERHIResourceAccess::TransferSrc);
                RHI::RHIWaitInfo presentWait;
                presentWait.QueueType = fence.QueueType;
                presentWait.Value = fence.Value;
                api->GetPresentExecutor()->Present(Swapchain.get(), { presentWait });
            }

            std::cout << "RenderGraph CopyTexturePass test finished for " << kFrameCount << " frames." << std::endl;
        }

        void Teardown() override {
            // 资源清理如有需要可补充
            GRenderTargetPool->Clear();
            delete GRenderTargetPool;
            Swapchain.reset();
            Window.reset();
            samplerSP.reset();
            RHI::GRHIApi->Shutdown();
            delete RHI::GRHIApi;
        }

    private:
        RenderGraphTextureDesc texDesc;
        RHI::RHISwapchainSP Swapchain;
        Slate::WindowSP Window;
        RenderCore::Shader* blitTextureShader0;
        std::shared_ptr<StaticMeshAsset> staticMeshAsset;
        StaticMeshComponent* staticMeshComponent;
        SceneInterface* scene;
        RHI::RHIGraphicsPipelineStateDesc pipelineDesc;
        Shader* vertexShader;
        Shader* pixelShader;

    };

    REGISTER_RENDER_TEST("StaticMeshRenderTest", StaticMeshRenderTest);
}