#include "ShaderLibrary.h"
#include <cassert>

namespace RenderCore {
RHI::ERHIShaderPlatform CurrentShaderPlatform;
std::unordered_map<RHI::ERHIShaderPlatform, RHI::RHIShaderLibrarySP> ShaderLibrary::PlatformLibraryMap;
std::mutex ShaderLibrary::PlatformLibraryMutex;

// 获取或创建某个平台的RHIShaderLibrary
RHI::RHIShaderLibrarySP ShaderLibrary::GetOrCreateLibrary(RHI::ERHIShaderPlatform platform) {
    std::lock_guard<std::mutex> lock(PlatformLibraryMutex);
    auto it = PlatformLibraryMap.find(platform);
    if (it != PlatformLibraryMap.end()) {
        return it->second;
    }
    // 这里假设有全局RHI接口可创建对应平台的ShaderLibrary
    // 例如: auto lib = GetGlobalRHIApi(platform)->CreateShaderLibrary();
    // 这里只能伪代码，实际需你实现RHI接口
    RHI::RHIShaderLibrarySP lib = nullptr;
    // TODO: 替换为实际RHI接口
    assert(lib && "RHIShaderLibrary creation failed!");
    PlatformLibraryMap[platform] = lib;
    return lib;
}

RHI::RHIShaderSP ShaderLibrary::CreateShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKeyHash& key) {
    auto lib = GetOrCreateLibrary(platform);
    if (!lib) return nullptr;
    return lib->CreateShader(key);
}

RHI::RHIVertexShaderSP ShaderLibrary::CreateVertexShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKeyHash& key) {
    return std::static_pointer_cast<RHI::RHIVertexShader>(CreateShader(platform, key));
}

RHI::RHIFragmentShaderSP ShaderLibrary::CreateFragmentShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKeyHash& key) {
    return std::static_pointer_cast<RHI::RHIFragmentShader>(CreateShader(platform, key));
}

RHI::RHIComputeShaderSP ShaderLibrary::CreateComputeShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKeyHash& key) {
    return std::static_pointer_cast<RHI::RHIComputeShader>(CreateShader(platform, key));
}

RHI::RHIGeometryShaderSP ShaderLibrary::CreateGeometryShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKeyHash& key) {
    return std::static_pointer_cast<RHI::RHIGeometryShader>(CreateShader(platform, key));
}

RHI::RHITessControlShaderSP ShaderLibrary::CreateTessControlShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKeyHash& key) {
    return std::static_pointer_cast<RHI::RHITessControlShader>(CreateShader(platform, key));
}

RHI::RHITessEvalShaderSP ShaderLibrary::CreateTessEvalShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKeyHash& key) {
    return std::static_pointer_cast<RHI::RHITessEvalShader>(CreateShader(platform, key));
}

} // namespace RenderCore