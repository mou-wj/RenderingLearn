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

    void Run() override {
        using namespace RHI;
        auto* api = GRHIApi;
        constexpr int kFrameCount = 10;
        // 1. 创建随机数据纹理并上传
        RHITextureDesc randomTexDesc = texDesc;
        randomTexDesc.Usage |= ERHITextureCreateFlag::CopyDest | ERHITextureCreateFlag::ShaderResource;
        auto randomTex = api->CreateTexture(randomTexDesc);
        std::vector<uint32_t> randomData(randomTexDesc.Width * randomTexDesc.Height);
        for (auto& v : randomData) v = rand();
        auto* queue = api->GetQueue(EQueueType::Graphics);
        auto* cmdContext = queue->AcquireCommandContext();
        auto* graphicContext = dynamic_cast<RHIGraphicContex*>(cmdContext);
        RHIGraphicCommandList cmdList(graphicContext);
        TransitionResource(api, cmdList, randomTex.get(), ERHIResourceAccess::CopyDest);
        api->UpdateTexture(cmdList, randomTex.get(), randomData.data(), RHITextureRegion::Create2DRegion(randomTexDesc.Width, randomTexDesc.Height));
        TransitionResource(api, cmdList, randomTex.get(), ERHIResourceAccess::SRVGraphics);

        for (int frame = 0; frame < kFrameCount; ++frame) {
            // 2. 从全局RenderTargetPool获取2D RenderTarget
            PoolRenderTargetDesc rtDesc = {};
            rtDesc.Width = texDesc.Width;
            rtDesc.Height = texDesc.Height;
            rtDesc.Format = texDesc.Format;
            rtDesc.Usage = ERHITextureCreateFlag::RenderTarget | ERHITextureCreateFlag::CopySrc | ERHITextureCreateFlag::CopyDest;
            auto renderTarget = GRenderTargetPool->GetFreeRenderTarget(rtDesc);
            // 资源访问转换
            auto lastAccess = renderTarget->GetTracker().GetSubresourceAccess(RHISubresourceRange{});
            if (lastAccess != ERHIResourceAccess::RenderTargetView) {
                TransitionResource(api, cmdList, renderTarget->GetRHI(), ERHIResourceAccess::RenderTargetView);
                renderTarget->GetTracker().UpdateSubresourceAccess(RHISubresourceRange{}, ERHIResourceAccess::RenderTargetView);
            }

            // 3. 注册到builder，获取RDGTexture
            RenderGraphBuilder builder;
            auto rdgSrc = builder.RegisterExternalTexture("SrcTex", randomTex.get());
            auto rdgDst = builder.RegisterExternalTexture("DstTex", renderTarget->GetRHI());

            // 4. 初始化参数
            auto* params = builder.AllocateParameter<CopyTexturePassParameters>();
            params->SrcTexture = rdgSrc.get();
            params->DstTexture = rdgDst.get();

            // 5. 添加拷贝pass
            builder.AddPass<CopyTexturePassParameters>(
                "CopyTexturePass",
                &CopyTexturePassParameters::GetMetaData(),
                params,
                EPassFlag::Transfer,
                [=](RHI::RHICommandListBase& RHICmdList) {
					auto& gTransferCommandList = static_cast<RHI::RHITransferCommandList&>(RHICmdList);
					RHICopyTextureDesc copyDesc;
                    gTransferCommandList.CopyTexture(params->SrcTexture->GetRHITexture(), params->DstTexture->GetRHITexture(), copyDesc);
                }
            );
            builder.Execute();

            // 6. 获取swapchain backbuffer
            // 假设有Swapchain对象
            auto swapchainSlot = Swapchain->AcquireNextSlot();
            auto* backTexture = swapchainSlot.Texture;
            // 访问转换
            // 这里假设有backbuffer的访问记录，实际可用静态变量或全局map
            static std::unordered_map<void*, ERHIResourceAccess> backbufferAccess;
            auto& lastBackAccess = backbufferAccess[backTexture];
            if (lastBackAccess != ERHIResourceAccess::CopyDest) {
                TransitionResource(api, cmdList, backTexture, ERHIResourceAccess::CopyDest);
                lastBackAccess = ERHIResourceAccess::CopyDest;
            }
            auto TransferContext = dynamic_cast<RHITransferContext*>(api->GetQueue(EQueueType::Transfer)->AcquireCommandContext());

			RHI::RHITransferCommandList transferCmdList(TransferContext);
            // 7. Blit到swapchain
            transferCmdList.BlitTexture(renderTarget->GetRHI(), backTexture, RHIBlitTextureDesc{});
            // 8. 转换为Present
            TransitionResource(api, transferCmdList, backTexture, ERHIResourceAccess::Present);
            lastBackAccess = ERHIResourceAccess::Present;
            // 9. Present
            api->GetPresentExecutor()->Present(Swapchain.get(), {});
        }
        std::cout << "RenderGraph CopyTexturePass test finished for " << kFrameCount << " frames." << std::endl;
    }

    void Teardown() override {
        // 资源清理如有需要可补充
    }

private:
    RenderGraphTextureDesc texDesc;
    RHI::RHISwapchainSP Swapchain;
};

REGISTER_RENDER_TEST("RenderGraphBuildTest", RenderGraphBuildTest);
}