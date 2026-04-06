#include "VulkanCommandContex.h"
#include "VulkanCommandBuffer.h"
#include "VulkanQueue.h"
#include "VulkanShader.h"
#include "VulkanDescriptorSets.h"

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
}

RHICmdBuffer VulkanCommandContext::End()
{
    device->GetDescriptorSetManager()->GarbageCollect();
    commandBufferManager->GarbageCollect();
    device->ReleaseDeferredResources();

    auto commandBuffer = commandBufferManager->EndActiveCommandBuffer();
    if (!commandBuffer)
    {
        return 0;
    }
    return reinterpret_cast<RHICmdBuffer>(commandBuffer);
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

VulkanTransferContext* VulkanTransferContext::CastFrom(RHITransferContext* context)
{
    return dynamic_cast<VulkanTransferContext*>(context);
}

VulkanTransferContext::VulkanTransferContext(VulkanDevice* device, VulkanQueue* queue)
    : VulkanCommandContext(device, queue)
{
}

void VulkanTransferContext::CopyTexture(RHITexture* src, RHITexture* dst, const RHICopyTextureDesc& copyDesc)
{
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
            PendingCompute->SetShaderParameter(shaderType, uniformParam.BaseIndex, uniformParam.Size, valuePtr);
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
            PendingCompute->SetTexture(shaderType, resourceParam.Index, static_cast<VulkanTexture*>(resourceParam.GetResourceAs<RHITexture>()));
            break;
        case RHIShaderResourceParameter::EType::SRV:
            PendingCompute->SetSRV(shaderType, resourceParam.Index, static_cast<VulkanShaderResourceView*>(resourceParam.GetResourceAs<RHIShaderResourceView>()));
            break;
        case RHIShaderResourceParameter::EType::UAV:
            PendingCompute->SetUAV(shaderType, resourceParam.Index, static_cast<VulkanUnorderedAccessView*>(resourceParam.GetResourceAs<RHIUnorderedAccessView>()));
            break;
        case RHIShaderResourceParameter::EType::Sampler:
            PendingCompute->SetSampler(shaderType, resourceParam.Index, static_cast<VulkanSampler*>(resourceParam.GetResourceAs<RHISampler>()));
            break;
        case RHIShaderResourceParameter::EType::UniformBuffer:
            PendingCompute->SetUniformBuffer(shaderType, resourceParam.Index, static_cast<VulkanBuffer*>(resourceParam.GetResourceAs<RHIBuffer>()));
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
        vkCmdDispatch(commandBuffer->GetHandle(), groupCountX, groupCountY, groupCountZ);
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
    LooseUniformDataUploader = new VulkanLooseUniformDataUploader(device);
}

VulkanGraphicContext::~VulkanGraphicContext()
{
    delete PendingGfx;
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
            PendingGfx->SetShaderParameter(shaderType, uniformParam.BaseIndex, uniformParam.Size, valuePtr);
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
            PendingGfx->SetTexture(shaderType, resourceParam.Index, static_cast<VulkanTexture*>(resourceParam.GetResourceAs<RHITexture>()));
            break;
        case RHIShaderResourceParameter::EType::SRV:
            PendingGfx->SetSRV(shaderType, resourceParam.Index, static_cast<VulkanShaderResourceView*>(resourceParam.GetResourceAs<RHIShaderResourceView>()));
            break;
        case RHIShaderResourceParameter::EType::UAV:
            PendingGfx->SetUAV(shaderType, resourceParam.Index, static_cast<VulkanUnorderedAccessView*>(resourceParam.GetResourceAs<RHIUnorderedAccessView>()));
            break;
        case RHIShaderResourceParameter::EType::Sampler:
            PendingGfx->SetSampler(shaderType, resourceParam.Index, static_cast<VulkanSampler*>(resourceParam.GetResourceAs<RHISampler>()));
            break;
        case RHIShaderResourceParameter::EType::UniformBuffer:
            PendingGfx->SetUniformBuffer(shaderType, resourceParam.Index, static_cast<VulkanBuffer*>(resourceParam.GetResourceAs<RHIBuffer>()));
            break;
        default:
            break;
        }
    }
}

void VulkanGraphicContext::SetStreamSource(uint32_t streamIndex, RHIBufferSP VertexBuffer, uint32_t Offset)
{
    VulkanBuffer* vulkanVertexBuffer = dynamic_cast<VulkanBuffer*>(VertexBuffer.get());
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
        vkCmdDraw(commandBuffer->GetHandle(), vertexCount, instanceCount, firstVertex, firstInstance);
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
        vkCmdBindIndexBuffer(commandBuffer->GetHandle(), static_cast<VulkanBuffer*>(indexBuffer)->GetHandle(), 0, indexType);
        vkCmdDrawIndexed(commandBuffer->GetHandle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
}

void VulkanGraphicContext::SetRayTracingPipelineState(RHIRayTracingPipelineState* pipelineState)
{
}

void VulkanGraphicContext::SetShaderTable()
{
}

void VulkanGraphicContext::TraceRays(uint32_t width, uint32_t height, uint32_t depth)
{
    VkCommandBuffer commandBuffer = commandBufferManager->GetActiveCommandBuffer()->GetHandle();
    VkStridedDeviceAddressRegionKHR raygenShaderBindingTable = {};
    VkStridedDeviceAddressRegionKHR missShaderBindingTable = {};
    VkStridedDeviceAddressRegionKHR hitShaderBindingTable = {};
    VkStridedDeviceAddressRegionKHR callableShaderBindingTable = {};
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
