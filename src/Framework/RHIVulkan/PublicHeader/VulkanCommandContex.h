// VulkanContexts.h
#pragma once

#include "RHICommandContex.h"
#include "VulkanDevice.h"
#include "VulkanPipelineState.h"
#include "VulkanPendingPipelineState.h"
#include "VulkanResource.h"
#include "VulkanMemory.h"
#include "RHICommandList.h"
#include "VulkanCommandBuffer.h"
#include "VulkanBarriers.h"
#include "VulkanFuncWrapper.h"


using namespace RHI;

namespace RHIVulkan {
class VulkanDescriptorSet;
class VulkanCommandBufferManager;


// -------------------------------------------------------------------------------------------------
// Vulkan Graphics Context
// -------------------------------------------------------------------------------------------------
class RHIVULKAN_API VulkanCommandContext : public RHICommandContext
{
public:
    inline static VulkanCommandContext* CastFrom(RHIComputeContext* context);
    VulkanCommandContext(VulkanDevice* device, VulkanQueue* queue);
    virtual ~VulkanCommandContext();
    void SetBatchedShaderParameters(RHIComputeShader* shader, const RHIBatchedShaderParameters& parameter) override;
    void SetBatchedShaderParameters(RHIGraphicShader* shader, const RHIBatchedShaderParameters& parameter) override;

    // Compute接口
    void SetComputePipelineState(RHIComputePipelineState* pipelineState) override;
    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
    void CopyTexture(RHITexture* src, RHITexture* dst, const RHICopyTextureDesc& copyDesc) override;

    virtual void SetStreamSource(uint32_t streamIndex, RHIBufferSP VertexBuffer, uint32_t Offset) override;

    // Graphics接口
    void SetGraphicPipelineState(RHIGraphicsPipelineState* pipelineState) override;
    virtual void SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth) override;
    virtual void SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h) override;
    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
    void DrawIndexed(RHIBuffer* indexBuffer, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;

    // RayTracing接口
    void SetRayTracingPipelineState(RHIRayTracingPipelineState* pipelineState) override;
    void SetShaderTable() override;
    void TraceRays(uint32_t width, uint32_t height, uint32_t depth = 1) override;

    // 域
    void Begin() override;
    RHICmdBuffer End() override;
    void BeginRenderPass(const RHIRenderPassInfo& renderPassInfo) override;
    void EndRenderPass() override;
    void BeginTransitions(std::vector<const RHITransition*> Transitions) override;
    void EndTransitions(std::vector<const RHITransition*> Transitions) override;


    VulkanQueue* GetQueue() const { return queue; }
    VulkanCommandBufferManager* GetCommandBufferManager() const { return commandBufferManager; }
    VulkanDevice* GetDevice() const { return device; }
	VulkanLooseUniformDataUploader* GetLooseUniformDataUploader() const { return LooseUniformDataUploader; }

private:
    void SetShaderTextureInternal(RHIShader* shader, uint32_t textureIndex, RHITexture* texture);
    void SetShaderSamplerInternal(RHIShader* shader, uint32_t samplerIndex, RHISampler* sampler);
    void SetShaderUAVInternal(RHIShader* shader, uint32_t uavIndex, RHIUnorderedAccessView* uav);
    void SetShaderSRVInternal(RHIShader* shader, uint32_t srvIndex, RHIShaderResourceView* srv);
    void SetShaderUniformBufferInternal(RHIShader* shader, uint32_t bufferIndex, RHIBuffer* buffer);

    VulkanDevice* device;
    VulkanQueue* queue;
    VulkanCommandBufferManager* commandBufferManager;

    VulkanPendingGfxState*     PendingGfx;
    VulkanPendingComputeState* PendingCompute;
    VulkanLooseUniformDataUploader* LooseUniformDataUploader;

};


struct VulkanCommandUpdateTexture : public RHICommandBase
{
    VulkanTexture* texture;
    std::shared_ptr<VulkanStagingBuffer> staging;
    VkBufferImageCopy copyRegion{};
    VulkanCommandUpdateTexture(VulkanTexture* texture, std::shared_ptr<VulkanStagingBuffer> buffer, VkBufferImageCopy region) : texture(texture), staging(buffer) , copyRegion(region){}
    void Execute(RHICommandList& cmdList) override {

        auto contex = cmdList.GetCommandContex();
        VulkanCommandContext* vulkanContex = dynamic_cast<VulkanCommandContext*>(contex);
        // 2. 获取一个可用命令缓冲区
        VulkanCommandBuffer* cmdBuffer = vulkanContex->GetCommandBufferManager()->BeginUploadCommandBuffer();

        VulkanPipelineBarrier barrier;
        VkImageSubresourceRange transientRegion = VulkanPipelineBarrier::MakeSubresourceRange(copyRegion.imageSubresource.aspectMask,
            copyRegion.imageSubresource.mipLevel,
            1, copyRegion.imageSubresource.baseArrayLayer,
            copyRegion.imageSubresource.layerCount);
        auto layout = cmdBuffer->GetImageLayoutManager()->GetFullLayout(texture->GetImage());

		barrier.TransitionLayout(
			texture->GetImage(),
            *layout,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            transientRegion
		);
        barrier.Execute(cmdBuffer);
        //记录当前的修改待提交
        cmdBuffer->GetImageLayoutManager()->SetLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, transientRegion);


        CmdCopyBufferToImage(
            cmdBuffer->GetHandle(), // Vulkan 命令缓冲区
            staging->GetHandle(), // staging buffer
            texture->GetImage(), // 目标纹理
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // 假设已经是 transfer dst layout
            1,
            &copyRegion
        );
        vulkanContex->GetCommandBufferManager()->EndAndSubmitUploadCommandBuffer(cmdBuffer);
        vulkanContex->GetDevice()->GetStagingManager()->ReleaseToCmdBuffer(cmdBuffer, staging);
        vulkanContex->GetDevice()->GetStagingManager()->OnCommandBufferSubmitted(cmdBuffer);

        
    }
};

struct VulkanCommandUpdateBuffer : public RHICommandBase
{
    VulkanBuffer* buffer;
    std::shared_ptr<VulkanStagingBuffer> staging;
    VkBufferCopy copyRegion{};

    VulkanCommandUpdateBuffer(
        VulkanBuffer* buffer,
        std::shared_ptr<VulkanStagingBuffer> stagingBuffer,
        VkBufferCopy region)
        : buffer(buffer), staging(stagingBuffer), copyRegion(region) {
    }

    void Execute(RHICommandList& cmdList) override
    {
        auto context = cmdList.GetCommandContex();
        VulkanCommandContext* vulkanContext = dynamic_cast<VulkanCommandContext*>(context);

        // 1. 获取 upload command buffer
        VulkanCommandBuffer* cmdBuffer =
            vulkanContext->GetCommandBufferManager()->BeginUploadCommandBuffer();

        // 2. 执行 copy
        vkCmdCopyBuffer(
            cmdBuffer->GetHandle(),
            staging->GetHandle(),
            buffer->GetHandle(),
            1,
            &copyRegion
        );

        // 3. 提交 command buffer
        vulkanContext->GetCommandBufferManager()->EndAndSubmitUploadCommandBuffer(cmdBuffer);

        // 4. staging 生命周期绑定到 cmdBuffer
        vulkanContext->GetDevice()->GetStagingManager()->ReleaseToCmdBuffer(cmdBuffer, staging);
        vulkanContext->GetDevice()->GetStagingManager()->OnCommandBufferSubmitted(cmdBuffer);
    }
};


struct RHIVULKAN_API VulkanCommandInitializeImageState : public RHICommandBase
{
    VulkanTexture* texture;
    VkImageLayout InitialLayout;
    bool onlyUpdateImageLayout;
    
    VulkanCommandInitializeImageState(VulkanTexture* texture, VkImageLayout InitialLayout, bool onUpdateImageLayout) : texture(texture), InitialLayout(InitialLayout), onlyUpdateImageLayout(onlyUpdateImageLayout) {}
    void Execute(RHICommandList& cmdList) override {
        if (onlyUpdateImageLayout) {
            
        }
        else {
            auto commmandContex = cmdList.GetCommandContex();
            VulkanCommandContext* vulkanContex = VulkanCommandContext::CastFrom(commmandContex);
            texture->InitialImageState(vulkanContex, InitialLayout);
        }
    }
};



} // namespace WR::RHIVulkan