#pragma once

#include "RHICommandContex.h"

#if defined(_WIN32)
#include <d3d12.h>
#include <wrl/client.h>
#endif

namespace RHIDirectX12
{
    class DirectX12Queue;

    class DirectX12CommandContext : public virtual RHI::RHIContextBase
    {
    public:
        explicit DirectX12CommandContext(DirectX12Queue* inQueue = nullptr);
        ~DirectX12CommandContext() override = default;

        void Begin() override;
        void End() override;
        void BeginTransitions(std::vector<const RHI::RHITransition*> transitions) override;
        void EndTransitions(std::vector<const RHI::RHITransition*> transitions) override;
        void CopyTexture(RHI::RHITexture* src, RHI::RHITexture* dst, const RHI::RHICopyTextureDesc& copyDesc) override;
        void BlitTexture(RHI::RHITexture* src, RHI::RHITexture* dst, const RHI::RHIBlitTextureDesc& blitDesc) override;

        DirectX12Queue* GetQueue() const { return Queue; }

    #if defined(_WIN32)
        ID3D12GraphicsCommandList* GetNativeCommandList() const { return CommandList.Get(); }
    #endif

    protected:
        DirectX12Queue* Queue = nullptr;

    #if defined(_WIN32)
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;
        bool bRecording = false;
    #endif
    };

    class DirectX12ComputeContext : public DirectX12CommandContext, public RHI::RHIComputeContex
    {
    public:
        explicit DirectX12ComputeContext(DirectX12Queue* inQueue = nullptr);
        ~DirectX12ComputeContext() override = default;

        void SetComputePipelineState(RHI::RHIComputePipelineState* pipelineState) override;
        void SetBatchedShaderParameters(RHI::RHIComputeShader* shader, const RHI::RHIBatchedShaderParameters& parameter) override;
        void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
    };

    class DirectX12GraphicContext : public DirectX12CommandContext, public RHI::RHIGraphicContex
    {
    public:
        explicit DirectX12GraphicContext(DirectX12Queue* inQueue = nullptr);
        ~DirectX12GraphicContext() override = default;

        void SetBatchedShaderParameters(RHI::RHIGraphicShader* shader, const RHI::RHIBatchedShaderParameters& parameter) override;
        void SetStreamSource(uint32_t streamIndex, RHI::RHIBuffer* vertexBuffer, uint32_t offset) override;
        void SetGraphicPipelineState(RHI::RHIGraphicsPipelineState* pipelineState) override;
        void SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth) override;
        void SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h) override;
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
        void DrawIndexed(RHI::RHIBuffer* indexBuffer, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;
        void BeginRenderPass(const RHI::RHIRenderPassInfo& renderPassInfo) override;
        void EndRenderPass() override;

        void SetBatchedShaderParameters(RHI::RHIRayTracingShader* shader, const RHI::RHIBatchedShaderParameters& parameter) override;
        void SetRayTracingPipelineState(RHI::RHIRayTracingPipelineState* pipelineState) override;
        void BuildAccelerationStructure(RHI::RHIRayTracingAccelerationStructure* accelerationStructure) override;
        void UpdateAccelerationStructure(RHI::RHIRayTracingAccelerationStructure* accelerationStructure) override;
        void TraceRays(uint32_t width, uint32_t height, uint32_t depth = 1) override;
    };
}
