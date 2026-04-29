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
#include <unordered_map>
using namespace RenderCore;
#include "TestBase.h"
namespace RenderCore {
    // 用宏定义shader参数结构体
    BEGIN_SHADER_PARAMETER_STRUCT(CopyTexturePassParameters)
        SHADER_PARAMETER_TEXTURE(RenderGraphTexture, SrcTexture)
        SHADER_PARAMETER_TEXTURE(RenderGraphTexture, DstTexture)
    END_SHADER_PARAMETER_STRUCT(CopyTexturePassParameters)

}


namespace Test {

    namespace {
        std::unordered_map<const RHI::RHIViewableResource*, RHI::ERHIResourceAccess> GTrackedResourceAccess;

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
    }






class RenderGraphBuildTest : public TestBase {
public:
    void Setup() override {
        RHI::GRHIApi = new RHIVulkan::VulkanRHIApi();
        RHI::GRHIApi->Init();
		RenderCore::GShaderCompilationCache = new RenderCore::ShaderCompilationCache();
        auto* api = RHI::GRHIApi;
        if (!api)
        {
            return;
        }
        InitShaderMap();

        InitPipelines();


        // 只保存静态描述
        texDesc.Width = 256;
        texDesc.Height = 256;
        texDesc.Format = ERHIFormat::R8G8B8A8_UNorm;
        texDesc.Usage = ERHITextureCreateFlag::ShaderResource | ERHITextureCreateFlag::CopySrc | ERHITextureCreateFlag::CopyDest;
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
		auto blitTextureShader0 = GShaderMap->GetShader(blitShaderType,0);
        if (!blitTextureShader0) {
            assert(0);
        }
       
        // 创建计算管线状态
        RHI::RHIComputePipelineStateDesc computeDesc;
        computeDesc.computeShader = dynamic_cast<RHIComputeShader*>(blitTextureShader0->GetRHIShader().get());
        ComputePipelineState = RHI::GRHIApi->CreateComputePipelineState(computeDesc);
	}

    void Run() override {
        using namespace RHI;
        using namespace RenderCore;

        auto* api = GRHIApi;
        constexpr int kFrameCount = 10;

        // =========================================
        // 1️⃣ 创建随机源纹理（只做一次）
        // =========================================
        RHITextureDesc randomTexDesc = texDesc;
        randomTexDesc.Usage |= ERHITextureCreateFlag::CopyDest | ERHITextureCreateFlag::ShaderResource;

        RenderTextureSP randomTex = std::make_shared<RenderTexture>(randomTexDesc);

        std::vector<uint32_t> randomData(randomTexDesc.Width * randomTexDesc.Height);
        for (auto& v : randomData) v = rand();

        {
            auto* queue = api->GetQueue(EQueueType::Graphics);
            auto* ctx = queue->AcquireCommandContext();
            RHIGraphicCommandList cmd(dynamic_cast<RHIGraphicContex*>(ctx));

            cmd.Begin();

            TransitionResource(api, cmd, randomTex.get()->GetRHI(), ERHIResourceAccess::CopyDest);

            api->UpdateTexture(cmd, randomTex.get()->GetRHI(), randomData.data(),
                RHITextureRegion::Create2DRegion(randomTexDesc.Width, randomTexDesc.Height));

            TransitionResource(api, cmd, randomTex.get()->GetRHI(), ERHIResourceAccess::SRVGraphics);

            cmd.End();

            queue->ExecuteContext({ ctx }, {});
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
                ERHITextureCreateFlag::CopySrc |
                ERHITextureCreateFlag::CopyDest;

            auto renderTarget = GRenderTargetPool->GetFreeRenderTarget(rtDesc);

            // -------------------------------------
            // RDG 构建
            // -------------------------------------
            RenderGraphBuilder builder;

            // 👉 注册 external
            auto rdgSrc = builder.RegisterExternalTexture("SrcTex", randomTex.get());
            auto rdgDst = builder.RegisterExternalTexture("DstTex", renderTarget.get());

            // 👉 创建 SRV / UAV
            //auto srcTexture = builder.CreateTextureSRV("SrcSRV", { rdgSrc });
            //auto dstUAV = builder.CreateTextureUAV("DstUAV", { rdgDst });

            // 👉 参数
            auto* params = builder.AllocateParameter<BlitTextureParameters>();

            //params->SrcTexture = srcSRV;
            //params->DstTexture = dstUAV;

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
                &BlitTextureParameters::GetMetaData(),
                params,
                EPassFlag::Compute,
                [=](RHI::RHICommandListBase& RHICmdList)
                {
                    auto& cmd = static_cast<RHI::RHIComputeCommandList&>(RHICmdList);

                    cmd.SetComputePipelineState(ComputePipelineState.get());
                    //cmd.SetShaderParameters(params);

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

            auto* transferQueue = api->GetQueue(EQueueType::Transfer);
            auto* ctx = transferQueue->AcquireCommandContext();
            RHITransferCommandList cmd(dynamic_cast<RHITransferContext*>(ctx));

            cmd.Begin();

            TransitionResource(api, cmd, renderTarget->GetRHI(), ERHIResourceAccess::CopySrc);
            TransitionResource(api, cmd, backTexture, ERHIResourceAccess::CopyDest);

            cmd.BlitTexture(renderTarget->GetRHI(), backTexture, {});

            TransitionResource(api, cmd, backTexture, ERHIResourceAccess::Present);

            cmd.End();

            auto fence = transferQueue->ExecuteContext({ ctx }, {});

            // 👉 更新 tracker（关键！）
            renderTarget->GetTracker().UpdateLastAccessFence(fence);

            api->GetPresentExecutor()->Present(Swapchain.get(), {});
        }
        std::cout << "RenderGraph CopyTexturePass test finished for " << kFrameCount << " frames." << std::endl;
    }

    void Teardown() override {
        // 资源清理如有需要可补充
    }

private:
    RenderGraphTextureDesc texDesc;
    RHI::RHIComputePipelineStateSP ComputePipelineState;
    RHI::RHISwapchainSP Swapchain;
};

REGISTER_RENDER_TEST("RenderGraphBuildTest", RenderGraphBuildTest);
}