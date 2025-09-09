#pragma once
#include <vector>
#include <memory>
#include <list>
#include "RHIRenderTargetInfo.h"

namespace RHI {

    class RHICommandContex; // 统一后的 Contex
    class RHICommandList;

    // -----------------------------
    // 命令基类
    // -----------------------------
    struct RHICommandBase
    {
        virtual ~RHICommandBase() = default;
        virtual void Execute(RHICommandList& cmdList) = 0;
    };

    // -----------------------------
    // 具体命令
    // -----------------------------
    struct RHICommandDispatch : public RHICommandBase
    {
        uint32_t X, Y, Z;
        RHICommandDispatch(uint32_t x, uint32_t y, uint32_t z);
        void Execute(RHICommandList& cmdList) override;
    };

    struct RHICommandDraw : public RHICommandBase
    {
        uint32_t VertexCount, InstanceCount, FirstVertex, FirstInstance;
        RHICommandDraw(uint32_t v, uint32_t i, uint32_t fv, uint32_t fi);
        void Execute(RHICommandList& cmdList) override;
    };

    struct RHICommandTraceRays : public RHICommandBase
    {
        uint32_t Width, Height, Depth;
        RHICommandTraceRays(uint32_t w, uint32_t h, uint32_t d);
        void Execute(RHICommandList& cmdList) override;
    };

    // -----------------------------
    // 统一的命令列表
    // -----------------------------
    class RHICommandList
    {
    public:
        explicit RHICommandList(RHICommandContex* context);
        virtual ~RHICommandList();

        // 添加命令
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

        // 执行所有命令
        virtual void ExecuteAll();
        void Clear();

        void SetImmediate(bool bImmediate);
        bool IsImmediate() const;

        // -----------------
        // Compute 接口
        // -----------------
        void Dispatch(uint32_t x, uint32_t y, uint32_t z);

        // -----------------
        // Graphics 接口
        // -----------------
        void SetRenderTarget(const RHIRenderTargetsInfo& renderTargets);
        void SetStreamSource(uint32_t streamIndex, RHIBuffer* VertexBuffer, uint32_t Offset);
        void SetGraphicPipelineState(const RHIGraphicsPipelineStateSP& pipelineState);
        void ViewportPresent(const RHIVIewportSP& viewport, const RHITextureSP& presentRenderTarget);
        void SetViewPortRect(const RHIIntRect& viewport);
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1,
            uint32_t firstVertex = 0, uint32_t firstInstance = 0);

        // -----------------
        // RayTracing 接口
        // -----------------
        void TraceRays(uint32_t width, uint32_t height, uint32_t depth = 1);

        // -----------------
        // 获取绑定的 Context
        // -----------------
        RHICommandContex* GetCommandContex() const;

    protected:
        bool immediate = false;
        std::list<std::shared_ptr<RHICommandBase>> commands;
        RHICommandContex* CommandContex = nullptr;

        friend struct RHICommandDispatch;
        friend struct RHICommandDraw;
        friend struct RHICommandTraceRays;
    };

    // -----------------------------
    // 类型别名
    // -----------------------------
    using RHICommandListSP = std::shared_ptr<RHICommandList>;

} // namespace WR::RHI