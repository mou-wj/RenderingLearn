#include "RHIPipelineStateCache.h"
#include <functional>
#include "RHIApi.h"

namespace RHI {

std::unordered_map<size_t, RHIGraphicsPipelineStateSP> RHIPipelineStateCache::s_graphicsCache;
std::unordered_map<size_t, RHIComputePipelineStateSP> RHIPipelineStateCache::s_computeCache;
std::unordered_map<size_t, RHIVertexDescStateSP> RHIPipelineStateCache::s_vertexDescCache;
std::unordered_map<size_t, RHIRasterizerStateSP> RHIPipelineStateCache::s_rasterizerCache;
std::unordered_map<size_t, RHIColorBlendStateSP> RHIPipelineStateCache::s_colorBlendCache;
std::unordered_map<size_t, RHIDepthStencilStateSP> RHIPipelineStateCache::s_depthStencilCache;

std::mutex RHIPipelineStateCache::s_graphicsMutex;
std::mutex RHIPipelineStateCache::s_computeMutex;
std::mutex RHIPipelineStateCache::s_vertexDescMutex;
std::mutex RHIPipelineStateCache::s_rasterizerMutex;
std::mutex RHIPipelineStateCache::s_colorBlendMutex;
std::mutex RHIPipelineStateCache::s_depthStencilMutex;

RHIGraphicsPipelineState* RHIPipelineStateCache::GetOrCreateGraphicsPipelineState(const RHIGraphicsPipelineStateDesc& desc) {
	std::lock_guard<std::mutex> lock(RHIPipelineStateCache::s_graphicsMutex);
	size_t hash = RHIPipelineStateCache::HashGraphicsPipelineDesc(desc);

	auto it = RHIPipelineStateCache::s_graphicsCache.find(hash);
	if (it != RHIPipelineStateCache::s_graphicsCache.end()) {
		return it->second.get();
	}
	auto state = GRHIApi->CreateGraphicsPipelineState(desc);
	RHIPipelineStateCache::s_graphicsCache[hash] = state;
	return state.get();
}


RHIComputePipelineState* RHIPipelineStateCache::GetOrCreateComputePipelineState(const RHIComputePipelineStateDesc& desc) {
	std::lock_guard<std::mutex> lock(s_computeMutex);
	size_t hash = HashComputePipelineDesc(desc);

	auto it = s_computeCache.find(hash);
	if (it != s_computeCache.end()) {
		return it->second.get();
	}
	auto state = GRHIApi->CreateComputePipelineState(desc);
	s_computeCache[hash] = state;
	return state.get();

}

RHIVertexDescState* RHIPipelineStateCache::GetOrCreateVertexDescState(const RHIVertexDescStateDesc& desc) {
    std::lock_guard<std::mutex> lock(s_vertexDescMutex);
    size_t hash = HashVertexDescState(desc);
    
    auto it = s_vertexDescCache.find(hash);
    if (it != s_vertexDescCache.end()) {
        return it->second.get();
    }

    auto state = GRHIApi->CreateVertexDescState(desc);
    s_vertexDescCache[hash] = state;
    return state.get();
}

RHIRasterizerState* RHIPipelineStateCache::GetOrCreateRasterizerState(const RHIRasterizerStateDesc& desc) {
    std::lock_guard<std::mutex> lock(s_rasterizerMutex);
    size_t hash = HashRasterizerState(desc);
    
    auto it = s_rasterizerCache.find(hash);
    if (it != s_rasterizerCache.end()) {
        return it->second.get();
    }

    auto state = GRHIApi->CreateRasterizerState(desc);
    s_rasterizerCache[hash] = state;
    return state.get();
}

RHIColorBlendState* RHIPipelineStateCache::GetOrCreateColorBlendState(const RHIColorBlendStateDesc& desc) {
    std::lock_guard<std::mutex> lock(s_colorBlendMutex);
    size_t hash = HashColorBlendState(desc);
    
    auto it = s_colorBlendCache.find(hash);
    if (it != s_colorBlendCache.end()) {
        return it->second.get();
    }

    auto state = GRHIApi->CreateColorBlendState(desc);
    s_colorBlendCache[hash] = state;
    return state.get();
}

RHIDepthStencilState* RHIPipelineStateCache::GetOrCreateDepthStencilState(const RHIDepthStencilStateDesc& desc) {
    std::lock_guard<std::mutex> lock(s_depthStencilMutex);
    size_t hash = HashDepthStencilState(desc);
    
    auto it = s_depthStencilCache.find(hash);
    if (it != s_depthStencilCache.end()) {
        return it->second.get();
    }

    auto state = GRHIApi->CreateDepthStencilState(desc);
    s_depthStencilCache[hash] = state;
    return state.get();
}

void RHIPipelineStateCache::ClearAll() {
    {
        std::lock_guard<std::mutex> lock(s_graphicsMutex);
        s_graphicsCache.clear();
    }
    {
        std::lock_guard<std::mutex> lock(s_computeMutex);
        s_computeCache.clear();
    }
    {
        std::lock_guard<std::mutex> lock(s_vertexDescMutex);
        s_vertexDescCache.clear();
    }
    {
        std::lock_guard<std::mutex> lock(s_rasterizerMutex);
        s_rasterizerCache.clear();
    }
    {
        std::lock_guard<std::mutex> lock(s_colorBlendMutex);
        s_colorBlendCache.clear();
    }
    {
        std::lock_guard<std::mutex> lock(s_depthStencilMutex);
        s_depthStencilCache.clear();
    }
}

size_t RHIPipelineStateCache::HashGraphicsPipelineDesc(const RHIGraphicsPipelineStateDesc& desc) {
	size_t hash = 0;
	hash ^= HashVertexDescState(desc.vertexDescState->GetDesc());
	hash ^= HashRasterizerState(desc.rasterizerState->GetDesc());
	hash ^= HashColorBlendState(desc.colorBlendState->GetDesc());
	hash ^= HashDepthStencilState(desc.depthStencilState->GetDesc());
	return hash;

}
size_t RHIPipelineStateCache::HashComputePipelineDesc(const RHIComputePipelineStateDesc& desc) {
	size_t hash = 0;
	hash ^= std::hash<uint64_t>()(desc.computeShader ? (uint64_t)desc.computeShader : 0);
	return hash;

}


size_t RHIPipelineStateCache::HashVertexDescState(const RHIVertexDescStateDesc& desc) {
    size_t hash = 0;
    for (const auto& binding : desc.bindings) {
        hash ^= std::hash<uint32_t>()(binding.binding);
        hash ^= std::hash<uint32_t>()(binding.stride);
        hash ^= std::hash<int>()(static_cast<int>(binding.inputRate));
    }
    for (const auto& attr : desc.attributes) {
        hash ^= std::hash<uint32_t>()(attr.location);
        hash ^= std::hash<uint32_t>()(attr.offset);
        hash ^= std::hash<int>()(static_cast<int>(attr.format));
    }
    return hash;
}

size_t RHIPipelineStateCache::HashRasterizerState(const RHIRasterizerStateDesc& desc) {
    size_t hash = 0;
    hash ^= std::hash<bool>()(desc.depthClampEnable);
    hash ^= std::hash<bool>()(desc.rasterizerDiscardEnable);
    hash ^= std::hash<int>()(static_cast<int>(desc.polygonMode));
    hash ^= std::hash<int>()(static_cast<int>(desc.cullMode));
    hash ^= std::hash<int>()(static_cast<int>(desc.frontFace));
    hash ^= std::hash<bool>()(desc.depthBiasEnable);
    hash ^= std::hash<float>()(desc.depthBiasConstantFactor);
    hash ^= std::hash<float>()(desc.depthBiasClamp);
    hash ^= std::hash<float>()(desc.depthBiasSlopeFactor);
    hash ^= std::hash<float>()(desc.lineWidth);
    return hash;
}

size_t RHIPipelineStateCache::HashColorBlendState(const RHIColorBlendStateDesc& desc) {
    size_t hash = std::hash<bool>()(desc.logicOpEnable);
    for (const auto& attachment : desc.attachments) {
        hash ^= std::hash<bool>()(attachment.blendEnable);
        hash ^= std::hash<uint32_t>()(attachment.colorWriteMask);
    }
    for (int i = 0; i < 4; ++i) {
        hash ^= std::hash<float>()(desc.blendConstants[i]);
    }
    return hash;
}

size_t RHIPipelineStateCache::HashDepthStencilState(const RHIDepthStencilStateDesc& desc) {
    size_t hash = 0;
    hash ^= std::hash<bool>()(desc.depthTestEnable);
    hash ^= std::hash<bool>()(desc.depthWriteEnable);
    hash ^= std::hash<int>()(static_cast<int>(desc.depthCompareOp));
    hash ^= std::hash<bool>()(desc.depthBoundsTestEnable);
    hash ^= std::hash<bool>()(desc.stencilTestEnable);
    hash ^= std::hash<uint32_t>()(desc.stencilReadMask);
    hash ^= std::hash<uint32_t>()(desc.stencilWriteMask);
    return hash;
}

} // namespace WR::RHI
