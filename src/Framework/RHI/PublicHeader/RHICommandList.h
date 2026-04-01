#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <list>
#include "RHIShaderParameter.h"

namespace RHI {

    using RHICmdBuffer = uint64_t;

    class RHIComputeContext;
    class RHICommandContext;
    class RHICommandList;
    class RHITransition;

    // -----------------------------
    // �������
    // -----------------------------
    struct RHI_API RHICommandBase
    {
        virtual ~RHICommandBase() = default;
        virtual void Execute(RHICommandList& cmdList) = 0;
    };

    // -----------------------------
    // ��������
    // -----------------------------
    struct RHI_API RHICommandDispatch : public RHICommandBase
    {
        uint32_t X, Y, Z;
        RHICommandDispatch(uint32_t x, uint32_t y, uint32_t z);
        void Execute(RHICommandList& cmdList) override;
    };

    struct RHI_API RHICommandDraw : public RHICommandBase
    {
        uint32_t VertexCount, InstanceCount, FirstVertex, FirstInstance;
        RHICommandDraw(uint32_t v, uint32_t i, uint32_t fv, uint32_t fi);
        void Execute(RHICommandList& cmdList) override;
    };

    struct RHI_API RHICommandTraceRays : public RHICommandBase
    {
        uint32_t Width, Height, Depth;
        RHICommandTraceRays(uint32_t w, uint32_t h, uint32_t d);
        void Execute(RHICommandList& cmdList) override;
    };



    // -----------------------------
    // ͳһ�������б�
    // -----------------------------
    class RHI_API RHICommandList
    {
    public:
        explicit RHICommandList(RHIComputeContext* context);
        virtual ~RHICommandList();

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

        void SetBatchedShaderParameters(RHIComputeShader* shader, const RHIBatchedShaderParameters& bacthedShaderParameter);
        void SetBatchedShaderParameters(RHIGraphicShader* shader, const RHIBatchedShaderParameters& bacthedShaderParameter);
        void UpdateTexture(RHITexture* texture, const void* data, const RHITextureRegion& size);
        //
        void UpdateBuffer(RHIBuffer* buffer, const void* data, const RHIBufferRegion& region);

        // -----------------
        // Compute �ӿ�
        // -----------------
        void Dispatch(uint32_t x, uint32_t y, uint32_t z);
        void CopyTexture(RHITexture* src, RHITexture* dst,const RHICopyTextureDesc& copyDesc);

        // -----------------
        // Graphics �ӿ�
        // -----------------
        void SetStreamSource(uint32_t streamIndex, RHIBufferSP VertexBuffer, uint32_t Offset);
        void SetGraphicPipelineState(RHIGraphicsPipelineState* pipelineState);

        
        void SetViewport(float x, float y, float w, float h, float minDepth,float maxDepth);
        void SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h);
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1,
            uint32_t firstVertex = 0, uint32_t firstInstance = 0);

        // -----------------
        // RayTracing �ӿ�
        // -----------------
        void TraceRays(uint32_t width, uint32_t height, uint32_t depth = 1);

        // -----------------
        // ��ȡ�󶨵� Context
        // -----------------
        RHIComputeContext* GetCommandContex() const;

        // -----------------
        // ��ӿ�
        // -----------------
        void Begin();
        RHICmdBuffer End();
        void BeginRenderPass(const RHIRenderPassInfo& renderPassInfo);
        void EndRenderPass();

        void BeginTransitions(std::vector<const RHITransition*> Transitions);
        void EndTransitions(std::vector<const RHITransition*> Transitions);

        void Merge(std::shared_ptr<RHICommandList> other);

    protected:
        bool immediate = false;
        std::list<std::shared_ptr<RHICommandBase>> commands;
        RHIComputeContext* CommandContex = nullptr;

        friend struct RHICommandDispatch;
        friend struct RHICommandDraw;
        friend struct RHICommandTraceRays;
    };



    // -----------------------------
    // ���ͱ���
    // -----------------------------
    using RHICommandListSP = std::shared_ptr<RHICommandList>;

} // namespace WR::RHI