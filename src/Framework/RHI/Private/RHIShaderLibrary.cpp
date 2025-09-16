#include "RHIShaderLibrary.h"
#include <fstream>
#include <sstream>

namespace RHI {

    // 静态成员定义
    std::unordered_map<ShaderKey, std::unique_ptr<ShaderBinary>> RHIShaderLibrary::ShaderCache;
    std::mutex RHIShaderLibrary::ShaderCacheMutex;

    // 读取shader源码并缓存
    bool RHIShaderLibrary::LoadShaderSource(const std::string& FilePath, ShaderType Type, const std::string& EntryPoint) {
        std::ifstream file(FilePath);
        if (!file.is_open()) return false;
        std::stringstream ss;
        ss << file.rdbuf();
        ShaderKey key{ FilePath, Type };
        std::lock_guard<std::mutex> lock(ShaderCacheMutex);
        auto& bin = ShaderCache[key];
        if (!bin) bin = std::make_unique<ShaderBinary>();
        bin->Name = FilePath;
        bin->Type = Type;
        bin->EntryPoint = EntryPoint;
        bin->SourceCode = ss.str();
        return true;
    }

    // 编译shader源码为二进制并缓存（伪编译，实际应调用编译器）
    bool RHIShaderLibrary::CompileShader(const std::string& Name, ShaderType Type) {
        ShaderKey key{ Name, Type };
        std::lock_guard<std::mutex> lock(ShaderCacheMutex);
        auto it = ShaderCache.find(key);
        if (it == ShaderCache.end() || it->second->SourceCode.empty()) return false;
        // 伪编译：直接将源码转为字节码（实际应调用shaderc或DXC等工具）
        const std::string& src = it->second->SourceCode;
        it->second->BinaryCode.assign(src.begin(), src.end());
        return true;
    }

    // 查找已缓存的二进制shader
    const ShaderBinary* RHIShaderLibrary::FindShaderBinary(const std::string& Name, ShaderType Type) {
        ShaderKey key{ Name, Type };
        std::lock_guard<std::mutex> lock(ShaderCacheMutex);
        auto it = ShaderCache.find(key);
        if (it != ShaderCache.end()) return it->second.get();
        return nullptr;
    }

    // 通过RHI创建各种类型的RHIShader实例（实际应调用RHI接口，这里仅示例）
    RHIShaderSP RHIShaderLibrary::CreateShader(const std::string& Name, ShaderType Type) {
        const ShaderBinary* bin = FindShaderBinary(Name, Type);
        if (!bin) return nullptr;
        // 这里应调用RHI的CreateShader接口，伪代码如下
        // return GetGlobalRHIApi()->CreateShader(bin->BinaryCode, Type);
        return nullptr;
    }

    RHIVertexShaderSP RHIShaderLibrary::CreateVertexShader(const std::string& Name) {
        return std::static_pointer_cast<RHIVertexShader>(CreateShader(Name, ShaderType::Vertex));
    }
    RHIFragmentShaderSP RHIShaderLibrary::CreateFragmentShader(const std::string& Name) {
        return std::static_pointer_cast<RHIFragmentShader>(CreateShader(Name, ShaderType::Fragment));
    }
    RHIComputeShaderSP RHIShaderLibrary::CreateComputeShader(const std::string& Name) {
        return std::static_pointer_cast<RHIComputeShader>(CreateShader(Name, ShaderType::Compute));
    }
    RHIGeometryShaderSP RHIShaderLibrary::CreateGeometryShader(const std::string& Name) {
        return std::static_pointer_cast<RHIGeometryShader>(CreateShader(Name, ShaderType::Geometry));
    }
    RHITessControlShaderSP RHIShaderLibrary::CreateTessControlShader(const std::string& Name) {
        return std::static_pointer_cast<RHITessControlShader>(CreateShader(Name, ShaderType::TessControl));
    }
    RHITessEvalShaderSP RHIShaderLibrary::CreateTessEvalShader(const std::string& Name) {
        return std::static_pointer_cast<RHITessEvalShader>(CreateShader(Name, ShaderType::TessEvaluation));
    }
}