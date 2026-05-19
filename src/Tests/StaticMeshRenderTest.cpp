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
#include "ShaderParameter.h"
using namespace RenderCore;
using namespace Engine;
using namespace Renderer;
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
            InitSwapchain();

            InitPipelines();
            InitGlobalResources();

            // 只保存静态描述
            texDesc.Width = FrameWidth;
            texDesc.Height = FrameHeight;
            texDesc.Format = ERHIFormat::R8G8B8A8_UNorm;
            texDesc.Usage = ERHITextureCreateFlag::ShaderResource | ERHITextureCreateFlag::TransferSrc | ERHITextureCreateFlag::TransferDest;
        }



        void InitPipelines()
        {
            //获取第0好blittextureshader变体
            
            
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
        }

        void InitStaticMesh() {
            staticMeshAsset = AssetManager::Get().LoadSync<StaticMeshAsset>(Core::GetProjectDir() + "/resources/obj/sphere.obj");
            staticMeshComponent = new StaticMeshComponent();
            staticMeshComponent->SetStaticMesh(staticMeshAsset->GetMesh());
            scene = GetRenderModuleInstance()->AllocateScene();
            scene->AddPrimitive(staticMeshComponent);
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
                rtDesc.Usage = ERHITextureCreateFlag::UAV |
                    ERHITextureCreateFlag::TransferSrc |
                    ERHITextureCreateFlag::TransferDest;

                auto renderTarget = GRenderTargetPool->GetFreeRenderTarget(rtDesc);

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
					auto materialOwner = batch.MaterialProxy->GetOwnerMaterial();
                    auto state = MaterialInterface::GetGraphicPipelineState(materialOwner);


                    // 参数
                    auto* vfparams = builder.AllocateParameter<LocalVertexFactoryParameters>();
                    
                    BEGIN_SHADER_PARAMETER_STRUCT(PixelMaterialParameters)
                        SHADER_PARAMETER_RENDER_TARGET_BINDING_SLOTS(renderTargetSlots)
                    END_SHADER_PARAMETER_STRUCT(PixelMaterialParameters)

                    BEGIN_SHADER_PARAMETER_STRUCT(MaterialParameters)
                        SHADER_PARAMETER_STRUCT(LocalVertexFactoryParameters, vertexFactoryParameters)
                        SHADER_PARAMETER_STRUCT(PixelMaterialParameters, pixelParameters)
                    END_SHADER_PARAMETER_STRUCT(MaterialParameters)
                    auto* params = builder.AllocateParameter<MaterialParameters>();
                    params->vertexFactoryParameters = vfparams;

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
                            auto vertexShader = MaterialInterface::GetShader(materialOwner,ERHIShaderFrequency::Vertex);
                            SetShaderParameters(cmd, vertexShader, materialParams->vertexFactoryParameters);

                            //设置pixel参数
                            auto pixelShader = MaterialInterface::GetShader(materialOwner, ERHIShaderFrequency::Fragment);
                            SetShaderParameters(cmd, pixelShader, materialParams->pixelParameters);

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

    };

    REGISTER_RENDER_TEST("StaticMeshRenderTest", StaticMeshRenderTest);
}