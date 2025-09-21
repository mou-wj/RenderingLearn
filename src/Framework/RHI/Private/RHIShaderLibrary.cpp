#include "RHIShaderLibrary.h"

namespace RHI {
    RHIShaderLibrary::RHIShaderLibrary(const std::string& name, ERHIShaderPlatform platform) : Name(name), Platform(platform)
    {
    }
    ShaderKey RHIShaderLibrary::AddShaderBinary(const ShaderBinary& binary) {
    ShaderKey key{ binary.Name, binary.Type };
    std::lock_guard<std::mutex> lock(ShaderCacheMutex);
    ShaderCache[key] = std::make_unique<ShaderBinary>(binary);
    return key;
}

const ShaderBinary* RHIShaderLibrary::FindShaderBinary(ShaderKey key) const {
    std::lock_guard<std::mutex> lock(ShaderCacheMutex);
    auto it = ShaderCache.find(key);
    if (it != ShaderCache.end()) {
        return it->second.get();
    }
    return nullptr;
}

} // namespace RHI