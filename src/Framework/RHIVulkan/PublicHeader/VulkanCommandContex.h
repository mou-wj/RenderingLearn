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
        // 使用active命令缓冲区，隐式layout转换由调用方通过RHI Transition系统显式处理
        VulkanCommandBuffer* cmdBuffer = vulkanContex->GetCommandBufferManager()->GetActiveCommandBuffer();

        CmdCopyBufferToImage(
            cmdBuffer->GetHandle(),
            staging->GetHandle(),
            texture->GetImage(),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &copyRegion
        );
        vulkanContex->GetDevice()->GetStagingManager()->ReleaseToCmdBuffer(cmdBuffer, staging);
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

        VulkanCommandBuffer* cmdBuffer =
            vulkanContext->GetCommandBufferManager()->GetActiveCommandBuffer();

        vkCmdCopyBuffer(
            cmdBuffer->GetHandle(),
            staging->GetHandle(),
            buffer->GetHandle(),
            1,
            &copyRegion
        );

        vulkanContext->GetDevice()->GetStagingManager()->ReleaseToCmdBuffer(cmdBuffer, staging);
    }
};



} // namespace WR::RHIVulkan