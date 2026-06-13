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
    void RenderModule::PreComputeIBL(RenderCore::RenderTexture* InHDRTexture, RenderCore::RenderTexture* OutDiffuseIBL, RenderCore::RenderTexture* OutSpecularIBL) {
        if (!InHDRTexture || !OutDiffuseIBL || !OutSpecularIBL) return;
        IBLPrecomputeParameters params;
        uint32_t roughnessCount = OutDiffuseIBL->GetRHI()->GetDesc().MipLevels;
        Core::Int2 outsize;
        outsize.x = OutDiffuseIBL->GetRHI()->GetDesc().Width;
        outsize.y = OutDiffuseIBL->GetRHI()->GetDesc().Height;
        
        params.EnvironmentMapParameter.SampleCount = 4;


        TransitionTextureImmediate(RHI::GRHIApi, InHDRTexture, RHI::RHISubresourceRange{}, RHI::ERHIResourceAccess::SRV, RHI::EQueueType::Compute);
        TransitionTextureImmediate(RHI::GRHIApi, OutDiffuseIBL, RHI::RHISubresourceRange{}, RHI::ERHIResourceAccess::UAV, RHI::EQueueType::Compute);
        TransitionTextureImmediate(RHI::GRHIApi, OutSpecularIBL, RHI::RHISubresourceRange{}, RHI::ERHIResourceAccess::UAV, RHI::EQueueType::Compute);
        params.EnvironmentMapParameter.EnvSampler = RenderCore::GlobalSampler.get();
        params.EnvironmentMapParameter.EnvironmentMap = InHDRTexture->GetRHI();


        // 获取 shader 实例（按变体 id）
        auto& GShaderMap = RenderCore::GShaderMap;
        auto shaderType = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::Global]["IBLPrecomputeCS"];
        
        

        // 创建或获取 compute pipeline state

        
        auto computeContex = GRHIApi->GetQueue(RHI::EQueueType::Compute)->AcquireCastedCommandContext<RHI::RHIComputeContex>();
        RHI::RHIComputeCommandList cmdlist(computeContex);
        cmdlist.SetImmediate(true);
        
        float roughnessStep = 1 / 6.0;
        //计算镜面反射预计算贴图
		for (uint32_t mip = 0; mip < 6; mip++) {

            params.EnvironmentMapParameter.Roughness = mip * roughnessStep;
            RenderCore::Shader* shader = GShaderMap.GetShader(shaderType, 0);
            if (!shader) return;
            RHI::RHIComputePipelineStateDesc computeDesc;
            computeDesc.computeShader = dynamic_cast<RHI::RHIComputeShader*>(shader->GetRHIShader());
            auto pipelineState = RHI::RHIPipelineStateCache::GetOrCreateComputePipelineState(computeDesc);
            cmdlist.SetComputePipelineState(pipelineState);
            RHI::RHITexUAVCreateInfo SpecEnvUAVDesc;
            SpecEnvUAVDesc.Format = OutSpecularIBL->GetRHI()->GetDesc().Format;
            SpecEnvUAVDesc.ArraySize = OutSpecularIBL->GetRHI()->GetDesc().ArraySize;
			SpecEnvUAVDesc.FirstMipSlice = mip;
            auto specuav = OutSpecularIBL->GetViewCache().GetOrCreateUAV(OutSpecularIBL->GetRHI(), SpecEnvUAVDesc);
            
            params.EnvironmentMapParameter.OutputSpecularParam.OutputSpecularTexture = specuav;


            // 设置 shader 参数
            SetShaderParameters(cmdlist, shader, IBLPrecomputeParameters::GetMetaData(), &params);
            // Dispatch
            params.OutputSize = outsize / pow(2,mip);

            int width = params.OutputSize.x;
            int height = params.OutputSize.y;
            uint32_t groupX = (width + 15) / 16;
            uint32_t groupY = (height + 15) / 16;
            cmdlist.Dispatch(groupX, groupY, 6);
            
        }
		//计算漫反射预计算贴图
        RHI::RHITexUAVCreateInfo DifEnvUAVDesc;
        DifEnvUAVDesc.Format = OutDiffuseIBL->GetRHI()->GetDesc().Format;
        DifEnvUAVDesc.ArraySize = OutDiffuseIBL->GetRHI()->GetDesc().ArraySize;
        auto difuav = OutDiffuseIBL->GetViewCache().GetOrCreateUAV(OutDiffuseIBL->GetRHI(), DifEnvUAVDesc);
        params.EnvironmentMapParameter.OutputDiffuseParam.OutputDiffuseTexture = difuav;
        RenderCore::Shader* shader = GShaderMap.GetShader(shaderType, 1);
        if (!shader) return;
        SetShaderParameters(cmdlist, shader, IBLPrecomputeParameters::GetMetaData(), &params);
        // Dispatch
        params.OutputSize = outsize;
        int width = params.OutputSize.x;
        int height = params.OutputSize.y;
        uint32_t groupX = (width + 15) / 16;
		uint32_t groupY = (height + 15) / 16;
        cmdlist.Dispatch(groupX, groupY, 6);
        //生成diffuse的mipmap



        cmdlist.End();
        auto fence = GRHIApi->GetQueue(RHI::EQueueType::Compute)->ExecuteContext(computeContex);
        InHDRTexture->GetTracker().UpdateLastAccessFence(fence);
        OutDiffuseIBL->GetTracker().UpdateLastAccessFence(fence);
        OutSpecularIBL->GetTracker().UpdateLastAccessFence(fence);
    }

	IMPLEMENT_SIMPLE_MODULE(RenderModule, "Renderer");


}