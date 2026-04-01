#pragma once

#include "RHIResource.h"
#include "RHIShaderParameter.h"
#include "RHICommandList.h"
#include "RHITransition.h"

namespace RHI
{
    using RHICmdBuffer = uint64_t;

    // Base transfer context interface: copy operations, transitions, begin/end recording.
    // Does not contain data — implementors must provide GetCommandList().
    class RHI_API RHITransferContext
    {
    public:
        virtual ~RHITransferContext() = default;
        virtual RHICommandList& GetCommandList() = 0;
        virtual void Begin() = 0;
        virtual RHICmdBuffer End() = 0;
        virtual void CopyTexture(RHITexture* src, RHITexture* dst, const RHICopyTextureDesc& copyDesc) = 0;
        virtual void BeginTransitions(std::vector<const RHITransition*> Transitions) = 0;
        virtual void EndTransitions(std::vector<const RHITransition*> Transitions) = 0;
    };

    // Compute context: inherits the transfer interface, owns CommandList/Pipeline data,
    // and adds compute dispatch capability.
    class RHI_API RHIComputeContext : public RHITransferContext
    {
    public:
        RHIComputeContext();
        virtual ~RHIComputeContext() = default;
        // ========================
        // Compute �ӿ�
        // ========================
        virtual void SetComputePipelineState(RHIComputePipelineState* pipelineState) = 0;
        virtual void SetBatchedShaderParameters(RHIComputeShader* shader, const RHIBatchedShaderParameters& parameter) = 0;


        virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
        // ========================
        // Command List
        // ========================
        RHICommandList& GetCommandList() override { return CommandList; }

        virtual void Begin() = 0;
        virtual RHICmdBuffer End() = 0;

    protected:
        RHICommandList CommandList; // ָ��ǰ�����б�
        ERHIPipeline Pipeline;
    };

    // Compute context + graphics extensions.
    class RHI_API RHICommandContext : public RHIComputeContext
    {
    public:
        RHICommandContext() = default;
        virtual ~RHICommandContext() = default;

        using RHIComputeContext::SetBatchedShaderParameters;
        virtual void SetBatchedShaderParameters(RHIGraphicShader* shader, const RHIBatchedShaderParameters& parameter) = 0;

        // ========================
        // Graphics �ӿ�
        // ========================
        virtual void SetStreamSource(uint32_t streamIndex, RHIBufferSP VertexBuffer, uint32_t Offset) = 0;
        virtual void SetGraphicPipelineState(RHIGraphicsPipelineState* pipelineState) = 0;
        virtual void SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth) = 0;
        virtual void SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h) = 0;
        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1,
            uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
        virtual void DrawIndexed(RHIBuffer* indexBuffer, uint32_t indexCount,
            uint32_t instanceCount = 1, uint32_t firstIndex = 0,
            int32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;

        virtual void BeginRenderPass(const RHIRenderPassInfo& renderPassInfo) = 0;
        virtual void EndRenderPass() = 0;

        // ========================
        // RayTracing �ӿ�
        // ========================
        virtual void SetRayTracingPipelineState(RHIRayTracingPipelineState* pipelineState) = 0;
        virtual void SetShaderTable() = 0;
        virtual void TraceRays(uint32_t width, uint32_t height, uint32_t depth = 1) = 0;
    };

    // Backward-compatible alias for the original type name.
    using RHICommandContex = RHICommandContext;

    using RHICommandContexSP = std::shared_ptr<RHICommandContex>;
    using RHITransferContextSP = std::shared_ptr<RHITransferContext>;
    using RHIComputeContextSP = std::shared_ptr<RHIComputeContext>;
    using RHICommandContextSP = std::shared_ptr<RHICommandContext>;

enum class EQueueType{
    Graphics,
    Compute,
    Transfer
};

class RHISyncPoint {
public:
    virtual ~RHISyncPoint() = default;

    EQueueType Type;   // 产生该信号的队列类型 (Graphics/Compute/Transfer)
    uint64_t Value;    // 该队列时间轴上的单调递增数值

    // 核心接口
    virtual bool IsReached() const = 0; // 非阻塞检查是否完成
    virtual void Wait() const = 0;      // CPU 阻塞等待
};

class RHISwapchain {
public:
    virtual ~RHISwapchain() = default;

    struct RHISwapchainSlot {
    RHITexture* Texture;    // 物理资源
    RHISyncPoint* ReadySync; // 准入证
    };

    // 关键：获取当前帧可以写入的纹理（不透明的 RHITexture）
    virtual RHISwapchainSlot AcquireNextSlot() = 0;
    
    // 窗口缩放处理
    virtual void Resize(uint32_t Width, uint32_t Height) = 0;

protected:

};

// Present capability, separate from queue submission.
// Implement on objects that own a present-capable surface (e.g. a graphics queue).
class RHI_API RHIPresentExecutor
{
public:
    virtual ~RHIPresentExecutor() = default;
    virtual void Present(RHISwapchain* Swapchain) = 0;
};

class RHIQueue {
public:
    virtual ~RHIQueue() = default;

    virtual EQueueType GetType() const { return Type; };

    virtual RHIComputeContext* AcquireCommandContext() = 0;
    virtual RHIComputeContext* ReleaseCommandContext(RHIComputeContext* Context) = 0;

    // 提交指令包，并返回一个新的同步点
    virtual RHISyncPoint* Submit(RHICmdBuffer CmdBuffer) = 0;

    // 批量提交并处理跨队列等待
    virtual RHISyncPoint* Submit(const std::vector<RHICmdBuffer>& Cmds, 
                               const std::vector<RHISyncPoint*>& WaitPoints) = 0;

    // 强制刷新硬件队列
    virtual void WaitIdle() = 0;
private:
    EQueueType Type;
};

using RHISwapchainSP = std::shared_ptr<RHISwapchain>;


} // namespace WR::RHI