#include "VulkanCommandContex.h"
#include "VulkanCommandBuffer.h"
#include "VulkanQueue.h"
#include "VulkanShader.h"
#include "VulkanDescriptorSets.h"
#include "VulkanRHIUtils.h"
#include "Log.h"

namespace RHIVulkan {

VulkanCommandContext::VulkanCommandContext(VulkanDevice* device, VulkanQueue* queue)
    : device(device)
    , queue(queue)
    , commandBufferManager(new VulkanCommandBufferManager(device, this))
{
}

VulkanCommandContext::~VulkanCommandContext()
{
    delete commandBufferManager;
}

void VulkanCommandContext::Begin()
{
    commandBufferManager->GetActiveCommandBuffer();
    InnerBegin();
}

void VulkanCommandContext::End()
{
    device->GetDescriptorSetManager()->GarbageCollect();
	device->GetStagingManager()->GarbageCollect();
    commandBufferManager->GarbageCollect();


    auto commandBuffer = commandBufferManager->EndActiveCommandBuffer();
    RecordedCommandBuffer = commandBuffer;
}

void VulkanCommandContext::BeginTransitions(std::vector<const RHITransition*> Transitions)
{
    for (const RHITransition* transition : Transitions)
    {
        const VulkanPipelineBarrier* pipelineBarrier = transition->GetPrivateData<VulkanPipelineBarrier>();
        if (pipelineBarrier)
        {
        }
    }
}

void VulkanCommandContext::EndTransitions(std::vector<const RHITransition*> Transitions)
{
    for (const RHITransition* transition : Transitions)
    {
        VulkanPipelineBarrier* pipelineBarrier = transition->GetPrivateData<VulkanPipelineBarrier>();
        if (pipelineBarrier)
        {
            auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();
            pipelineBarrier->Execute(commandBuffer);
        }
    }
}

void VulkanCommandContext::CopyTexture(RHITexture* src, RHITexture* dst, const RHICopyTextureDesc& copyDesc)
{

    if (!(copyDesc.SrcRegion.Width == copyDesc.DstRegion.Width &&
        copyDesc.SrcRegion.Height == copyDesc.DstRegion.Height &&
        copyDesc.SrcRegion.Depth == copyDesc.DstRegion.Depth)) {
#ifdef DEBUG_INFO
        LOG_ERROR("%s", "src extent no match dst");
#endif // DEBUG_INFO
        return;
     }



    VulkanTexture* vkSrc = static_cast<VulkanTexture*>(src);
    VulkanTexture* vkDst = static_cast<VulkanTexture*>(dst);

    VkImage srcImage = vkSrc->GetImage();
    VkImage dstImage = vkDst->GetImage();

    auto srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    auto dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    VkImageCopy region{};

    // =========================
    // Src
    // =========================
    region.srcSubresource.aspectMask = vkSrc->GetAspectFlags();
    region.srcSubresource.mipLevel = copyDesc.SrcMipIndex;
    region.srcSubresource.baseArrayLayer = copyDesc.SrcArraySlice;
    region.srcSubresource.layerCount = 1;

    region.srcOffset = {
        copyDesc.SrcRegion.OffsetX,
        copyDesc.SrcRegion.OffsetY,
        copyDesc.SrcRegion.OffsetZ
    };

    // =========================
    // Dst������������ǡ����� copy�������� offset ��ͬ��
    // =========================
    region.dstSubresource.aspectMask = vkDst->GetAspectFlags();
    region.dstSubresource.mipLevel = copyDesc.DstMipIndex;
    region.dstSubresource.baseArrayLayer = copyDesc.DstArraySlice;
    region.dstSubresource.layerCount = 1;

    region.dstOffset = {
        copyDesc.DstRegion.OffsetX,
        copyDesc.DstRegion.OffsetY,
        copyDesc.DstRegion.OffsetZ
    };


    region.extent = {
        copyDesc.SrcRegion.Width,
        copyDesc.SrcRegion.Height,
        copyDesc.SrcRegion.Depth == 0 ? 1u : copyDesc.SrcRegion.Depth
    };

    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();

    VKFunc::CmdCopyImage(
        commandBuffer->GetHandle(),
        srcImage,
        srcImageLayout,
        dstImage,
        dstImageLayout,
        1,
        &region);

}
void VulkanCommandContext::BlitTexture(RHITexture* src, RHITexture* dst, const RHIBlitTextureDesc& blitDesc)
{
    if (!(blitDesc.SrcRegion.Width > 0 &&
        blitDesc.SrcRegion.Height > 0 &&
        blitDesc.DstRegion.Width > 0 &&
        blitDesc.DstRegion.Height > 0)) {
#ifdef DEBUG_INFO
        LOG_ERROR("%s","BlitTexture: Region size must be > 0");
#endif // DEBUG_INFO


    }
    VulkanTexture* vkSrc = static_cast<VulkanTexture*>(src);
    VulkanTexture* vkDst = static_cast<VulkanTexture*>(dst);

    VkImage srcImage = vkSrc->GetImage();
    VkImage dstImage = vkDst->GetImage();

    // =========================
    // Layout ��ȡ��ע������ȡ����
    // =========================
    auto srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    auto dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    // =========================
    // ����У�飨���鱣����
    // =========================


    // =========================
    // VkImageBlit ����
    // =========================
    VkImageBlit region{};

    // ---- Src ----
    region.srcSubresource.aspectMask = vkSrc->GetAspectFlags();
    region.srcSubresource.mipLevel = blitDesc.SrcMipIndex;
    region.srcSubresource.baseArrayLayer = blitDesc.SrcArraySlice;
    region.srcSubresource.layerCount = blitDesc.LayerCount;

    region.srcOffsets[0] = {
        blitDesc.SrcRegion.OffsetX,
        blitDesc.SrcRegion.OffsetY,
        blitDesc.SrcRegion.OffsetZ
    };

    region.srcOffsets[1] = {
        blitDesc.SrcRegion.OffsetX + (int32_t)blitDesc.SrcRegion.Width,
        blitDesc.SrcRegion.OffsetY + (int32_t)blitDesc.SrcRegion.Height,
        blitDesc.SrcRegion.OffsetZ + (int32_t)(blitDesc.SrcRegion.Depth == 0 ? 1 : blitDesc.SrcRegion.Depth)
    };

    // ---- Dst ----
    region.dstSubresource.aspectMask = vkDst->GetAspectFlags();
    region.dstSubresource.mipLevel = blitDesc.DstMipIndex;
    region.dstSubresource.baseArrayLayer = blitDesc.DstArraySlice;
    region.dstSubresource.layerCount = blitDesc.LayerCount;

    region.dstOffsets[0] = {
        blitDesc.DstRegion.OffsetX,
        blitDesc.DstRegion.OffsetY,
        blitDesc.DstRegion.OffsetZ
    };

    region.dstOffsets[1] = {
        blitDesc.DstRegion.OffsetX + (int32_t)blitDesc.DstRegion.Width,
        blitDesc.DstRegion.OffsetY + (int32_t)blitDesc.DstRegion.Height,
        blitDesc.DstRegion.OffsetZ + (int32_t)(blitDesc.DstRegion.Depth == 0 ? 1 : blitDesc.DstRegion.Depth)
    };

    // =========================
    // Filter ӳ��
    // =========================
    VkFilter vkFilter = VK_FILTER_LINEAR;
    switch (blitDesc.Filter)
    {
    case ERHIFilter::Nearest:
        vkFilter = VK_FILTER_NEAREST;
        break;
    case ERHIFilter::Linear:
        vkFilter = VK_FILTER_LINEAR;
        break;
    default:
        vkFilter = VK_FILTER_LINEAR;
        break;
    }

    // =========================
    // �ύ����
    // =========================
    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();

    VKFunc::CmdBlitImage(
        commandBuffer->GetHandle(),
        srcImage,
        srcImageLayout,
        dstImage,
        dstImageLayout,
        1,
        &region,
        vkFilter);
}


VulkanComputeContext* VulkanComputeContext::CastFrom(RHIComputeContext* context)
{
    return dynamic_cast<VulkanComputeContext*>(context);
}

VulkanComputeContext::VulkanComputeContext(VulkanDevice* device, VulkanQueue* queue)
    : VulkanCommandContext(device, queue)
{
    PendingCompute = new VulkanPendingComputeState(device, this);
    LooseUniformDataUploader = new VulkanLooseUniformDataUploader(device);

}

VulkanComputeContext::~VulkanComputeContext()
{
    delete PendingCompute;
    delete LooseUniformDataUploader;
}

void VulkanComputeContext::SetBatchedShaderParameters(RHIComputeShader* shader, const RHIBatchedShaderParameters& parameter)
{
    if (!PendingCompute)
    {
        return;
    }

    auto shaderType = shader->GetShaderType();

    for (const auto& uniformParam : parameter.UniformParameters)
    {
        const uint8_t* valuePtr = parameter.Data.data() + uniformParam.Offset;
        if (shaderType == RHI::ERHIShaderFrequency::Compute)
        {
            PendingCompute->SetShaderParameter(shaderType, uniformParam.BufferIndex, uniformParam.BaseIndex, uniformParam.Size, valuePtr);
        }
    }

    for (const auto& resourceParam : parameter.ResourceParameters)
    {
        if (shaderType != RHI::ERHIShaderFrequency::Compute)
        {
            continue;
        }

        switch (resourceParam.Type)
        {
        case RHIShaderResourceParameter::EType::Texture:
            PendingCompute->SetTexture(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanTexture*>(resourceParam.GetResourceAs<RHITexture>()));
            break;
        case RHIShaderResourceParameter::EType::SRV:
            PendingCompute->SetSRV(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanShaderResourceView*>(resourceParam.GetResourceAs<RHIShaderResourceView>()));
            break;
        case RHIShaderResourceParameter::EType::UAV:
            PendingCompute->SetUAV(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanUnorderedAccessView*>(resourceParam.GetResourceAs<RHIUnorderedAccessView>()));
            break;
        case RHIShaderResourceParameter::EType::Sampler:
            PendingCompute->SetSampler(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanSampler*>(resourceParam.GetResourceAs<RHISampler>()));
            break;
        case RHIShaderResourceParameter::EType::UniformBuffer:
            PendingCompute->SetUniformBuffer(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanBuffer*>(resourceParam.GetResourceAs<RHIBuffer>()));
            break;
        default:
            break;
        }
    }
}

void VulkanComputeContext::SetComputePipelineState(RHIComputePipelineState* pipelineState)
{
    auto vkPipeline = static_cast<VulkanComputePipelineState*>(pipelineState);
    if (PendingCompute)
    {
        PendingCompute->SetPipeline(vkPipeline);
    }
}

void VulkanComputeContext::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();
    if (PendingCompute && PendingCompute->HasPipeline())
    {
        PendingCompute->PrepareForDispatch(commandBuffer);
        VKFunc::CmdDispatch(commandBuffer->GetHandle(), groupCountX, groupCountY, groupCountZ);
    }
}

void VulkanComputeContext::InnerBegin()
{
    if (PendingCompute) {
        PendingCompute->Reset();
    }
}

VulkanGraphicContext* VulkanGraphicContext::CastFrom(RHI::RHIGraphicContex* context)
{
    return dynamic_cast<VulkanGraphicContext*>(context);
}

VulkanGraphicContext::VulkanGraphicContext(VulkanDevice* device, VulkanQueue* queue)
    : VulkanCommandContext(device, queue)
{
    PendingGfx = new VulkanPendingGfxState(device, this);
    PendingRayTracing = new VulkanPendingRayTracingState(device, this);
    LooseUniformDataUploader = new VulkanLooseUniformDataUploader(device);
}

VulkanGraphicContext::~VulkanGraphicContext()
{
    delete PendingGfx;
    delete PendingRayTracing;
    delete LooseUniformDataUploader;
}

void VulkanGraphicContext::SetBatchedShaderParameters(RHIGraphicShader* shader, const RHIBatchedShaderParameters& parameter)
{
    if (!PendingGfx)
    {
        return;
    }

    auto shaderType = shader->GetShaderType();

    for (const auto& uniformParam : parameter.UniformParameters)
    {
        const uint8_t* valuePtr = parameter.Data.data() + uniformParam.Offset;
        switch (shaderType)
        {
        case RHI::ERHIShaderFrequency::Vertex:
        case RHI::ERHIShaderFrequency::Fragment:
        case RHI::ERHIShaderFrequency::Geometry:
        case RHI::ERHIShaderFrequency::Mesh:
        case RHI::ERHIShaderFrequency::Task:
            PendingGfx->SetShaderParameter(shaderType, uniformParam.BufferIndex, uniformParam.BaseIndex, uniformParam.Size, valuePtr);
            break;
        default:
            break;
        }
    }

    for (const auto& resourceParam : parameter.ResourceParameters)
    {
        const bool isGraphicsStage =
            shaderType == RHI::ERHIShaderFrequency::Vertex
            || shaderType == RHI::ERHIShaderFrequency::Fragment
            || shaderType == RHI::ERHIShaderFrequency::Geometry
            || shaderType == RHI::ERHIShaderFrequency::Mesh
            || shaderType == RHI::ERHIShaderFrequency::Task;

        if (!isGraphicsStage)
        {
            continue;
        }

        switch (resourceParam.Type)
        {
        case RHIShaderResourceParameter::EType::Texture:
            PendingGfx->SetTexture(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanTexture*>(resourceParam.GetResourceAs<RHITexture>()));
            break;
        case RHIShaderResourceParameter::EType::SRV:
            PendingGfx->SetSRV(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanShaderResourceView*>(resourceParam.GetResourceAs<RHIShaderResourceView>()));
            break;
        case RHIShaderResourceParameter::EType::UAV:
            PendingGfx->SetUAV(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanUnorderedAccessView*>(resourceParam.GetResourceAs<RHIUnorderedAccessView>()));
            break;
        case RHIShaderResourceParameter::EType::Sampler:
            PendingGfx->SetSampler(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanSampler*>(resourceParam.GetResourceAs<RHISampler>()));
            break;
        case RHIShaderResourceParameter::EType::UniformBuffer:
            PendingGfx->SetUniformBuffer(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanBuffer*>(resourceParam.GetResourceAs<RHIBuffer>()));
            break;
        default:
            break;
        }
    }
}

void VulkanGraphicContext::SetStreamSource(uint32_t streamIndex, RHIBuffer* VertexBuffer, uint32_t Offset)
{
    VulkanBuffer* vulkanVertexBuffer = dynamic_cast<VulkanBuffer*>(VertexBuffer);
    VkDeviceSize offset = Offset;
    if (PendingGfx && PendingGfx->HasPipeline())
    {
        PendingGfx->SetVertexStream(streamIndex, vulkanVertexBuffer->GetHandle(), offset);
    }
}

void VulkanGraphicContext::SetGraphicPipelineState(RHIGraphicsPipelineState* pipelineState)
{
    auto vkPipeline = static_cast<VulkanGraphicsPipelineState*>(pipelineState);
    if (PendingGfx)
    {
        PendingGfx->SetPipeline(vkPipeline);
    }
}

void VulkanGraphicContext::SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth)
{
    VkViewport vkViewport = {};
    vkViewport.x = static_cast<float>(x);
    vkViewport.y = static_cast<float>(y);
    vkViewport.width = static_cast<float>(w);
    vkViewport.height = static_cast<float>(h);
    vkViewport.minDepth = minDepth;
    vkViewport.maxDepth = maxDepth;
    if (PendingGfx && PendingGfx->HasPipeline())
    {
        PendingGfx->SetViewport(vkViewport);
    }
}

void VulkanGraphicContext::SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h)
{
    VkRect2D scissorRect = {};
    scissorRect.offset = { x, y };
    scissorRect.extent = { w, h };
    if (PendingGfx && PendingGfx->HasPipeline())
    {
        PendingGfx->SetScissor(scissorRect);
    }
}

void VulkanGraphicContext::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();

    if (PendingGfx && PendingGfx->HasPipeline())
    {
        PendingGfx->PrepareForDraw(commandBuffer);
        VKFunc::CmdDraw(commandBuffer->GetHandle(), vertexCount, instanceCount, firstVertex, firstInstance);
    }
}

void VulkanGraphicContext::DrawIndexed(RHIBuffer* indexBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();
    if (PendingGfx && PendingGfx->HasPipeline())
    {
        PendingGfx->PrepareForDraw(commandBuffer);
        VulkanBuffer* vulkanIndexBuffer = static_cast<VulkanBuffer*>(indexBuffer);
        VkIndexType indexType = vulkanIndexBuffer->GetDesc().Stride == 4 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
        VKFunc::CmdBindIndexBuffer(commandBuffer->GetHandle(), static_cast<VulkanBuffer*>(indexBuffer)->GetHandle(), 0, indexType);
        VKFunc::CmdDrawIndexed(commandBuffer->GetHandle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
}

void VulkanGraphicContext::SetBatchedShaderParameters(RHIRayTracingShader* shader, const RHIBatchedShaderParameters& parameter)
{
    if (!PendingRayTracing)
    {
        return;
    }

    auto shaderType = shader->GetShaderType();

    for (const auto& uniformParam : parameter.UniformParameters)
    {

        const uint8_t* valuePtr = parameter.Data.data() + uniformParam.Offset;
        switch (shaderType)
        {
        case RHI::ERHIShaderFrequency::RayGen:
        case RHI::ERHIShaderFrequency::ClosestHit:
        case RHI::ERHIShaderFrequency::Miss:
        case RHI::ERHIShaderFrequency::AnyHit:
        case RHI::ERHIShaderFrequency::Intersection:
        case RHI::ERHIShaderFrequency::Callable:
            PendingRayTracing->SetShaderParameter(shaderType, uniformParam.BufferIndex, uniformParam.BaseIndex, uniformParam.Size, valuePtr);
            break;
        default:
            break;
        }
    }

    for (const auto& resourceParam : parameter.ResourceParameters)
    {
        const bool isRayTracingStage =
            shaderType == RHI::ERHIShaderFrequency::RayGen
            || shaderType == RHI::ERHIShaderFrequency::ClosestHit
            || shaderType == RHI::ERHIShaderFrequency::Miss
            || shaderType == RHI::ERHIShaderFrequency::AnyHit
            || shaderType == RHI::ERHIShaderFrequency::Intersection
            || shaderType == RHI::ERHIShaderFrequency::Callable;

        if (!isRayTracingStage)
        {
            continue;
        }

        switch (resourceParam.Type)
        {
        case RHIShaderResourceParameter::EType::Texture:
            PendingRayTracing->SetTexture(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanTexture*>(resourceParam.GetResourceAs<RHITexture>()));
            break;
        case RHIShaderResourceParameter::EType::SRV:
            PendingRayTracing->SetSRV(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanShaderResourceView*>(resourceParam.GetResourceAs<RHIShaderResourceView>()));
            break;
        case RHIShaderResourceParameter::EType::UAV:
            PendingRayTracing->SetUAV(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanUnorderedAccessView*>(resourceParam.GetResourceAs<RHIUnorderedAccessView>()));
            break;
        case RHIShaderResourceParameter::EType::Sampler:
            PendingRayTracing->SetSampler(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanSampler*>(resourceParam.GetResourceAs<RHISampler>()));
            break;
        case RHIShaderResourceParameter::EType::UniformBuffer:
            PendingRayTracing->SetUniformBuffer(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanBuffer*>(resourceParam.GetResourceAs<RHIBuffer>()));
            break;
        case RHIShaderResourceParameter::EType::AccelerationStructure:
            PendingRayTracing->SetAccelerationStructure(shaderType, resourceParam.Index, resourceParam.ArrayIndex, static_cast<VulkanRayTracingInstance*>(resourceParam.GetResourceAs<RHIRayTracingInstance>()));
            break;
        default:
            break;
        }
    }
}

void VulkanGraphicContext::SetRayTracingPipelineState(RHIRayTracingPipelineState* pipelineState)
{
    if (!PendingRayTracing)
    {
        return;
    }

    auto vkPipeline = dynamic_cast<VulkanRayTracingPipeline*>(pipelineState);
    PendingRayTracing->SetPipeline(vkPipeline);
}

void VulkanGraphicContext::BuildAccelerationStructure(RHIRayTracingAccelerationStructure* accelerationStructure)
{
    if (!PendingRayTracing || !accelerationStructure)
    {
        return;
    }

    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();
    PendingRayTracing->BuildAccelerationStructure(commandBuffer, accelerationStructure, false);
}

void VulkanGraphicContext::UpdateAccelerationStructure(RHIRayTracingAccelerationStructure* accelerationStructure)
{
    if (!PendingRayTracing || !accelerationStructure)
    {
        return;
    }

    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();
    PendingRayTracing->BuildAccelerationStructure(commandBuffer, accelerationStructure, true);
}

void VulkanGraphicContext::TraceRays(uint32_t width, uint32_t height, uint32_t depth)
{
    if (!PendingRayTracing || !PendingRayTracing->HasPipeline())
    {
        return;
    }

    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();
    PendingRayTracing->PrepareForTraceRays(commandBuffer);

    const auto& raygenShaderBindingTable = PendingRayTracing->GetRayGenShaderBindingTableRegion();
    const auto& missShaderBindingTable = PendingRayTracing->GetMissShaderBindingTableRegion();
    const auto& hitShaderBindingTable = PendingRayTracing->GetHitShaderBindingTableRegion();
    const auto& callableShaderBindingTable = PendingRayTracing->GetCallableShaderBindingTableRegion();

    VKFunc::CmdTraceRaysKHR(
        commandBuffer->GetHandle(),
        &raygenShaderBindingTable,
        &missShaderBindingTable,
        &hitShaderBindingTable,
        &callableShaderBindingTable,
        width,
        height,
        depth);
}

void VulkanGraphicContext::InnerBegin()
{
    if (PendingGfx) {
        PendingGfx->Reset();
    }
    if (PendingRayTracing) {
        PendingRayTracing->Reset();
    }
}

void VulkanGraphicContext::BeginRenderPass(const RHIRenderPassInfo& renderPassInfo)
{
    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();
    device->GetRenderPassManager()->BeginRenderPass(commandBuffer, renderPassInfo);
}

void VulkanGraphicContext::EndRenderPass()
{
    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();
    device->GetRenderPassManager()->EndRenderPass(commandBuffer);
}

} // namespace WR::RHIVulkan
