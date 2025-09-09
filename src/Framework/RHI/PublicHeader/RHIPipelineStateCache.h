#pragma once

#include "RHIResource.h"
#include <unordered_map>
#include <mutex>
#include <map>

namespace RHI {
    
class RHIPipelineStateCache {
public:

    static RHIGraphicsPipelineStateSP GetGraphicsPipelineState(const RHIGraphicsPipelineStateDesc& desc);
    
  
    static RHIComputePipelineStateSP GetComputePipelineState(const RHIComputePipelineStateDesc& desc);


    static RHIVertexDescStateSP GetVertexDescState(const RHIVertexDescStateDesc& desc);


    static RHIRasterizerStateSP GetRasterizerState(const RHIRasterizerStateDesc& desc);


    static RHIColorBlendStateSP GetColorBlendState(const RHIColorBlendStateDesc& desc);


    static RHIDepthStencilStateSP GetDepthStencilState(const RHIDepthStencilStateDesc& desc);


    static void ClearAll();

private:

    static size_t HashGraphicsPipelineDesc(const RHIGraphicsPipelineStateDesc& desc);
    static size_t HashComputePipelineDesc(const RHIComputePipelineStateDesc& desc);
    static size_t HashVertexDescState(const RHIVertexDescStateDesc& desc);
    static size_t HashRasterizerState(const RHIRasterizerStateDesc& desc);
    static size_t HashColorBlendState(const RHIColorBlendStateDesc& desc);
    static size_t HashDepthStencilState(const RHIDepthStencilStateDesc& desc);

    static ::std::unordered_map<size_t, RHIGraphicsPipelineStateSP> s_graphicsCache;
    static std::unordered_map<size_t, RHIComputePipelineStateSP> s_computeCache;
    static std::unordered_map<size_t, RHIVertexDescStateSP> s_vertexDescCache;
    static std::unordered_map<size_t, RHIRasterizerStateSP> s_rasterizerCache;
    static std::unordered_map<size_t, RHIColorBlendStateSP> s_colorBlendCache;
    static std::unordered_map<size_t, RHIDepthStencilStateSP> s_depthStencilCache;
    
    static std::mutex s_graphicsMutex;
    static std::mutex s_computeMutex;
    static std::mutex s_vertexDescMutex;
    static std::mutex s_rasterizerMutex;
    static std::mutex s_colorBlendMutex;
    static std::mutex s_depthStencilMutex;

};

} // namespace RHI
