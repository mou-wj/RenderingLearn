#include "DistanceFieldProcess.h"

#include "RHIPipelineStateCache.h"
#include "Shader.h"
#include "RHIApi.h"
#include "Common.h"

namespace Renderer {

    IMPLEMENT_GLOBAL_SHADER_TYPE(
        DistanceFieldVoxelizeCS,
        "DistanceFieldVoxelizeCS",
        "/tools/DistanceFieldVoxelizeCS.sf",
        "MainCS",
        RHI::ERHIShaderFrequency::Compute
    );

    IMPLEMENT_GLOBAL_SHADER_TYPE(
        DistanceFieldMergeCS,
        "DistanceFieldMergeCS",
        "/tools/DistanceFieldMergeCS.sf",
        "MainCS",
        RHI::ERHIShaderFrequency::Compute
    );

    IMPLEMENT_GLOBAL_SHADER_TYPE(
        DistanceFieldJumpFlood3DCS,
        "DistanceFieldJumpFlood3DCS",
        "/tools/DistanceFieldJumpFlood3DCS.sf",
        "MainCS",
        RHI::ERHIShaderFrequency::Compute
    );

    static RenderCore::Shader* GetGlobalComputeShader(const char* ShaderName)
    {
        auto shaderTypeFlagIt = RenderCore::ShaderType::GetRegisterMap().find(RenderCore::ShaderType::EShaderTypeFlag::Global);
        if (shaderTypeFlagIt == RenderCore::ShaderType::GetRegisterMap().end())
        {
            return nullptr;
        }

        auto shaderTypeIt = shaderTypeFlagIt->second.find(ShaderName);
        if (shaderTypeIt == shaderTypeFlagIt->second.end() || !shaderTypeIt->second)
        {
            return nullptr;
        }

        return RenderCore::GShaderMap.GetShader(shaderTypeIt->second, 0);
    }

    static RenderCore::RenderTextureSP CreateTemporarySeedTexture3D(const Core::Int3& resolution, const char* debugName)
    {
        RHI::RHITextureDesc desc;
        desc.Width = static_cast<uint32_t>(CORE_MAX(1, resolution.x));
        desc.Height = static_cast<uint32_t>(CORE_MAX(1, resolution.y));
        desc.Depth = static_cast<uint32_t>(CORE_MAX(1, resolution.z));
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Type = RHI::ERHITextureType::Texture3D;
        desc.Format = RHI::ERHIFormat::R32G32B32A32_Float;
        desc.SampleCount = 1;
        desc.InitialQueueType = RHI::EQueueType::Compute;
        desc.Usage =
            RHI::ERHITextureCreateFlag::ShaderResource |
            RHI::ERHITextureCreateFlag::UAV |
            RHI::ERHITextureCreateFlag::TransferDest;
        desc.DebugName = debugName;

        auto texture = std::make_shared<RenderCore::RenderTexture>(desc);
        texture->InitRHIResource();
        return texture;
    }

    bool ExecuteDistanceFieldVoxelizePass(const DistanceFieldVoxelizePassInput& Input)
    {
        if (!Input.VertexBuffer || !Input.IndexBuffer || !Input.OutputSDFTexture)
        {
            return false;
        }

        if (Input.VertexCount == 0 || Input.IndexCount < 3)
        {
            return true;
        }

        const uint32_t primitiveCount = Input.PrimitiveCount > 0 ? Input.PrimitiveCount : (Input.IndexCount / 3);
        if (primitiveCount == 0)
        {
            return true;
        }

        auto* shader = GetGlobalComputeShader("DistanceFieldVoxelizeCS");
        if (!shader)
        {
            return false;
        }

        auto* computeShader = dynamic_cast<RHI::RHIComputeShader*>(shader->GetRHIShader());
        if (!computeShader)
        {
            return false;
        }

        RHI::RHIBufferSRVCreateInfo vertexSRVDesc;
        vertexSRVDesc.Offset = 0;
        vertexSRVDesc.NumElements = Input.VertexCount;
        vertexSRVDesc.Stride = sizeof(SDFVertexPosition);
        vertexSRVDesc.Format = RHI::ERHIFormat::Unknown;

        RHI::RHIBufferSRVCreateInfo indexSRVDesc;
        indexSRVDesc.Offset = 0;
        indexSRVDesc.NumElements = Input.IndexCount;
        indexSRVDesc.Stride = sizeof(uint32_t);
        indexSRVDesc.Format = RHI::ERHIFormat::Unknown;

        const auto& sdfDesc = Input.OutputSDFTexture->GetRHI()->GetDesc();
        RHI::RHITexUAVCreateInfo sdfUAVDesc;
        sdfUAVDesc.Format = sdfDesc.Format;
        sdfUAVDesc.ArraySize = sdfDesc.ArraySize;

        auto* vertexSRV = Input.VertexBuffer->GetViewCache().GetOrCreateSRV(Input.VertexBuffer->GetRHI(), vertexSRVDesc);
        auto* indexSRV = Input.IndexBuffer->GetViewCache().GetOrCreateSRV(Input.IndexBuffer->GetRHI(), indexSRVDesc);
        auto* sdfUAV = Input.OutputSDFTexture->GetViewCache().GetOrCreateUAV(Input.OutputSDFTexture->GetRHI(), sdfUAVDesc);

        if (!vertexSRV || !indexSRV || !sdfUAV)
        {
            return false;
        }

        RenderCore::TransitionBufferImmediate(
            RHI::GRHIApi,
            Input.VertexBuffer,
            RHI::ERHIResourceAccess::SRV,
            RHI::EQueueType::Compute);

        RenderCore::TransitionBufferImmediate(
            RHI::GRHIApi,
            Input.IndexBuffer,
            RHI::ERHIResourceAccess::SRV,
            RHI::EQueueType::Compute);

        RenderCore::TransitionTextureImmediate(
            RHI::GRHIApi,
            Input.OutputSDFTexture,
            RHI::ERHIResourceAccess::UAV,
            RHI::EQueueType::Compute);

        auto* computeQueue = RHI::GRHIApi->GetQueue(RHI::EQueueType::Compute);
        auto* computeContext = computeQueue->AcquireCommandContext();
        auto* computeContextCasted = dynamic_cast<RHI::RHIComputeContex*>(computeContext);
        if (!computeContextCasted)
        {
            return false;
        }

        RHI::RHIComputeCommandList cmd(computeContextCasted);
        cmd.SetImmediate(true);
        cmd.Begin();

        RHI::RHIComputePipelineStateDesc computeDesc;
        computeDesc.computeShader = computeShader;
        auto* pipelineState = RHI::RHIPipelineStateCache::GetOrCreateComputePipelineState(computeDesc);
        cmd.SetComputePipelineState(pipelineState);

        DistanceFieldVoxelizeParameters params;
        params.WorldToVoxel = Input.WorldToVoxel;
        params.GridResolution = Input.GridResolution;
        params.PrimitiveCount = primitiveCount;
        params.VertexBuffer = vertexSRV;
        params.IndexBuffer = indexSRV;
        params.OutputSDFTexture = sdfUAV;

        SetShaderParameters(cmd, shader, &params);

        const uint32_t groupX = (primitiveCount + 63u) / 64u;
        cmd.Dispatch(groupX, 1, 1);

        cmd.End();

        auto fence = computeQueue->ExecuteContext(computeContext);
        Input.VertexBuffer->GetTracker().UpdateLastAccessFence(fence);
        Input.IndexBuffer->GetTracker().UpdateLastAccessFence(fence);
        Input.OutputSDFTexture->GetTracker().UpdateLastAccessFence(fence);

        return true;
    }

    bool ExecuteDistanceFieldMergePass(const DistanceFieldMergePassInput& Input)
    {
        if (!Input.InputSDFTexture || !Input.OutputSDFTexture)
        {
            return false;
        }

        if (Input.OutputResolution.x <= 0 || Input.OutputResolution.y <= 0 || Input.OutputResolution.z <= 0)
        {
            return false;
        }

        auto* shader = GetGlobalComputeShader("DistanceFieldMergeCS");
        if (!shader)
        {
            return false;
        }

        auto* computeShader = dynamic_cast<RHI::RHIComputeShader*>(shader->GetRHIShader());
        if (!computeShader)
        {
            return false;
        }

        const auto& inputDesc = Input.InputSDFTexture->GetRHI()->GetDesc();
        const auto& outputDesc = Input.OutputSDFTexture->GetRHI()->GetDesc();

        RHI::RHITexSRVCreateInfo inputSRVDesc;
        inputSRVDesc.Format = inputDesc.Format;
        inputSRVDesc.ArraySize = inputDesc.ArraySize;

        RHI::RHITexUAVCreateInfo outputUAVDesc;
        outputUAVDesc.Format = outputDesc.Format;
        outputUAVDesc.ArraySize = outputDesc.ArraySize;

        auto* inputSRV = Input.InputSDFTexture->GetViewCache().GetOrCreateSRV(Input.InputSDFTexture->GetRHI(), inputSRVDesc);
        auto* outputUAV = Input.OutputSDFTexture->GetViewCache().GetOrCreateUAV(Input.OutputSDFTexture->GetRHI(), outputUAVDesc);

        if (!inputSRV || !outputUAV)
        {
            return false;
        }

        RenderCore::TransitionTextureImmediate(
            RHI::GRHIApi,
            Input.InputSDFTexture,
            RHI::ERHIResourceAccess::SRV,
            RHI::EQueueType::Compute);

        RenderCore::TransitionTextureImmediate(
            RHI::GRHIApi,
            Input.OutputSDFTexture,
            RHI::ERHIResourceAccess::UAV,
            RHI::EQueueType::Compute);

        auto* computeQueue = RHI::GRHIApi->GetQueue(RHI::EQueueType::Compute);
        auto* computeContext = computeQueue->AcquireCommandContext();
        auto* computeContextCasted = dynamic_cast<RHI::RHIComputeContex*>(computeContext);
        if (!computeContextCasted)
        {
            return false;
        }

        RHI::RHIComputeCommandList cmd(computeContextCasted);
        cmd.SetImmediate(true);
        cmd.Begin();

        RHI::RHIComputePipelineStateDesc computeDesc;
        computeDesc.computeShader = computeShader;
        auto* pipelineState = RHI::RHIPipelineStateCache::GetOrCreateComputePipelineState(computeDesc);
        cmd.SetComputePipelineState(pipelineState);

        DistanceFieldMergeParameters params;
        params.SourceToOutput = Input.SourceToOutput;
        params.OutputToSource = Input.OutputToSource;
        params.OutputResolution = Input.OutputResolution;
        params.InputSDFTexture = inputSRV;
        params.OutputSDFTexture = outputUAV;
        params.InputSDFSampler = RenderCore::GlobalSampler.get();

        SetShaderParameters(cmd, shader, &params);

        const uint32_t groupX = (static_cast<uint32_t>(Input.OutputResolution.x) + 3u) / 4u;
        const uint32_t groupY = (static_cast<uint32_t>(Input.OutputResolution.y) + 3u) / 4u;
        const uint32_t groupZ = (static_cast<uint32_t>(Input.OutputResolution.z) + 3u) / 4u;
        cmd.Dispatch(groupX, groupY, groupZ);

        cmd.End();

        auto fence = computeQueue->ExecuteContext(computeContext);
        Input.InputSDFTexture->GetTracker().UpdateLastAccessFence(fence);
        Input.OutputSDFTexture->GetTracker().UpdateLastAccessFence(fence);

        return true;
    }

    bool ExecuteDistanceFieldJumpFlood3DPass(const DistanceFieldJumpFlood3DPassInput& Input)
    {
        if (!Input.SurfaceMaskTexture || !Input.OutputDistanceTexture)
        {
            return false;
        }

        Core::Int3 resolution = Input.GridResolution;
        if (resolution.x <= 0 || resolution.y <= 0 || resolution.z <= 0)
        {
            const auto& desc = Input.OutputDistanceTexture->GetRHI()->GetDesc();
            resolution = Core::Int3(static_cast<int>(desc.Width), static_cast<int>(desc.Height), static_cast<int>(desc.Depth));
        }
        if (resolution.x <= 0 || resolution.y <= 0 || resolution.z <= 0)
        {
            return false;
        }

        auto seedTextureA = CreateTemporarySeedTexture3D(resolution, "DistanceFieldJFASeedA");
        auto seedTextureB = CreateTemporarySeedTexture3D(resolution, "DistanceFieldJFASeedB");
        if (!seedTextureA || !seedTextureA->GetRHI() || !seedTextureB || !seedTextureB->GetRHI())
        {
            return false;
        }

        auto* shader = GetGlobalComputeShader("DistanceFieldJumpFlood3DCS");
        if (!shader)
        {
            return false;
        }

        auto* computeShader = dynamic_cast<RHI::RHIComputeShader*>(shader->GetRHIShader());
        if (!computeShader)
        {
            return false;
        }

        const auto& maskDesc = Input.SurfaceMaskTexture->GetRHI()->GetDesc();
        const auto& seedADesc = seedTextureA->GetRHI()->GetDesc();
        const auto& seedBDesc = seedTextureB->GetRHI()->GetDesc();
        const auto& distanceDesc = Input.OutputDistanceTexture->GetRHI()->GetDesc();

        RHI::RHITexSRVCreateInfo maskSRVDesc;
        maskSRVDesc.Format = maskDesc.Format;
        maskSRVDesc.ArraySize = maskDesc.ArraySize;

        RHI::RHITexSRVCreateInfo seedASRVDesc;
        seedASRVDesc.Format = seedADesc.Format;
        seedASRVDesc.ArraySize = seedADesc.ArraySize;

        RHI::RHITexSRVCreateInfo seedBSRVDesc;
        seedBSRVDesc.Format = seedBDesc.Format;
        seedBSRVDesc.ArraySize = seedBDesc.ArraySize;

        RHI::RHITexUAVCreateInfo seedAUAVDesc;
        seedAUAVDesc.Format = seedADesc.Format;
        seedAUAVDesc.ArraySize = seedADesc.ArraySize;

        RHI::RHITexUAVCreateInfo seedBUAVDesc;
        seedBUAVDesc.Format = seedBDesc.Format;
        seedBUAVDesc.ArraySize = seedBDesc.ArraySize;

        RHI::RHITexUAVCreateInfo distanceUAVDesc;
        distanceUAVDesc.Format = distanceDesc.Format;
        distanceUAVDesc.ArraySize = distanceDesc.ArraySize;

        auto* maskSRV = Input.SurfaceMaskTexture->GetViewCache().GetOrCreateSRV(Input.SurfaceMaskTexture->GetRHI(), maskSRVDesc);
        auto* seedASRV = seedTextureA->GetViewCache().GetOrCreateSRV(seedTextureA->GetRHI(), seedASRVDesc);
        auto* seedBSRV = seedTextureB->GetViewCache().GetOrCreateSRV(seedTextureB->GetRHI(), seedBSRVDesc);
        auto* seedAUAV = seedTextureA->GetViewCache().GetOrCreateUAV(seedTextureA->GetRHI(), seedAUAVDesc);
        auto* seedBUAV = seedTextureB->GetViewCache().GetOrCreateUAV(seedTextureB->GetRHI(), seedBUAVDesc);
        auto* distanceUAV = Input.OutputDistanceTexture->GetViewCache().GetOrCreateUAV(Input.OutputDistanceTexture->GetRHI(), distanceUAVDesc);

        if (!maskSRV || !seedASRV || !seedBSRV || !seedAUAV || !seedBUAV || !distanceUAV)
        {
            return false;
        }

        RenderCore::TransitionTextureImmediate(
            RHI::GRHIApi,
            Input.SurfaceMaskTexture,
            RHI::ERHIResourceAccess::SRV,
            RHI::EQueueType::Compute);

        RenderCore::TransitionTextureImmediate(
            RHI::GRHIApi,
            seedTextureA.get(),
            RHI::ERHIResourceAccess::UAV,
            RHI::EQueueType::Compute);

        RenderCore::TransitionTextureImmediate(
            RHI::GRHIApi,
            seedTextureB.get(),
            RHI::ERHIResourceAccess::UAV,
            RHI::EQueueType::Compute);

        RenderCore::TransitionTextureImmediate(
            RHI::GRHIApi,
            Input.OutputDistanceTexture,
            RHI::ERHIResourceAccess::UAV,
            RHI::EQueueType::Compute);

        auto* computeQueue = RHI::GRHIApi->GetQueue(RHI::EQueueType::Compute);
        auto* computeContext = computeQueue->AcquireCommandContext();
        auto* computeContextCasted = dynamic_cast<RHI::RHIComputeContex*>(computeContext);
        if (!computeContextCasted)
        {
            return false;
        }

        RHI::RHIComputeCommandList cmd(computeContextCasted);
        cmd.SetImmediate(true);
        cmd.Begin();

        RHI::RHIComputePipelineStateDesc computeDesc;
        computeDesc.computeShader = computeShader;
        auto* pipelineState = RHI::RHIPipelineStateCache::GetOrCreateComputePipelineState(computeDesc);
        cmd.SetComputePipelineState(pipelineState);

        const uint32_t groupX = (static_cast<uint32_t>(resolution.x) + 3u) / 4u;
        const uint32_t groupY = (static_cast<uint32_t>(resolution.y) + 3u) / 4u;
        const uint32_t groupZ = (static_cast<uint32_t>(resolution.z) + 3u) / 4u;

        // Pass 0: initialize seed texture from surface mask.
        DistanceFieldJumpFlood3DParameters params;
        params.GridResolution = resolution;
        params.JumpStep = 0;
        params.bInitializeFromMask = 1;
        params.bFinalizeDistance = 0;
        params.SurfaceMaskTexture = maskSRV;
        params.InputSeedTexture = seedASRV;
        params.OutputSeedTexture = seedAUAV;
        params.OutputDistanceTexture = distanceUAV;
        SetShaderParameters(cmd, shader, &params);
        cmd.Dispatch(groupX, groupY, groupZ);

        uint32_t maxDim = static_cast<uint32_t>(CORE_MAX(resolution.x, CORE_MAX(resolution.y, resolution.z)));
        uint32_t jumpStep = 1;
        while (jumpStep < maxDim)
        {
            jumpStep <<= 1;
        }
        jumpStep >>= 1;

        bool readFromA = true;
        while (jumpStep > 0)
        {
            DistanceFieldJumpFlood3DParameters passParams;
            passParams.GridResolution = resolution;
            passParams.JumpStep = jumpStep;
            passParams.bInitializeFromMask = 0;
            passParams.bFinalizeDistance = 0;
            passParams.SurfaceMaskTexture = maskSRV;
            passParams.InputSeedTexture = readFromA ? seedASRV : seedBSRV;
            passParams.OutputSeedTexture = readFromA ? seedBUAV : seedAUAV;
            passParams.OutputDistanceTexture = distanceUAV;

            SetShaderParameters(cmd, shader, &passParams);
            cmd.Dispatch(groupX, groupY, groupZ);

            readFromA = !readFromA;
            jumpStep >>= 1;
        }

        DistanceFieldJumpFlood3DParameters finalizeParams;
        finalizeParams.GridResolution = resolution;
        finalizeParams.JumpStep = 1;
        finalizeParams.bInitializeFromMask = 0;
        finalizeParams.bFinalizeDistance = 1;
        finalizeParams.SurfaceMaskTexture = maskSRV;
        finalizeParams.InputSeedTexture = readFromA ? seedASRV : seedBSRV;
        finalizeParams.OutputSeedTexture = readFromA ? seedAUAV : seedBUAV;
        finalizeParams.OutputDistanceTexture = distanceUAV;

        SetShaderParameters(cmd, shader, &finalizeParams);
        cmd.Dispatch(groupX, groupY, groupZ);

        cmd.End();

        auto fence = computeQueue->ExecuteContext(computeContext);
        Input.SurfaceMaskTexture->GetTracker().UpdateLastAccessFence(fence);
        seedTextureA->GetTracker().UpdateLastAccessFence(fence);
        seedTextureB->GetTracker().UpdateLastAccessFence(fence);
        Input.OutputDistanceTexture->GetTracker().UpdateLastAccessFence(fence);

        return true;
    }

} // namespace Renderer
