// RenderGraphBuildTest.cpp
// 测试RenderGraph的构建和执行：实现一个简单的拷贝纹理pass
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
#include <unordered_map>
using namespace RenderCore;
#include "TestBase.h"

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






class RenderGraphBuildTest : public TestBase {
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
        InitShaderMap();

        InitPipelines();
        InitGlobalResources();

        // 只保存静态描述
        texDesc.Width = FrameWidth;
        texDesc.Height = FrameHeight;
        texDesc.Format = ERHIFormat::R8G8B8A8_UNorm;
        texDesc.Usage = ERHITextureCreateFlag::ShaderResource | ERHITextureCreateFlag::TransferSrc | ERHITextureCreateFlag::TransferDest;
    }

    

    void InitShaderMap()
    {
        GShaderMap = new GlobalShaderMap();
        GShaderMap->Initialize();

    }

	void InitPipelines()
	{
		//获取第0好blittextureshader变体
        auto blitShaderType = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::Global]["BlitTextureCS"];
        blitTextureShader0 = GShaderMap->GetShader(blitShaderType,0);
        if (!blitTextureShader0) {
            assert(0);
        }
       
        // 创建计算管线状态
        RHI::RHIComputePipelineStateDesc computeDesc;
        computeDesc.computeShader = dynamic_cast<RHIComputeShader*>(blitTextureShader0->GetRHIShader().get());
        ComputePipelineState = RHI::GRHIApi->CreateComputePipelineState(computeDesc);
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

    void Run() override {
        using namespace RHI;
        using namespace RenderCore;

        auto* api = GRHIApi;
        constexpr int kFrameCount = 10;

        RHI::RHISamplerDesc samplerDesc{};
        samplerSP = api->CreateSampler(samplerDesc);

        // =========================================
        // 1️⃣ 创建随机源纹理（只做一次）
        // =========================================
        RHITextureDesc randomTexDesc = texDesc;
        texDesc.InitialQueueType = EQueueType::Compute;
        randomTexDesc.Usage |= ERHITextureCreateFlag::TransferDest | ERHITextureCreateFlag::ShaderResource;
        randomTex = std::make_shared<RenderTexture>(randomTexDesc);
        randomTex->InitRHIResource();
        std::vector<uint32_t> randomData(randomTexDesc.Width * randomTexDesc.Height);
        for (int i = 0; i < randomData.size(); i++) {
            randomData[i] = rand();
        }

        {
            auto* queue = api->GetQueue(EQueueType::Compute);
            auto* ctx = queue->AcquireCommandContext();
            RHIComputeCommandList cmd(dynamic_cast<RHIComputeContex*>(ctx));
            cmd.SetImmediate(true);
            cmd.Begin();

            TransitionResource(api, cmd, randomTex.get()->GetRHI(), ERHIResourceAccess::TransferDest);

            api->UpdateTexture(cmd, randomTex.get()->GetRHI(), randomData.data(),
                RHIUpdateTextureRegion::Create2DRegion(randomTexDesc.Width, randomTexDesc.Height));

            TransitionResource(api, cmd, randomTex.get()->GetRHI(), ERHIResourceAccess::SRVCompute);

            cmd.End();

            queue->ExecuteContext({ ctx }, {});
            randomTex->GetTracker().UpdateSubresourceAccess(RHI::RHISubresourceRange{}, ERHIResourceAccess::SRVCompute);
        }

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

            // -------------------------------------
            // RDG 构建
            // -------------------------------------
            RenderGraphBuilder builder;

            // 👉 注册 external
            auto rdgSrc = builder.RegisterExternalTexture("SrcTex", randomTex.get());
            auto rdgDst = builder.RegisterExternalTexture("DstTex", renderTarget.get());

            // 👉 创建 SRV / UAV
            RenderGraphTextureUAVDesc dstUAVDesc{};
            dstUAVDesc.Texture = rdgDst;
            dstUAVDesc.Format = rtDesc.Format;
            auto dstUAV = builder.CreateTextureUAV("DstUAV", dstUAVDesc);

            // 👉 参数
            auto* params = builder.AllocateParameter<BlitTextureParameters>();

            params->SrcTexture = rdgSrc;
            params->DstTexture = dstUAV;
            params->SrcSampler = samplerSP.get();
            params->SrcSize = { (float)texDesc.Width, (float)texDesc.Height };
            params->SrcInvSize = { 1.0f / texDesc.Width, 1.0f / texDesc.Height };
            params->DstSize = params->SrcSize;
            params->DstInvSize = params->SrcInvSize;
            params->SrcMipLevel = 0;

            // -------------------------------------
            // 添加 pass
            // -------------------------------------
            builder.AddPass<BlitTextureParameters>(
                "BlitPass",
                BlitTextureParameters::GetMetaData(),
                params,
                EPassFlag::Compute,
                [=](RHI::RHICommandListBase& RHICmdList)
                {
                    auto& cmd = static_cast<RHI::RHIComputeCommandList&>(RHICmdList);

                    cmd.SetComputePipelineState(ComputePipelineState.get());
                    SetShaderParameters(cmd, blitTextureShader0, params);
                    cmd.Dispatch(
                        texDesc.Width / 8,
                        texDesc.Height / 8,
                        1);
                }
            );

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

            // 👉 更新 tracker（关键！）
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
        GShaderMap->Clear();
        delete GRenderTargetPool;
        delete GShaderMap;
		ComputePipelineState.reset();
        Swapchain.reset();
        Window.reset();
		samplerSP.reset();
        randomTex.reset();
        RHI::GRHIApi->Shutdown();
        delete RHI::GRHIApi;
    }

private:
    RenderGraphTextureDesc texDesc;
    RHI::RHIComputePipelineStateSP ComputePipelineState;
    RHI::RHISwapchainSP Swapchain;
    Slate::WindowSP Window;
    RenderCore::Shader* blitTextureShader0;
    RenderTextureSP randomTex;
};

REGISTER_RENDER_TEST("RenderGraphBuildTest", RenderGraphBuildTest);
}