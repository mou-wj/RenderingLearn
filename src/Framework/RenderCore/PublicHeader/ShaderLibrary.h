#pragma once

#include <string>
#include <memory>
#include "RHIShaderLibrary.h"

namespace RenderCore {

class ShaderLibrary
{
public:
    // 指定平台和ShaderKey创建通用RHIShader
    static RHI::RHIShaderSP CreateShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKey& key);

    // 指定平台和ShaderKey创建VertexShader
    static RHI::RHIVertexShaderSP CreateVertexShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKey& key);

    // 指定平台和ShaderKey创建Fragment/Pixel Shader
    static RHI::RHIFragmentShaderSP CreateFragmentShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKey& key);

    // 指定平台和ShaderKey创建ComputeShader
    static RHI::RHIComputeShaderSP CreateComputeShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKey& key);

    // 指定平台和ShaderKey创建GeometryShader
    static RHI::RHIGeometryShaderSP CreateGeometryShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKey& key);

    // 指定平台和ShaderKey创建TessellationControlShader
    static RHI::RHITessControlShaderSP CreateTessControlShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKey& key);

    // 指定平台和ShaderKey创建TessellationEvalShader
    static RHI::RHITessEvalShaderSP CreateTessEvalShader(RHI::ERHIShaderPlatform platform, const RHI::ShaderKey& key);

private:
    // 缓存每个平台的RHIShaderLibrary
    static std::unordered_map<RHI::ERHIShaderPlatform, RHI::RHIShaderLibrarySP> PlatformLibraryMap;
    static std::mutex PlatformLibraryMutex;

    // 获取或创建某个平台的RHIShaderLibrary
    static RHI::RHIShaderLibrarySP GetOrCreateLibrary(RHI::ERHIShaderPlatform platform);
};

} // namespace RenderCore