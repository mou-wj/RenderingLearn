#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "RHIResource.h"
#include "RHIApi.h"

namespace RHI {

    using ShaderKeyHash = size_t;

struct RHI_API ShaderBinary {
    std::string Name;
    ERHIShaderFrequency Type;
    std::string EntryPoint;
    std::vector<uint8_t> BinaryCode;
    std::string SourceCode;
};
class RHI_API RHIShaderLibrary {
public:
    RHIShaderLibrary(const std::string& name, ERHIShaderPlatform platform);
    virtual ~RHIShaderLibrary() = default;

    virtual RHIShaderSP CreateShader(ShaderKeyHash key) = 0;

    ERHIShaderPlatform GetPlatform() const { return Platform; }

    const std::string& GetName() const { return Name; }
protected:
    std::string Name;
    ERHIShaderPlatform Platform;
    mutable std::mutex ShaderCacheMutex;
    std::unordered_map<ShaderKeyHash, std::shared_ptr<RHIShader>> ShaderCache;

};

} // namespace RHI