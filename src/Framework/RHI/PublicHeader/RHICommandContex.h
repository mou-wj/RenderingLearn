#pragma once

#include "RHIResource.h"
#include "RHIShaderParameter.h"
#include "RHIRenderTargetInfo.h"
#include "RHICommandList.h"

namespace RHI
{

    // 统一的 RHIContext，整合 Compute / Graphics / RayTracing 接口
    class RHICommandContex
    {
    public:
        RHICommandContex();
        virtual ~RHICommandContex() = default;

        // ========================
        // Compute 接口
        // ========================
        virtual void SetComputePipelineState(const RHIComputePipelineStateSP& pipelineState) = 0;
        virtual void SetShaderParameter(RHIShader* shader, const RHIShaderParameterSP& parameter) = 0;
        virtual void SetShaderBatchedShaderParameter(RHIShader* shader, const RHIBatchedShaderParameter& parameter) = 0;
        virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

        // ========================
        // Graphics 接口
        // ========================
        virtual void SetRenderTarget(const RHIRenderTargetsInfo& renderTargets) = 0;
        virtual void SetStreamSource(uint32_t streamIndex, RHIBuffer* VertexBuffer, uint32_t Offset) = 0;
        virtual void SetGraphicPipelineState(const RHIGraphicsPipelineStateSP& pipelineState) = 0;
        virtual void ViewportPresent(const RHIVIewportSP& viewport, const RHITextureSP& presentRenderTarget) = 0;
        virtual void SetViewPortRect(const RHIIntRect& viewport) = 0;
        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1,
            uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
        virtual void DrawIndexed(RHIBuffer* indexBuffer, uint32_t indexCount,
            uint32_t instanceCount = 1, uint32_t firstIndex = 0,
            int32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;

        // ========================
        // RayTracing 接口
        // ========================
        virtual void SetRayTracingPipelineState(const RHIRayTracingPipelineStateSP& pipelineState) = 0;
        virtual void SetShaderTable() = 0;
        virtual void TraceRays(uint32_t width, uint32_t height, uint32_t depth = 1) = 0;

        // ========================
        // Command List
        // ========================
        RHICommandListSP GetCommandList() const { return CommandList; }

    protected:
        RHICommandListSP CommandList; // 指向当前命令列表
        ERHIPipelineType PipelineType;
    };

    using RHICommandContexSP = std::shared_ptr<RHICommandContex>;

} // namespace WR::RHI