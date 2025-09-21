#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "RHIResource.h"
#include "RHIApi.h"

namespace RHI {

    using ShaderType = ERHIShaderType;

    struct ShaderKey {
        std::string Name;
        ShaderType Type;

        bool operator==(const ShaderKey& other) const {
            return Name == other.Name && Type == other.Type;
        }
    };

}


namespace std {
    template <>
    struct hash<RHI::ShaderKey> {
        std::size_t operator()(const RHI::ShaderKey& k) const {
            std::size_t h1 = std::hash<std::string>{}(k.Name);
            std::size_t h2 = std::hash<int>{}(static_cast<int>(k.Type));
            return h1 ^ (h2 << 1);
        }
    };
}

namespace RHI {
struct ShaderBinary {
    std::string Name;
    ShaderType Type;
    std::string EntryPoint;
    std::vector<uint8_t> BinaryCode;
    std::string SourceCode;
};
class RHIShaderLibrary {
public:
    RHIShaderLibrary(const std::string& name, ERHIShaderPlatform platform);
    virtual ~RHIShaderLibrary() = default;
    virtual ShaderKey AddShaderBinary(const ShaderBinary& binary);
    virtual const ShaderBinary* FindShaderBinary(ShaderKey key) const;

    virtual RHIShaderSP CreateShader(ShaderKey key) = 0;

    ERHIShaderPlatform GetPlatform() const { return Platform; }

    const std::string& GetName() const { return Name; }
protected:
    std::string Name;
    ERHIShaderPlatform Platform;
    mutable std::mutex ShaderCacheMutex;
    std::unordered_map<ShaderKey, std::unique_ptr<ShaderBinary>> ShaderCache;

};

} // namespace RHI