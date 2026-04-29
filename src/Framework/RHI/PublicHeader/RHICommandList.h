#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <list>
#include "RHIShaderParameter.h"
#include "RHICommandContex.h"

namespace RHI {

    using RHICmdBuffer = uint64_t;

    class RHICommandListBase;
    class RHITransferCommandList;
    class RHIComputeCommandList;
    class RHIGraphicCommandList;
    class RHITransition;

    // -----------------------------
    // �������
    // -----------------------------
    struct RHI_API RHICommandBase
    {
        virtual ~RHICommandBase() = default;
        virtual void Execute(RHICommandListBase& cmdList) = 0;
    };

    // -----------------------------
    // ��������
    // -----------------------------
    struct RHI_API RHICommandDispatch : public RHICommandBase
    {
        uint32_t X, Y, Z;
        RHICommandDispatch(uint32_t x, uint32_t y, uint32_t z);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandDraw : public RHICommandBase
    {
        uint32_t VertexCount, InstanceCount, FirstVertex, FirstInstance;
        RHICommandDraw(uint32_t v, uint32_t i, uint32_t fv, uint32_t fi);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandTraceRays : public RHICommandBase
    {
        uint32_t Width, Height, Depth;
        RHICommandTraceRays(uint32_t w, uint32_t h, uint32_t d);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandCopyTexture : public RHICommandBase
    {
        RHITexture* Src = nullptr;
        RHITexture* Dst = nullptr;
        RHICopyTextureDesc CopyDesc{};
        RHICommandCopyTexture(RHITexture* src, RHITexture* dst, const RHICopyTextureDesc& copyDesc);
        void Execute(RHICommandListBase& cmdList) override;
    };
    struct RHI_API RHICommandBlitTexture : public RHICommandBase
    {
        RHITexture* Src = nullptr;
        RHITexture* Dst = nullptr;
        RHIBlitTextureDesc BlitDesc{};
        RHICommandBlitTexture(RHITexture* src, RHITexture* dst, const RHIBlitTextureDesc& blitDesc);
        void Execute(RHICommandListBase& cmdList) override;
    };
    struct RHI_API RHICommandUpdateTexture : public RHICommandBase
    {
        RHITexture* texture;
        const void* data;
        RHITextureRegion region;
        RHICommandUpdateTexture(RHITexture* src, const void* data, const RHITextureRegion& region);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandUpdateBuffer : public RHICommandBase
    {
        RHIBuffer* buffer;
        const void* data;
        RHIBufferRegion region;
        RHICommandUpdateBuffer(RHIBuffer* buffer, const void* data, const RHIBufferRegion& region);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandSetComputePipelineState : public RHICommandBase
    {
        RHIComputePipelineState* PipelineState = nullptr;
        explicit RHICommandSetComputePipelineState(RHIComputePipelineState* pipelineState);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandSetComputeShaderParameters : public RHICommandBase
    {
        RHIComputeShader* Shader = nullptr;
        RHIBatchedShaderParameters Parameters{};
        RHICommandSetComputeShaderParameters(RHIComputeShader* shader, const RHIBatchedShaderParameters& parameters);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandSetGraphicShaderParameters : public RHICommandBase
    {
        RHIGraphicShader* Shader = nullptr;
        RHIBatchedShaderParameters Parameters{};
        RHICommandSetGraphicShaderParameters(RHIGraphicShader* shader, const RHIBatchedShaderParameters& parameters);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandSetStreamSource : public RHICommandBase
    {
        uint32_t StreamIndex = 0;
        RHIBufferSP VertexBuffer;
        uint32_t Offset = 0;
        RHICommandSetStreamSource(uint32_t streamIndex, RHIBufferSP vertexBuffer, uint32_t offset);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandSetGraphicPipelineState : public RHICommandBase
    {
        RHIGraphicsPipelineState* PipelineState = nullptr;
        explicit RHICommandSetGraphicPipelineState(RHIGraphicsPipelineState* pipelineState);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandSetViewport : public RHICommandBase
    {
        float X = 0.0f;
        float Y = 0.0f;
        float W = 0.0f;
        float H = 0.0f;
        float MinDepth = 0.0f;
        float MaxDepth = 1.0f;
        RHICommandSetViewport(float x, float y, float w, float h, float minDepth, float maxDepth);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandSetScissor : public RHICommandBase
    {
        int32_t X = 0;
        int32_t Y = 0;
        uint32_t W = 0;
        uint32_t H = 0;
        RHICommandSetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandBeginRenderPass : public RHICommandBase
    {
        RHIRenderPassInfo RenderPassInfo{};
        explicit RHICommandBeginRenderPass(const RHIRenderPassInfo& renderPassInfo);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandEndRenderPass : public RHICommandBase
    {
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandSetRayTracingPipelineState : public RHICommandBase
    {
        RHIRayTracingPipelineState* PipelineState = nullptr;
        explicit RHICommandSetRayTracingPipelineState(RHIRayTracingPipelineState* pipelineState);
        void Execute(RHICommandListBase& cmdList) override;
    };

    struct RHI_API RHICommandSetShaderTable : public RHICommandBase
    {
        void Execute(RHICommandListBase& cmdList) override;
    };



    // -----------------------------
    // ͳһ�������б�����
    // -----------------------------
    class RHI_API RHICommandListBase
    {
    public:
        explicit RHICommandListBase(RHIContextBase* context);
        virtual ~RHICommandListBase();

        // ��������
        template<typename T, typename... Args>
        void AddCommand(Args&&... args)
        {
            if (immediate)
            {
                T cmd(std::forward<Args>(args)...);
                cmd.Execute(*this);
            }
            else
            {
                commands.emplace_back(std::make_shared<T>(std::forward<Args>(args)...));
            }
        }

        // ִ����������
        virtual void ExecuteAll();
        void Clear();

        void SetImmediate(bool bImmediate);
        bool IsImmediate() const;

        RHIContextBase* GetContext() const;
        // Backward-compatible accessor name.
        RHIContextBase* GetCommandContex() const;

        // -----------------
        // ��ӿ�
        // -----------------
        void Begin();
        void End();

        void BeginTransitions(std::vector<const RHITransition*> Transitions);
        void EndTransitions(std::vector<const RHITransition*> Transitions);

        void Merge(const RHICommandListBase& other);

    protected:
        bool immediate = false;
        std::list<std::shared_ptr<RHICommandBase>> commands;
        RHIContextBase* Context = nullptr;

        friend struct RHICommandDispatch;
        friend struct RHICommandDraw;
        friend struct RHICommandTraceRays;
    };

    class RHI_API RHITransferCommandList : public RHICommandListBase
    {
    public:
        explicit RHITransferCommandList(RHITransferContext* context);
        RHITransferContext* GetTransferContext() const;
        void CopyTexture(RHITexture* src, RHITexture* dst, const RHICopyTextureDesc& copyDesc);
		void BlitTexture(RHITexture* src, RHITexture* dst, const RHIBlitTextureDesc& blitDesc);
        void UpdateTexture(RHITexture* texture, const void* data, const RHITextureRegion& size);
        void UpdateBuffer(RHIBuffer* buffer, const void* data, const RHIBufferRegion& region);
    };

    class RHI_API RHIComputeCommandList : public RHICommandListBase
    {
    public:
        explicit RHIComputeCommandList(RHIComputeContex* context);
        RHIComputeContex* GetComputeContext() const;
        void SetComputePipelineState(RHIComputePipelineState* pipelineState);
        void SetBatchedShaderParameters(RHIComputeShader* shader, const RHIBatchedShaderParameters& batchedShaderParameter);
        void Dispatch(uint32_t x, uint32_t y, uint32_t z);
    };

    class RHI_API RHIGraphicCommandList : public RHICommandListBase
    {
    public:
        explicit RHIGraphicCommandList(RHIGraphicContex* context);
        RHIGraphicContex* GetGraphicContext() const;

        void SetBatchedShaderParameters(RHIGraphicShader* shader, const RHIBatchedShaderParameters& batchedShaderParameter);
        void SetStreamSource(uint32_t streamIndex, RHIBufferSP VertexBuffer, uint32_t Offset);
        void SetGraphicPipelineState(RHIGraphicsPipelineState* pipelineState);
        void SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth);
        void SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h);
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1,
            uint32_t firstVertex = 0, uint32_t firstInstance = 0);
        void BeginRenderPass(const RHIRenderPassInfo& renderPassInfo);
        void EndRenderPass();
        void SetRayTracingPipelineState(RHIRayTracingPipelineState* pipelineState);
        void SetShaderTable();
        void TraceRays(uint32_t width, uint32_t height, uint32_t depth = 1);
    };


} // namespace WR::RHI