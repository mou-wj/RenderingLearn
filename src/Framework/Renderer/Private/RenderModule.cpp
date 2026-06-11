#include "RenderModule.h"
#include "DefferedSceneRenderer.h"
#include "ForwardSceneRenderer.h"
#include "Scene.h"
#include "RenderThread.h"
#include "ScreenPass.h"
#include "SceneInterface.h"
#include "Viewport.h"
#include "RenderResource.h"
#include "MeshMaterialShader.h"
#include "MateiralShader.h"
#include "GlobalShader.h"
#include "ShaderCompiler.h"
#include "RHIPipelineStateCache.h"
using namespace RenderCore;
using namespace Engine;

namespace Renderer {

    SceneRendererSP CreateSceneRenderer() {
        return std::make_shared<ForwardSceneRenderer>();
    }

    RenderModule::RenderModule() = default;
    RenderModule::~RenderModule() = default;

    void RenderModule::StartupModule()
    {
        //������Ⱦ�߳�
        RenderCore::StartRenderThread();
		InitGlobalRenderResource();
        GTransientResourceAllocator.InitRHI();
        GMeshMaterialShaderMap.Initialize();
        GMaterialShaderMap.Initialize();
        GShaderMap.Initialize();
        bLoaded = true;
    }

    void RenderModule::ShutdownModule()
    {
        bLoaded = false;
        GRenderTargetPool.Clear();
        GTransientResourceAllocator.ReleaseRHI();
		GMeshMaterialShaderMap.Clear();
        GMaterialShaderMap.Clear();
		GShaderMap.Clear();
        ReleaseGlobalRenderResource();
        // �ر���Ⱦ�߳�
        RenderCore::StopRenderThread();
		
    }

    bool RenderModule::IsLoaded() const
    {
        return bLoaded;
    }


    void RenderModule::BeginRender(
        Engine::SceneViewFamily* Views)
    {
        RenderCore::RenderGraphBuilder builder;
        auto ColorTex = Views->RenderTarget->GetRenderTarget();
        auto sceneColorTexture = builder.RegisterExternalTexture("RenderTarget", ColorTex);
        PoolRenderTargetDesc depthDesc;
        depthDesc.Format = RHI::ERHIFormat::D32_Float;
        depthDesc.Width = ColorTex->GetRHI()->GetDesc().Width;
		depthDesc.Height = ColorTex->GetRHI()->GetDesc().Height;
        depthDesc.Usage = RHI::ERHITextureCreateFlag::DepthStencil;
        auto PoolDepthTarget = GRenderTargetPool.GetFreeRenderTarget(depthDesc);
		PoolDepthTarget->MarkUsed(true);
		auto depthText = builder.RegisterExternalTexture("DepthTarget", PoolDepthTarget.get());

        auto sceneRenderer = CreateSceneRenderer();
        sceneRenderer->Scene =  dynamic_cast<Scene*>(Views->Scene);
        sceneRenderer->Views = Views;
        sceneRenderer->SceneTextures.SceneColor = sceneColorTexture;
        sceneRenderer->SceneTextures.SceneDepth = depthText;
        sceneRenderer->Build(builder);
        
        builder.Execute();
        PoolDepthTarget->MarkUsed(false);
    }
    Engine::SceneInterface* RenderModule::AllocateScene() {
        return new Scene();
    }
    void RenderModule::PreComputeIBL(RenderCore::RenderTexture* InHDRTexture, RenderCore::RenderTexture* OutDiffuseIBL, RenderCore::RenderTexture* OutSpecularIBL){
        if (!InHDRTexture || !OutDiffuseIBL || !OutSpecularIBL) return;
        IBLPrecomputeParameters params;
        uint32_t roughnessCount = OutDiffuseIBL->GetRHI()->GetDesc().MipLevels;
        Core::Int2 outsize;
        outsize.x = OutDiffuseIBL->GetRHI()->GetDesc().Width;
        outsize.y = OutDiffuseIBL->GetRHI()->GetDesc().Height;
		params.OutputSize = outsize;
        params.EnvironmentMapParameter.SampleCount = 4;
		params.EnvironmentMapParameter.RoughnessCount = roughnessCount;
        RHI::RHITexUAVCreateInfo DifEnvUAVDesc;
        DifEnvUAVDesc.Format = OutDiffuseIBL->GetRHI()->GetDesc().Format;
        DifEnvUAVDesc.MipCount = OutDiffuseIBL->GetRHI()->GetDesc().MipLevels;
        DifEnvUAVDesc.ArraySize = OutDiffuseIBL->GetRHI()->GetDesc().ArraySize;
        auto difuav = OutDiffuseIBL->GetViewCache().GetOrCreateUAV(OutDiffuseIBL->GetRHI(), DifEnvUAVDesc);
        RHI::RHITexUAVCreateInfo SpecEnvUAVDesc;
        SpecEnvUAVDesc.Format = OutSpecularIBL->GetRHI()->GetDesc().Format;
        SpecEnvUAVDesc.MipCount = OutSpecularIBL->GetRHI()->GetDesc().MipLevels;
        SpecEnvUAVDesc.ArraySize = OutSpecularIBL->GetRHI()->GetDesc().ArraySize;
        auto specuav = OutSpecularIBL->GetViewCache().GetOrCreateUAV(OutSpecularIBL->GetRHI(), SpecEnvUAVDesc);
        TransitionTextureImmediate(RHI::GRHIApi, InHDRTexture, RHI::RHISubresourceRange{}, RHI::ERHIResourceAccess::SRV, RHI::EQueueType::Compute);
        TransitionTextureImmediate(RHI::GRHIApi,OutDiffuseIBL, RHI::RHISubresourceRange{},RHI::ERHIResourceAccess::UAV, RHI::EQueueType::Compute);
        TransitionTextureImmediate(RHI::GRHIApi, OutSpecularIBL, RHI::RHISubresourceRange{}, RHI::ERHIResourceAccess::UAV, RHI::EQueueType::Compute);
        params.EnvironmentMapParameter.EnvSampler = RenderCore::GlobalSampler.get();
        params.EnvironmentMapParameter.EnvironmentMap = InHDRTexture->GetRHI();
        params.EnvironmentMapParameter.OutputDiffuseTexture = difuav;
        params.EnvironmentMapParameter.OutputSpecularTexture = specuav;

        // 获取 shader 实例（按变体 id）
        auto& GShaderMap = RenderCore::GShaderMap;
        auto shaderType = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::Global]["IBLPrecomputeCS"];
        RenderCore::Shader* shader = GShaderMap.GetShader(shaderType, 0);
        if (!shader) return;
        
        // 创建或获取 compute pipeline state
        RHI::RHIComputePipelineStateDesc computeDesc;
        computeDesc.computeShader = dynamic_cast<RHI::RHIComputeShader*>(shader->GetRHIShader());
        auto pipelineState = RHI::RHIPipelineStateCache::GetOrCreateComputePipelineState(computeDesc);
        auto computeContex = GRHIApi->GetQueue(RHI::EQueueType::Compute)->AcquireCastedCommandContext<RHI::RHIComputeContex>();
        RHI::RHIComputeCommandList cmdlist(computeContex);
        cmdlist.SetImmediate(true);
        cmdlist.SetComputePipelineState(pipelineState);
        
        // 设置 shader 参数
        SetShaderParameters(cmdlist, shader, IBLPrecomputeParameters::GetMetaData(), &params);
        // Dispatch
        int width = params.OutputSize.x;
        int height = params.OutputSize.y;
        uint32_t groupX = (width + 15) / 16;
        uint32_t groupY = (height + 15) / 16;
        cmdlist.Dispatch(groupX, groupY, 1);
        cmdlist.End();
		auto fence = GRHIApi->GetQueue(RHI::EQueueType::Compute)->ExecuteContext(computeContex);
        InHDRTexture->GetTracker().UpdateLastAccessFence(fence);
        OutDiffuseIBL->GetTracker().UpdateLastAccessFence(fence);
        OutSpecularIBL->GetTracker().UpdateLastAccessFence(fence);
    }

	IMPLEMENT_SIMPLE_MODULE(RenderModule, "Renderer");

    RenderModule* GetRenderModuleInstance() {
        static RenderModule* instance = nullptr;
        if (instance == nullptr) {
            instance = dynamic_cast<RenderModule*>(Core::ModuleManager::Get().GetModule("Renderer").get());
        }
        return instance;
    }
}