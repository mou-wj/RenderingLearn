#include "VulkanCommandContex.h"
#include "VulkanCommandBuffer.h"
#include "VulkanQueue.h"

namespace RHIVulkan {

// -------------------------------------------------------------------------------------------------
// Vulkan Command Context Implementation
// -------------------------------------------------------------------------------------------------
VulkanCommandContext::VulkanCommandContext(VulkanDevice* device, VulkanQueue* queue)
    : device(device), queue(queue), commandBufferManager(new VulkanCommandBufferManager(device,this)) {}

VulkanCommandContext::~VulkanCommandContext() {

}

void VulkanCommandContext::SetComputePipelineState(const RHIComputePipelineStateSP& pipelineState) {


    ComputeState.PipelineState = std::static_pointer_cast<VulkanComputePipeline>(pipelineState);
    VkDevice deviceHandle = device->GetDevice();

}

void VulkanCommandContext::SetShaderParameter(RHIShader* shader, const RHIShaderParameterSP& parameter) {
    // Placeholder: Implement shader parameter binding logic
}
void VulkanCommandContext::SetShaderBatchedShaderParameter(RHIShader* shader, const RHIBatchedShaderParameter& parameter) {

}

void VulkanCommandContext::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    VkCommandBuffer commandBuffer = commandBufferManager->GetAvailableCommandBuffer()->GetHandle();
    vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    
}

void VulkanCommandContext::SetStreamSource(uint32_t streamIndex, RHIBufferSP VertexBuffer, uint32_t Offset) {

}

void VulkanCommandContext::SetGraphicPipelineState(const RHIGraphicsPipelineStateSP& pipelineState) {


    GraphicState.PipelineState = std::static_pointer_cast<VulkanGraphicsPipeline>(pipelineState);
    VkDevice deviceHandle = device->GetDevice();

}

void VulkanCommandContext::ViewportPresent(const RHIVIewportSP& viewport, const RHITextureSP& presentRenderTarget)
{
	// Placeholder: Implement viewport presentation logic
	// This might involve setting the viewport and scissor rect based on the provided viewport
	if (viewport) {
		
	}
	// If presentRenderTarget is used, bind it as a render target if needed
}

void VulkanCommandContext::SetViewPortRect(const RHIIntRect& viewport) {
	VkCommandBuffer commandBuffer = commandBufferManager->GetAvailableCommandBuffer()->GetHandle();
	VkViewport vkViewport = {};
	vkViewport.x = static_cast<float>(viewport.X);
	vkViewport.y = static_cast<float>(viewport.Y);
	vkViewport.width = static_cast<float>(viewport.Width);
	vkViewport.height = static_cast<float>(viewport.Height);
	vkViewport.minDepth = 0.0f;
	vkViewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &vkViewport);
}

void VulkanCommandContext::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    VkCommandBuffer commandBuffer = commandBufferManager->GetAvailableCommandBuffer()->GetHandle();
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandContext::DrawIndexed(RHIBuffer* indexBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    VkCommandBuffer commandBuffer = commandBufferManager->GetAvailableCommandBuffer()->GetHandle();
	VulkanBuffer* vulkanIndexBuffer = static_cast<VulkanBuffer*>(indexBuffer);
	VkIndexType indexType = vulkanIndexBuffer->GetDesc().Stride == 4? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16; // Assuming 32-bit indices, adjust if needed
	vkCmdBindIndexBuffer(commandBuffer, static_cast<VulkanBuffer*>(indexBuffer)->GetBuffer(), 0, indexType);
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandContext::SetRayTracingPipelineState(const RHIRayTracingPipelineStateSP& pipelineState) {


    RayTracingState.PipelineState = std::static_pointer_cast<VulkanRayTracingPipeline>(pipelineState);
    VkDevice deviceHandle = device->GetDevice();

}

void VulkanCommandContext::SetShaderTable() {
    // Placeholder: Implement shader table binding logic
}

void VulkanCommandContext::TraceRays(uint32_t width, uint32_t height, uint32_t depth) {
    VkCommandBuffer commandBuffer = commandBufferManager->GetAvailableCommandBuffer()->GetHandle();
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



} // namespace WR::RHIVulkan
