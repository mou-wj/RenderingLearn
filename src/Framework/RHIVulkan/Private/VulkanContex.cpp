#include "VulkanCommandContex.h"
#include "VulkanCommandBuffer.h"
#include "VulkanQueue.h"
#include "VulkanShader.h"
#include "VulkanDescriptorSets.h"

namespace RHIVulkan {
    VulkanCommandContext* VulkanCommandContext::CastFrom(RHICommandContex* context)
    {
        return dynamic_cast<VulkanCommandContext*>(context);
    }
    // -------------------------------------------------------------------------------------------------
// Vulkan Command Context Implementation
// -------------------------------------------------------------------------------------------------
VulkanCommandContext::VulkanCommandContext(VulkanDevice* device, VulkanQueue* queue)
    : device(device)
    , queue(queue)
    , commandBufferManager(new VulkanCommandBufferManager(device,this))
    ,PendingGfx(new VulkanPendingGfxState(device,this))
    ,PendingCompute(new VulkanPendingComputeState(device,this))
	, LooseUniformDataUploader(new VulkanLooseUniformDataUploader(device))
{
    

}

VulkanCommandContext::~VulkanCommandContext() {
    delete commandBufferManager;
	delete PendingGfx;
	delete PendingCompute;
    delete LooseUniformDataUploader;
}

void VulkanCommandContext::RHISetShaderTexture(RHIShader* Shader, uint32_t TextureIndex, RHITexture* Texture)
{
	auto shaderType = Shader->GetShaderType();
	VulkanTexture* vulkanTexture = static_cast<VulkanTexture*>(Texture);
    switch (shaderType)
    {
    case RHI::ERHIShaderFrequency::Vertex:
    case RHI::ERHIShaderFrequency::Fragment:
    case RHI::ERHIShaderFrequency::Geometry:
    case RHI::ERHIShaderFrequency::Mesh:
    case RHI::ERHIShaderFrequency::Task:
    {
        PendingGfx->SetTexture(shaderType, TextureIndex, vulkanTexture);
        break;
    }
    case RHI::ERHIShaderFrequency::Compute:
    {
        PendingCompute->SetTexture(shaderType, TextureIndex, vulkanTexture);
        break;
    }

    case RHI::ERHIShaderFrequency::TessControl:
    case RHI::ERHIShaderFrequency::TessEvaluation:
    case RHI::ERHIShaderFrequency::RayGen:
    case RHI::ERHIShaderFrequency::ClosestHit:
    case RHI::ERHIShaderFrequency::Miss:
    case RHI::ERHIShaderFrequency::AnyHit:
    case RHI::ERHIShaderFrequency::Intersection:
    case RHI::ERHIShaderFrequency::Callable:
    {
        //to do
        break;
    }
    default:
        break;
    }
}

void VulkanCommandContext::RHISetShaderSampler(RHIShader* Shader, uint32_t SamplerIndex, RHISampler* NewState)
{
    auto shaderType = Shader->GetShaderType();
    // 假设你有 VulkanSampler 对象
    VulkanSampler* sampler = static_cast<VulkanSampler*>(NewState);

    switch (shaderType)
    {
    case RHI::ERHIShaderFrequency::Vertex:
    case RHI::ERHIShaderFrequency::Fragment:
    case RHI::ERHIShaderFrequency::Geometry:
    case RHI::ERHIShaderFrequency::Mesh:
    case RHI::ERHIShaderFrequency::Task:
        PendingGfx->SetSampler(shaderType, SamplerIndex, sampler);
        break;

    case RHI::ERHIShaderFrequency::Compute:
        PendingCompute->SetSampler(shaderType, SamplerIndex, sampler);
        break;

    default:
        break;
    }
}

void VulkanCommandContext::RHISetUAVParameter(RHIShader* Shader, uint32_t UAVIndex, RHIUnorderedAccessView* UAV)
{
    auto shaderType = Shader->GetShaderType();
    VulkanUnorderedAccessView* vulkanUAV = static_cast<VulkanUnorderedAccessView*>(UAV);

    switch (shaderType)
    {
    case RHI::ERHIShaderFrequency::Vertex:
    case RHI::ERHIShaderFrequency::Fragment:
    case RHI::ERHIShaderFrequency::Geometry:
    case RHI::ERHIShaderFrequency::Mesh:
    case RHI::ERHIShaderFrequency::Task:
        PendingGfx->SetUAV(shaderType, UAVIndex, vulkanUAV);
        break;

    case RHI::ERHIShaderFrequency::Compute:
        PendingCompute->SetUAV(shaderType, UAVIndex, vulkanUAV);
        break;

    default:
        break;
    }

}


void VulkanCommandContext::RHISetShaderResourceViewParameter(RHIShader* Shader, uint32_t SRVIndex, RHIShaderResourceView* SRV)
{
    auto shaderType = Shader->GetShaderType();
    VulkanShaderResourceView* vulkanSRV = static_cast<VulkanShaderResourceView*>(SRV);

    switch (shaderType)
    {
    case RHI::ERHIShaderFrequency::Vertex:
    case RHI::ERHIShaderFrequency::Fragment:
    case RHI::ERHIShaderFrequency::Geometry:
    case RHI::ERHIShaderFrequency::Mesh:
    case RHI::ERHIShaderFrequency::Task:
        PendingGfx->SetSRV(shaderType, SRVIndex, vulkanSRV);
        break;

    case RHI::ERHIShaderFrequency::Compute:
        PendingCompute->SetSRV(shaderType, SRVIndex, vulkanSRV);
        break;

    default:
        break;
    }
}

void VulkanCommandContext::RHISetShaderUniformBuffer(RHIShader* Shader, uint32_t BufferIndex, RHIBuffer* Buffer)
{
    auto shaderType = Shader->GetShaderType();
    VulkanBuffer* vulkanBuffer = static_cast<VulkanBuffer*>(Buffer);

    switch (shaderType)
    {
    case RHI::ERHIShaderFrequency::Vertex:
    case RHI::ERHIShaderFrequency::Fragment:
    case RHI::ERHIShaderFrequency::Geometry:
    case RHI::ERHIShaderFrequency::Mesh:
    case RHI::ERHIShaderFrequency::Task:
        PendingGfx->SetUniformBuffer(shaderType, BufferIndex, vulkanBuffer);
        break;

    case RHI::ERHIShaderFrequency::Compute:
        PendingCompute->SetUniformBuffer(shaderType, BufferIndex, vulkanBuffer);
        break;

    default:
        break;
    }
}
void VulkanCommandContext::RHISetShaderParameters(RHIShader* Shader, const std::vector<uint8_t>& InParametersData, const std::vector<RHIShaderUniformParameter>& InParameters, const std::vector<RHIShaderResourceParameter>& InResourceParameters)
{
    // 遍历 UniformBuffer 参数
    for (const auto& param : InParameters)
    {
        RHISetShaderParameter(Shader, param.BufferIndex, param.BaseIndex, param.Size, InParametersData.data() + param.Offset);
    }

    // 遍历 SRV/UAV/Texture 参数
    for (const auto& res : InResourceParameters)
    {
        switch (res.Type)
        {
        case RHIShaderResourceParameter::EType::Texture:
            RHISetShaderTexture(Shader, res.Index, res.GetResourceAs<RHITexture>());
            break;
        case RHIShaderResourceParameter::EType::SRV:
            RHISetShaderResourceViewParameter(Shader, res.Index, res.GetResourceAs<RHIShaderResourceView>());
            break;
        case RHIShaderResourceParameter::EType::UAV:
            RHISetUAVParameter(Shader, res.Index, res.GetResourceAs<RHIUnorderedAccessView>());
            break;
        case RHIShaderResourceParameter::EType::Sampler:
            RHISetShaderSampler(Shader, res.Index, res.GetResourceAs<RHISampler>());
            break;
        case RHIShaderResourceParameter::EType::UniformBuffer:
            RHISetShaderUniformBuffer(Shader, res.Index, res.GetResourceAs<RHIBuffer>());
            break;
        default:
            break;
        }
    }
}

void VulkanCommandContext::RHISetShaderParameter(RHIShader* Shader, uint32_t BufferIndex, uint32_t BaseIndex, uint32_t NumBytes, const void* NewValue)
{
    auto shaderType = Shader->GetShaderType();
    switch (shaderType)
    {
    case RHI::ERHIShaderFrequency::Vertex:
    case RHI::ERHIShaderFrequency::Fragment:
    case RHI::ERHIShaderFrequency::Geometry:
    case RHI::ERHIShaderFrequency::Mesh:
    case RHI::ERHIShaderFrequency::Task:
        PendingGfx->SetShaderParameter(shaderType, BaseIndex, NumBytes, reinterpret_cast<const uint8_t*>(NewValue));
        break;

    case RHI::ERHIShaderFrequency::Compute:
        PendingCompute->SetShaderParameter(shaderType, BaseIndex, NumBytes, reinterpret_cast<const uint8_t*>(NewValue));
        break;

    default:
        break;
    }
}

void VulkanCommandContext::SetBatchedShaderParameters(RHIShader* shader, const RHIBatchedShaderParameters& parameter)
{
	RHISetShaderParameters(shader, parameter.Data, parameter.UniformParameters, parameter.ResourceParameters);
}

void VulkanCommandContext::SetComputePipelineState(RHIComputePipelineState* pipelineState) {
    auto vkPipeline = static_cast<VulkanComputePipelineState*>(pipelineState);
    if (PendingCompute->CurrentPipeline != vkPipeline)
    {
        PendingCompute->SetPipeline(vkPipeline);
    }

}


void VulkanCommandContext::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();
    if (PendingCompute->CurrentPipeline)
    {
        PendingCompute->PrepareForDispatch(commandBuffer);
        vkCmdDispatch(commandBuffer->GetHandle(), groupCountX, groupCountY, groupCountZ);
    }


    
}

void VulkanCommandContext::CopyTexture(RHITexture* src, RHITexture* dst, const RHICopyTextureDesc& copyDesc)
{
}

void VulkanCommandContext::SetStreamSource(uint32_t streamIndex, RHIBufferSP VertexBuffer, uint32_t Offset) {
	VulkanBuffer* vulkanVertexBuffer = dynamic_cast<VulkanBuffer*>(VertexBuffer.get());
    VkCommandBuffer commandBuffer = commandBufferManager->GetActiveCommandBuffer()->GetHandle();
    VkDeviceSize offset = Offset;
    if (PendingGfx->CurrentPipeline)
    {
        PendingGfx->SetVertexStream(streamIndex, vulkanVertexBuffer->GetHandle(),offset);
    }
}

void VulkanCommandContext::SetGraphicPipelineState(RHIGraphicsPipelineState* pipelineState) {
    auto vkPipeline = static_cast<VulkanGraphicsPipelineState*>(pipelineState);
    if (PendingGfx->CurrentPipeline != vkPipeline)
    {
        PendingGfx->SetPipeline(vkPipeline);
    }

}


void VulkanCommandContext::SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth) {
	VkCommandBuffer commandBuffer = commandBufferManager->GetActiveCommandBuffer()->GetHandle();
	VkViewport vkViewport = {};
	vkViewport.x = static_cast<float>(x);
	vkViewport.y = static_cast<float>(y);
	vkViewport.width = static_cast<float>(w);
	vkViewport.height = static_cast<float>(h);
    vkViewport.minDepth = minDepth;
	vkViewport.maxDepth = maxDepth;
    if (PendingGfx->CurrentPipeline)
    {
        PendingGfx->SetViewport(vkViewport);
    }
}
void VulkanCommandContext::SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h) {
	VkCommandBuffer commandBuffer = commandBufferManager->GetActiveCommandBuffer()->GetHandle();
	VkRect2D scissorRect = {};
	scissorRect.offset = { x, y };
	scissorRect.extent = { w, h };
    if (PendingGfx->CurrentPipeline)
    {
        PendingGfx->SetScissor(scissorRect);
    }
}

void VulkanCommandContext::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();

    if (PendingGfx->CurrentPipeline)
    {
        PendingGfx->PrepareForDraw(commandBuffer);
        vkCmdDraw(commandBuffer->GetHandle(), vertexCount, instanceCount, firstVertex, firstInstance);
    }
    
}

void VulkanCommandContext::DrawIndexed(RHIBuffer* indexBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();
    if (PendingGfx->CurrentPipeline)
    {
        PendingGfx->PrepareForDraw(commandBuffer);
        VulkanBuffer* vulkanIndexBuffer = static_cast<VulkanBuffer*>(indexBuffer);
        VkIndexType indexType = vulkanIndexBuffer->GetDesc().Stride == 4 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16; // Assuming 32-bit indices, adjust if needed
        vkCmdBindIndexBuffer(commandBuffer->GetHandle(), static_cast<VulkanBuffer*>(indexBuffer)->GetHandle(), 0, indexType);
        vkCmdDrawIndexed(commandBuffer->GetHandle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

}

void VulkanCommandContext::SetRayTracingPipelineState(RHIRayTracingPipelineState* pipelineState) {


}

void VulkanCommandContext::SetShaderTable() {
    // Placeholder: Implement shader table binding logic
}

void VulkanCommandContext::TraceRays(uint32_t width, uint32_t height, uint32_t depth) {
    VkCommandBuffer commandBuffer = commandBufferManager->GetActiveCommandBuffer()->GetHandle();
    VkStridedDeviceAddressRegionKHR raygenShaderBindingTable = {};
    VkStridedDeviceAddressRegionKHR missShaderBindingTable = {};
    VkStridedDeviceAddressRegionKHR hitShaderBindingTable = {};
    VkStridedDeviceAddressRegionKHR callableShaderBindingTable = {};

    // 设置光线追踪相关的其他成员
    // 这里仅提供一个示例，具体成员需要根据pipelineDesc填充

    //vkCmdTraceRaysKHR(
    //    commandBuffer,
    //    &raygenShaderBindingTable,
    //    &missShaderBindingTable,
    //    &hitShaderBindingTable,
    //    &callableShaderBindingTable,
    //    width,
    //    height,
    //    depth
    //);

}

void VulkanCommandContext::BeginDrawingViewport(RHIViewport* Viewport, RHITexture* RenderTargetRHI)
{

}
void VulkanCommandContext::EndDrawingViewport(RHIViewport* Viewport, bool bPresent)
{
    VulkanViewport* vulkanViewport = dynamic_cast<VulkanViewport*>(Viewport);
    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();

	vulkanViewport->Present(this, commandBuffer, device->GetGraphicsQueue(), device->GetPresentQueue());
}
void VulkanCommandContext::BeginFrame()
{

}
void VulkanCommandContext::EndFrame()
{
    device->GetDescriptorSetManager()->GarbageCollect();
    commandBufferManager->GarbageCollect();
    device->ReleaseDeferredResources();

}
void VulkanCommandContext::BeginRenderPass(const RHIRenderPassInfo& renderPassInfo)
{
    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();
	device->GetRenderPassManager()->BeginRenderPass(commandBuffer, renderPassInfo);

}
void VulkanCommandContext::EndRenderPass()
{
    auto commandBuffer = commandBufferManager->GetActiveCommandBuffer();
    device->GetRenderPassManager()->EndRenderPass(commandBuffer);
}

void VulkanCommandContext::RHIBeginTransitions(std::vector<const RHITransition*> Transitions)
{
    for (const RHITransition* transition : Transitions)
	{
		const VulkanPipelineBarrier* pipelineBarrier = transition->GetPrivateData<VulkanPipelineBarrier>();
		if (pipelineBarrier)
		{
			
		}
	}
}

void VulkanCommandContext::RHIEndTransitions(std::vector<const RHITransition*> Transitions)
{
	for (const RHITransition* transition : Transitions)
	{
		const VulkanPipelineBarrier* pipelineBarrier = transition->GetPrivateData<VulkanPipelineBarrier>();
		if (pipelineBarrier)
		{
			
		}
	}
}

} // namespace WR::RHIVulkan
