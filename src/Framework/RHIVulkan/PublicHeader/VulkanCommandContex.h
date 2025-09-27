// VulkanContexts.h
#pragma once

#include "RHICommandContex.h"
#include "VulkanDevice.h"
#include "VulkanPipeline.h"
#include "VulkanResource.h"
#include "RHIRenderTargetInfo.h"



using namespace RHI;

namespace RHIVulkan {
class VulkanDescriptorSet;
class VulkanCommandBufferManager;



// -------------------------------------------------------------------------------------------------
// Vulkan Graphics Context
// -------------------------------------------------------------------------------------------------
class VulkanCommandContext : public RHICommandContex
{
public:
    VulkanCommandContext(VulkanDevice* device, VulkanQueue* queue);
    virtual ~VulkanCommandContext();

    // Compute接口
    void SetComputePipelineState(const RHIComputePipelineStateSP& pipelineState) override;
    void SetShaderParameter(RHIShader* shader, const RHIShaderParameterSP& parameter) override;
    virtual void SetShaderBatchedShaderParameter(RHIShader* shader, const RHIBatchedShaderParameter& parameter) override;
    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

    virtual void SetRenderTarget(const RHIRenderTargetsInfo& renderTargets) override {}

    virtual void SetStreamSource(uint32_t streamIndex, RHIBufferSP VertexBuffer, uint32_t Offset) override;

    // Graphics接口
    void SetGraphicPipelineState(const RHIGraphicsPipelineStateSP& pipelineState) override;
    virtual void ViewportPresent(const RHIVIewportSP& viewport, const RHITextureSP& presentRenderTarget) override;
    virtual void SetViewPortRect(const RHIIntRect& viewport) override;
    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
    void DrawIndexed(RHIBuffer* indexBuffer, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;

    // RayTracing接口
    void SetRayTracingPipelineState(const RHIRayTracingPipelineStateSP& pipelineState) override;
    void SetShaderTable() override;
    void TraceRays(uint32_t width, uint32_t height, uint32_t depth = 1) override;

    VulkanQueue* GetQueue() const { return queue; }

private:
    VulkanDevice* device;
    VulkanQueue* queue;
    VulkanCommandBufferManager* commandBufferManager;

    struct GraphicContexState{
        VulkanGraphicsPipelineSP PipelineState;
        std::vector<RHIBufferSP> VertexBuffers;
        RHIBufferSP IndexBuffer;

    } GraphicState;             

    struct ComputeContexState{
        VulkanComputePipelineSP PipelineState;
        uint32_t ShaderParameterCount = 0;
    } ComputeState;

    struct RayTracingContexState{   
        VulkanRayTracingPipelineSP PipelineState;
    } RayTracingState;



};

} // namespace WR::RHIVulkan