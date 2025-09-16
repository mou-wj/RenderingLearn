#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "RHIResource.h"
#include "RHIApi.h"

namespace RHI {

// 着色器类型枚举（可用RHIResource.h里的ERHIShaderType）
using ShaderType = ERHIShaderType;

// 着色器唯一键
struct ShaderKey {
    std::string Name;
    ShaderType Type;

    bool operator==(const ShaderKey& other) const {
        return Name == other.Name && Type == other.Type;
    }
};

// Hash算法
} // namespace RHI

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

// 着色器二进制描述
struct ShaderBinary {
    std::string Name;
    ShaderType Type;
    std::string EntryPoint;
    std::vector<uint8_t> BinaryCode;
    std::string SourceCode;
};

// ShaderLibrary
class RHIShaderLibrary {
public:
    RHIShaderLibrary() = default;
    ~RHIShaderLibrary() = default;

    // 读取shader源码并缓存
    static bool LoadShaderSource(const std::string& FilePath, ShaderType Type, const std::string& EntryPoint);

    // 编译shader源码为二进制并缓存
    static bool CompileShader(const std::string& Name, ShaderType Type);

    // 查找已缓存的二进制shader
    static const ShaderBinary* FindShaderBinary(const std::string& Name, ShaderType Type);

    // 通过RHI创建各种类型的RHIShader实例
    static RHIShaderSP CreateShader(const std::string& Name, ShaderType Type);
    static RHIVertexShaderSP CreateVertexShader(const std::string& Name);
    static RHIFragmentShaderSP CreateFragmentShader(const std::string& Name);
    static RHIComputeShaderSP CreateComputeShader(const std::string& Name);
    static RHIGeometryShaderSP CreateGeometryShader(const std::string& Name);
    static RHITessControlShaderSP CreateTessControlShader(const std::string& Name);
    static RHITessEvalShaderSP CreateTessEvalShader(const std::string& Name);

private:
    static std::unordered_map<ShaderKey, std::unique_ptr<ShaderBinary>> ShaderCache;
    static std::mutex ShaderCacheMutex;
};

} //