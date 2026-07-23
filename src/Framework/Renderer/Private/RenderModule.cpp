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
#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "StaticMeshResources.h"
#include "PrimitiveComponent.h"
#include "geometryprocess/DistanceFieldProcess.h"
#include "Shape.h"
#include "GBufferInfo.h"
#include "Common.h"

#include <algorithm>
#include <vector>

using namespace RenderCore;
using namespace Engine;

namespace Renderer {

    namespace
    {
        constexpr int32_t GLocalSDFResolution = 64;

        void FillTexture3D(RenderCore::RenderTexture* texture, float fillValue)
        {
            if (!texture || !texture->GetRHI())
            {
                return;
            }

            const auto& desc = texture->GetRHI()->GetDesc();
            const size_t voxelCount =
                static_cast<size_t>(desc.Width) *
                static_cast<size_t>(desc.Height) *
                static_cast<size_t>(desc.Depth);
            std::vector<float> initData(voxelCount, fillValue);
            texture->UploadData(initData.data(), 0, 0);
        }

        Core::Float4x4 BuildWorldToVoxel(const Core::AABB& bounds, const Core::Int3& resolution)
        {
            Core::Float4x4 m = Core::Float4x4::Identity();
            if (bounds.IsEmpty())
            {
                return m;
            }

            const float sizeX = CORE_MAX(bounds.Max.x - bounds.Min.x, 1e-4f);
            const float sizeY = CORE_MAX(bounds.Max.y - bounds.Min.y, 1e-4f);
            const float sizeZ = CORE_MAX(bounds.Max.z - bounds.Min.z, 1e-4f);

            const float sx = static_cast<float>(CORE_MAX(1, resolution.x - 1)) / sizeX;
            const float sy = static_cast<float>(CORE_MAX(1, resolution.y - 1)) / sizeY;
            const float sz = static_cast<float>(CORE_MAX(1, resolution.z - 1)) / sizeZ;

            m(0, 0) = sx;
            m(1, 1) = sy;
            m(2, 2) = sz;
            m(0, 3) = -bounds.Min.x * sx;
            m(1, 3) = -bounds.Min.y * sy;
            m(2, 3) = -bounds.Min.z * sz;
            return m;
        }

        std::shared_ptr<RenderCore::RenderBuffer> CreateStructuredBufferFromCPU(
            const void* data,
            uint64_t elementCount,
            uint32_t stride,
            const char* debugName)
        {
            RHI::RHIBufferDesc desc;
            desc.Size = CORE_MAX(1ull, elementCount * static_cast<uint64_t>(stride));
            desc.Stride = stride;
            desc.Usage =
                RHI::ERHIBufferUsageFlag::Structured |
                RHI::ERHIBufferUsageFlag::ShaderResource |
                RHI::ERHIBufferUsageFlag::TransferDst;
            desc.InitialQueueType = RHI::EQueueType::Compute;
            desc.DebugName = debugName;

            auto buffer = std::make_shared<RenderCore::RenderBuffer>(desc);
            buffer->InitRHIResource();
            if (data && elementCount > 0)
            {
                buffer->UploadData(data, static_cast<uint32_t>(elementCount * static_cast<uint64_t>(stride)));
            }
            return buffer;
        }
    }

    SceneRendererSP CreateSceneRenderer() {
        return std::make_shared<DefferedSceneRenderer>();
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
        //初始化ibl lut
        InitIBLLut();
        Engine::InitializeShapeStaticMeshes();
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
        GlobalIBLLutTexture.reset();
        Engine::ReleaseShapeStaticMeshes();
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
        depthDesc.Usage = RHI::ERHITextureCreateFlag::DepthStencil | RHI::ERHITextureCreateFlag::ShaderResource;
        auto PoolDepthTarget = GRenderTargetPool.GetFreeRenderTarget(depthDesc);
		PoolDepthTarget->MarkUsed(true);
		auto depthText = builder.RegisterExternalTexture("DepthTarget", PoolDepthTarget.get());
        //构建Gbuffer
        auto gbufferInfo = CreateGBufferInfo({});
        std::vector<RenderCore::RenderGraphTextureRef> gbuffers;
        std::vector<PooledRenderTarget*> GbufferTargets;
        for (int i = 0; i < gbufferInfo.NumTargets; i++) {
            PoolRenderTargetDesc gbufferDesc;
            gbufferDesc.Format = gbufferInfo.Targets[i].TargetFormat;
            gbufferDesc.Width = ColorTex->GetRHI()->GetDesc().Width;
            gbufferDesc.Height = ColorTex->GetRHI()->GetDesc().Height;
            gbufferDesc.Usage = RHI::ERHITextureCreateFlag::RenderTarget | RHI::ERHITextureCreateFlag::ShaderResource;
            auto PoolGbufferTarget = GRenderTargetPool.GetFreeRenderTarget(gbufferDesc);
            PoolGbufferTarget->MarkUsed(true);
            GbufferTargets.push_back(PoolGbufferTarget.get());
            auto gbufferText = builder.RegisterExternalTexture(std::string("GBufferTarget") + std::to_string(i), PoolGbufferTarget.get());
            gbuffers.push_back(gbufferText);
        }
		LocalVertexFactoryInstanceManager::Get().BeginUpdateGPUResources();
        auto sceneRenderer = CreateSceneRenderer();
        sceneRenderer->Scene =  dynamic_cast<Scene*>(Views->Scene);
        sceneRenderer->Views = Views;
        sceneRenderer->SceneTextures.SceneColor = sceneColorTexture;
        sceneRenderer->SceneTextures.SceneDepth = depthText;
        sceneRenderer->SceneTextures.GBufferA = gbuffers[0];
        sceneRenderer->SceneTextures.GBufferB = gbuffers[1];
        sceneRenderer->SceneTextures.GBufferC = gbuffers[2];
		sceneRenderer->BuildSceneLightShadowMap(sceneRenderer->Scene,builder);
        sceneRenderer->UploadShadowMapInfo(sceneRenderer->Scene,builder);
        sceneRenderer->Build(builder);
        LocalVertexFactoryInstanceManager::Get().UpdateGPUResources();
        builder.Execute();
        PoolDepthTarget->MarkUsed(false);
        for (auto gTarget : GbufferTargets) {
            gTarget->MarkUsed(false);
        }
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


        TransitionTextureImmediate(RHI::GRHIApi, InHDRTexture,  RHI::ERHIResourceAccess::SRV, RHI::EQueueType::Compute);
        TransitionTextureImmediate(RHI::GRHIApi, OutDiffuseIBL, RHI::ERHIResourceAccess::UAV, RHI::EQueueType::Compute);
        TransitionTextureImmediate(RHI::GRHIApi, OutSpecularIBL, RHI::ERHIResourceAccess::UAV, RHI::EQueueType::Compute);
        params.EnvironmentMapParameter.EnvSampler = RenderCore::GlobalSampler.get();
        params.EnvironmentMapParameter.EnvironmentMap = InHDRTexture->GetRHI();


        // 获取 shader 实例（按变体 id）
        auto& GShaderMap = RenderCore::GShaderMap;
        auto shaderType = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::Global]["IBLPrecomputeCS"];
        
        

        // 创建或获取 compute pipeline state

        
        auto computeContex = GRHIApi->GetQueue(RHI::EQueueType::Compute)->AcquireCastedCommandContext<RHI::RHIComputeContex>();
        RHI::RHIComputeCommandList cmdlist(computeContex);
        cmdlist.SetImmediate(true);
        cmdlist.Begin();
        float roughnessStep = 1 / 5.0;
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

            params.OutputSize = outsize / pow(2, mip);
            // 设置 shader 参数
            SetShaderParameters(cmdlist, shader, IBLPrecomputeParameters::GetMetaData(), &params);
            // Dispatch
            

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
        RHI::RHIComputePipelineStateDesc computeDesc;
        computeDesc.computeShader = dynamic_cast<RHI::RHIComputeShader*>(shader->GetRHIShader());
        auto pipelineState = RHI::RHIPipelineStateCache::GetOrCreateComputePipelineState(computeDesc);
        cmdlist.SetComputePipelineState(pipelineState);
        params.OutputSize = outsize;
        SetShaderParameters(cmdlist, shader, IBLPrecomputeParameters::GetMetaData(), &params);
        // Dispatch
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

    void RenderModule::InitIBLLut() {
        RHI::RHITextureDesc desc;
        desc.Width = 512;
        desc.Height = 512;
        desc.Format = RHI::ERHIFormat::R32G32_Float;
        desc.Usage = RHI::ERHITextureCreateFlag::ShaderResource | RHI::ERHITextureCreateFlag::UAV | RHI::ERHITextureCreateFlag::TransferSrc;
        GlobalIBLLutTexture = std::make_shared<RenderTexture>(desc);
        GlobalIBLLutTexture->InitRHIResource();

        IBLPrecomputeParameters params;
        Core::Int2 outsize;
        outsize.x = GlobalIBLLutTexture->GetRHI()->GetDesc().Width;
        outsize.y = GlobalIBLLutTexture->GetRHI()->GetDesc().Height;

        params.EnvironmentMapParameter.SampleCount = 4;


        TransitionTextureImmediate(RHI::GRHIApi, GlobalIBLLutTexture.get(),  RHI::ERHIResourceAccess::UAV, RHI::EQueueType::Compute);
        RHI::RHITexUAVCreateInfo lutUAVDesc;
        lutUAVDesc.Format = GlobalIBLLutTexture->GetRHI()->GetDesc().Format;
        lutUAVDesc.ArraySize = GlobalIBLLutTexture->GetRHI()->GetDesc().ArraySize;
        auto uav = GlobalIBLLutTexture->GetViewCache().GetOrCreateUAV(GlobalIBLLutTexture->GetRHI(), lutUAVDesc);
        params.BRDFParameter.OutputBRDFLUT = uav;
        // 获取 shader 实例（按变体 id）
        auto& GShaderMap = RenderCore::GShaderMap;
        auto shaderType = ShaderType::GetRegisterMap()[ShaderType::EShaderTypeFlag::Global]["IBLPrecomputeCS"];

        // 创建或获取 compute pipeline state
        auto computeContex = GRHIApi->GetQueue(RHI::EQueueType::Compute)->AcquireCastedCommandContext<RHI::RHIComputeContex>();
        RHI::RHIComputeCommandList cmdlist(computeContex);
        cmdlist.SetImmediate(true);

        //计算lut预计算贴图
        RenderCore::Shader* shader = GShaderMap.GetShader(shaderType, 2);
        if (!shader) return;
        RHI::RHIComputePipelineStateDesc computeDesc;
        computeDesc.computeShader = dynamic_cast<RHI::RHIComputeShader*>(shader->GetRHIShader());
        auto pipelineState = RHI::RHIPipelineStateCache::GetOrCreateComputePipelineState(computeDesc);
        cmdlist.SetComputePipelineState(pipelineState);
        params.OutputSize = outsize;
        SetShaderParameters(cmdlist, shader, IBLPrecomputeParameters::GetMetaData(), &params);
        // Dispatch
        
        int width = params.OutputSize.x;
        int height = params.OutputSize.y;
        uint32_t groupX = (width + 15) / 16;
        uint32_t groupY = (height + 15) / 16;
        cmdlist.Dispatch(groupX, groupY, 1);
        //生成diffuse的mipmap
        cmdlist.End();
        auto fence = GRHIApi->GetQueue(RHI::EQueueType::Compute)->ExecuteContext(computeContex);
        GlobalIBLLutTexture->GetTracker().UpdateLastAccessFence(fence);
        //写出测试
        //SaveTexture(GlobalIBLLutTexture.get(), "IBLLut.png",0,0);
    }

    bool RenderModule::PreComputePrimitiveSDF(Engine::PrimitiveComponent* PrimitiveComponent)
    {
        if (!PrimitiveComponent)
        {
            return false;
        }

        auto* staticMeshComponent = dynamic_cast<Engine::StaticMeshComponent*>(PrimitiveComponent);
        if (!staticMeshComponent)
        {
            return false;
        }

        Engine::StaticMesh* staticMesh = staticMeshComponent->GetStaticMesh();
        if (!staticMesh)
        {
            return false;
        }

        std::shared_ptr<Engine::StaticMeshRenderData> renderData = staticMesh->GetRenderData();
        if (!renderData || renderData->GetLODCount() == 0)
        {
            return false;
        }

        const Engine::LODResource& lod0 = renderData->GetLODResource(0);
        const std::vector<float>& positions = lod0.VertexBuffers.PositionBuffer.Vertices;
        const std::vector<uint32_t>& indices = lod0.IndexBuffer.Indices;
        const uint32_t numPositionComponents = lod0.VertexBuffers.PositionBuffer.NumComponents;

        if (positions.empty() || indices.size() < 3 || numPositionComponents != 3)
        {
            return false;
        }

        if (!renderData->EnsureSDFPrecomputeTextures(static_cast<uint32_t>(GLocalSDFResolution)))
        {
            return false;
        }

        RenderCore::RenderTexture* surfaceMaskTexture = renderData->GetSDFSurfaceMaskTexture3D();
        RenderCore::RenderTexture* outputSDFTexture = renderData->GetSDFTexture3D();
        if (!surfaceMaskTexture || !outputSDFTexture)
        {
            return false;
        }

        FillTexture3D(surfaceMaskTexture, 1.0f);

        auto vertexBuffer = CreateStructuredBufferFromCPU(
            positions.data(),
            positions.size() / 3,
            sizeof(float) * 3,
            "StaticMeshSDFVertexBuffer");
        auto indexBuffer = CreateStructuredBufferFromCPU(
            indices.data(),
            indices.size(),
            sizeof(uint32_t),
            "StaticMeshSDFIndexBuffer");

        DistanceFieldVoxelizePassInput voxelizeInput;
        voxelizeInput.VertexBuffer = vertexBuffer.get();
        voxelizeInput.VertexCount = static_cast<uint32_t>(positions.size() / 3);
        voxelizeInput.IndexBuffer = indexBuffer.get();
        voxelizeInput.IndexCount = static_cast<uint32_t>(indices.size());
        voxelizeInput.PrimitiveCount = static_cast<uint32_t>(indices.size() / 3);
        voxelizeInput.GridResolution = Core::Int3(GLocalSDFResolution, GLocalSDFResolution, GLocalSDFResolution);
        voxelizeInput.WorldToVoxel = BuildWorldToVoxel(renderData->Bounds.Box, voxelizeInput.GridResolution);
        voxelizeInput.OutputSDFTexture = surfaceMaskTexture;

        if (!ExecuteDistanceFieldVoxelizePass(voxelizeInput))
        {
            return false;
        }

        DistanceFieldJumpFlood3DPassInput jumpFloodInput;
        jumpFloodInput.SurfaceMaskTexture = surfaceMaskTexture;
        jumpFloodInput.OutputDistanceTexture = outputSDFTexture;
        jumpFloodInput.GridResolution = Core::Int3(GLocalSDFResolution, GLocalSDFResolution, GLocalSDFResolution);

        return ExecuteDistanceFieldJumpFlood3DPass(jumpFloodInput);
    }

	IMPLEMENT_SIMPLE_MODULE(RenderModule, "Renderer");


}