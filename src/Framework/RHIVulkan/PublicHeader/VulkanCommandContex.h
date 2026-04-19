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
class VulkanQueue;

class RHIVULKAN_API VulkanCommandContext : public virtual RHI::RHIContextBase
{
public:
    VulkanCommandContext(VulkanDevice* device, VulkanQueue* queue);
    virtual ~VulkanCommandContext();


    void Begin() override;
    void End() override;
    void BeginTransitions(std::vector<const RHITransition*> Transitions) override;
    void EndTransitions(std::vector<const RHITransition*> Transitions) override;

    VulkanQueue* GetQueue() const { return queue; }
    VulkanCommandBufferManager* GetCommandBufferManager() const { return commandBufferManager; }
    VulkanDevice* GetDevice() const { return device; }
    VulkanCommandBuffer* GetRecordedCommandBuffer() const { return RecordedCommandBuffer; }
protected:
    VulkanDevice* device = nullptr;
    VulkanQueue* queue = nullptr;
    VulkanCommandBufferManager* commandBufferManager = nullptr;
    VulkanCommandBuffer* RecordedCommandBuffer = nullptr;

};

class RHIVULKAN_API VulkanTransferContext : public VulkanCommandContext, public RHI::RHITransferContext
{
public:
    static VulkanTransferContext* CastFrom(RHITransferContext* context);

    VulkanTransferContext(VulkanDevice* device, VulkanQueue* queue);
    ~VulkanTransferContext() override = default;

    void CopyTexture(RHITexture* src, RHITexture* dst, const RHICopyTextureDesc& copyDesc) override;
    void BlitTexture(RHITexture* src, RHITexture* dst, const RHIBlitTextureDesc& blitDesc) override;
};

class RHIVULKAN_API VulkanComputeContext : public VulkanCommandContext, public RHI::RHIComputeContex
{
public:
    static VulkanComputeContext* CastFrom(RHIComputeContext* context);

    VulkanComputeContext(VulkanDevice* device, VulkanQueue* queue);
    ~VulkanComputeContext() override;

    VulkanLooseUniformDataUploader* GetLooseUniformDataUploader() const { return LooseUniformDataUploader; }

    void SetBatchedShaderParameters(RHIComputeShader* shader, const RHIBatchedShaderParameters& parameter) override;
    void SetComputePipelineState(RHIComputePipelineState* pipelineState) override;
    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

private:
    friend class VulkanCommandContext;
    VulkanPendingComputeState* PendingCompute = nullptr;
    VulkanLooseUniformDataUploader* LooseUniformDataUploader = nullptr;
};

class RHIVULKAN_API VulkanGraphicContext : public VulkanCommandContext, public RHI::RHIGraphicContex
{
public:
    static VulkanGraphicContext* CastFrom(RHI::RHIGraphicContex* context);

    VulkanGraphicContext(VulkanDevice* device, VulkanQueue* queue);
    ~VulkanGraphicContext() override;

    VulkanLooseUniformDataUploader* GetLooseUniformDataUploader() const { return LooseUniformDataUploader; }

    void SetBatchedShaderParameters(RHIGraphicShader* shader, const RHIBatchedShaderParameters& parameter) override;
    void SetStreamSource(uint32_t streamIndex, RHIBufferSP VertexBuffer, uint32_t Offset) override;
    void SetGraphicPipelineState(RHIGraphicsPipelineState* pipelineState) override;
    void SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth) override;
    void SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h) override;
    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
    void DrawIndexed(RHIBuffer* indexBuffer, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;
    void BeginRenderPass(const RHIRenderPassInfo& renderPassInfo) override;
    void EndRenderPass() override;
    void SetRayTracingPipelineState(RHIRayTracingPipelineState* pipelineState) override;
    void SetShaderTable() override;
    void TraceRays(uint32_t width, uint32_t height, uint32_t depth = 1) override;

protected:
private:
    friend class VulkanCommandContext;
    VulkanPendingGfxState* PendingGfx = nullptr;
    VulkanLooseUniformDataUploader* LooseUniformDataUploader = nullptr;
};

struct VulkanCommandUpdateTexture : public RHICommandBase
{
    VulkanTexture* texture;
    std::shared_ptr<VulkanStagingBuffer> staging;
    VkBufferImageCopy copyRegion{};

    VulkanCommandUpdateTexture(VulkanTexture* texture, std::shared_ptr<VulkanStagingBuffer> buffer, VkBufferImageCopy region)
        : texture(texture), staging(buffer), copyRegion(region) {}

    void Execute(RHICommandListBase& cmdList) override {
        auto* vulkanContex = dynamic_cast<VulkanCommandContext*>(cmdList.GetCommandContex());
        VulkanCommandBuffer* cmdBuffer = vulkanContex->GetCommandBufferManager()->GetActiveCommandBuffer();

        VKFunc::CmdCopyBufferToImage(
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

    void Execute(RHICommandListBase& cmdList) override
    {
        auto* vulkanContext = dynamic_cast<VulkanCommandContext*>(cmdList.GetCommandContex());

        VulkanCommandBuffer* cmdBuffer =
            vulkanContext->GetCommandBufferManager()->GetActiveCommandBuffer();

        VKFunc::CmdCopyBuffer(
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
