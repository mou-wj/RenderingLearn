#pragma once

#include "RHIResource.h"
#include "RHIShaderParameter.h"
#include "RHICommandList.h"

namespace RHI
{

    class RHI_API RHIPlatformCommandList {
    public:
        virtual ~RHIPlatformCommandList() = default;
    };

    // ͳһ�� RHIContext������ Compute / Graphics / RayTracing �ӿ�
    class RHI_API RHICommandContex
    {
    public:
        RHICommandContex();
        virtual ~RHICommandContex() = default;
        // ========================
        // Compute �ӿ�
        // ========================
        virtual void SetComputePipelineState(RHIComputePipelineState* pipelineState) = 0;
        /** Set the shader resource view of a surface. */
        virtual void RHISetShaderTexture(RHIShader* Shader, uint32_t TextureIndex, RHITexture* Texture) = 0;

        virtual void RHISetShaderSampler(RHIShader* Shader, uint32_t SamplerIndex, RHISampler* NewState) = 0;

        virtual void RHISetUAVParameter(RHIShader* Shader, uint32_t UAVIndex, RHIUnorderedAccessView* UAV) = 0;


        virtual void RHISetShaderResourceViewParameter(RHIShader* Shader, uint32_t SamplerIndex, RHIShaderResourceView* SRV) = 0;

        virtual void RHISetShaderUniformBuffer(RHIShader* Shader, uint32_t BufferIndex, RHIUniformBuffer* Buffer) = 0;
        virtual void RHISetShaderParameters(RHIShader* Shader, const std::vector<uint8_t>& InParametersData, const std::vector<RHIShaderUniformParameter>& InParameters, const std::vector<RHIShaderResourceParameter>& InResourceParameters) = 0;

        virtual void RHISetShaderParameter(RHIShader* Shader, uint32_t BufferIndex, uint32_t BaseIndex, uint32_t NumBytes, const void* NewValue) = 0;

        virtual void SetBatchedShaderParameters(RHIShader* shader, const RHIBatchedShaderParameters& parameter) = 0;


        virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
        virtual void CopyTexture(RHITexture* src, RHITexture* dst, const RHICopyTextureDesc& copyDesc) = 0;

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

        // ========================
        // RayTracing �ӿ�
        // ========================
        virtual void SetRayTracingPipelineState(RHIRayTracingPipelineState* pipelineState) = 0;
        virtual void SetShaderTable() = 0;
        virtual void TraceRays(uint32_t width, uint32_t height, uint32_t depth = 1) = 0;

        // ========================
        // Command List
        // ========================
        RHICommandList& GetCommandList() { return CommandList; }

        virtual void BeginDrawingViewport(RHIViewport* Viewport, RHITexture* RenderTargetRHI) = 0;
        virtual void EndDrawingViewport(RHIViewport* Viewport, bool bPresent) = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void BeginRenderPass(const RHIRenderPassInfo& renderPassInfo) = 0;
        virtual void EndRenderPass() = 0;

    protected:
        RHICommandList CommandList; // ָ��ǰ�����б�
        ERHIPipelineType PipelineType;
    };

    using RHICommandContexSP = std::shared_ptr<RHICommandContex>;

} // namespace WR::RHI