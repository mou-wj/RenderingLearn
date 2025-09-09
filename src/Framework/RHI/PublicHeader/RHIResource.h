#pragma once
#include "RHIDefine.h"
#include <vector>
#include <memory>

namespace RHI
{

// 资源基类
class RHIResource
{
public:
    explicit RHIResource(ERHIResourceType type = ERHIResourceType::Unknown)
        : ResourceType(type) {}
    virtual ~RHIResource() = default;

    ERHIResourceType GetResourceType() const { return ResourceType; }

protected:
    ERHIResourceType ResourceType;
};

// 纹理资源
class RHITexture : public RHIResource
{
public:

    RHITexture(const RHITextureDesc& desc) : RHIResource(ERHIResourceType::Texture), Desc(desc) {}
    virtual ~RHITexture() = default;

    // 获取纹理描述
    const RHITextureDesc& GetDesc() const { return Desc; }

protected:
    RHITextureDesc Desc;
};

// 缓冲区资源
class RHIBuffer : public RHIResource
{
public:

    RHIBuffer(const RHIBufferDesc& desc) : RHIResource(ERHIResourceType::Buffer), Desc(desc) {}
    virtual ~RHIBuffer() = default;

    // 获取缓冲区描述
    const RHIBufferDesc& GetDesc() const { return Desc; }

protected:
    RHIBufferDesc Desc;
};


// Shader基类
class RHIShader : public RHIResource
{
public:
    RHIShader() : RHIResource(ERHIResourceType::Shader) {}
    virtual ~RHIShader() = default;
    // 获取着色器类型
    ERHIShaderType GetShaderType() const { return ShaderType; }
protected:
    ERHIShaderType ShaderType = ERHIShaderType::Unknown; // 着色器类型
};

// 顶点着色器
class RHIVertexShader : public RHIShader
{
public:
    RHIVertexShader() {
        ResourceType = ERHIResourceType::VertexShader;
        ShaderType = ERHIShaderType::Vertex;
    }
    virtual ~RHIVertexShader() = default;
};

// 片元/像素着色器
class RHIFragmentShader : public RHIShader
{
public:
    RHIFragmentShader() {
        ResourceType = ERHIResourceType::FragmentShader;
        ShaderType = ERHIShaderType::Fragment;
    }
    virtual ~RHIFragmentShader() = default;
};

// 几何着色器
class RHIGeometryShader : public RHIShader
{
public:
    RHIGeometryShader() {
        ResourceType = ERHIResourceType::GeometryShader;
        ShaderType = ERHIShaderType::Geometry;
    }
    virtual ~RHIGeometryShader() = default;
};

// 计算着色器
class RHIComputeShader : public RHIShader
{
public:
    RHIComputeShader() {
        ResourceType = ERHIResourceType::ComputeShader;
        ShaderType = ERHIShaderType::Compute;
    }
    virtual ~RHIComputeShader() = default;
};

// 细分控制着色器
class RHITessControlShader : public RHIShader
{
public:
    RHITessControlShader() {
        ResourceType = ERHIResourceType::TessControlShader;
        ShaderType = ERHIShaderType::TessControl;
    }
    virtual ~RHITessControlShader() = default;
};

// 细分评估着色器
class RHITessEvalShader : public RHIShader
{
public:
    RHITessEvalShader() {
        ResourceType = ERHIResourceType::TessEvalShader;
        ShaderType = ERHIShaderType::TessEvaluation;
    }
    virtual ~RHITessEvalShader() = default;
};

// Mesh Shader
class RHIMeshShader : public RHIShader
{
public:
    RHIMeshShader() {
        ResourceType = ERHIResourceType::MeshShader;
        ShaderType = ERHIShaderType::Mesh;
    }
    virtual ~RHIMeshShader() = default;
};

// Task Shader
class RHITaskShader : public RHIShader
{
public:
    RHITaskShader() {
        ResourceType = ERHIResourceType::TaskShader;
        ShaderType = ERHIShaderType::Task;
    }
    virtual ~RHITaskShader() = default;
};

// 光线追踪管线着色器
class RHIRayGenShader : public RHIShader
{
public:
    RHIRayGenShader() {
        ResourceType = ERHIResourceType::RayGenShader;
        ShaderType = ERHIShaderType::RayGen;
    }
    virtual ~RHIRayGenShader() = default;
};

class RHICloseHitShader : public RHIShader
{
public:
    RHICloseHitShader() {
        ResourceType = ERHIResourceType::CloseHitShader;
        ShaderType = ERHIShaderType::ClosestHit;
    }
    virtual ~RHICloseHitShader() = default;
};

class RHIMissShader : public RHIShader
{
public:
    RHIMissShader() {
        ResourceType = ERHIResourceType::MissShader;
        ShaderType = ERHIShaderType::Miss;
    }
    virtual ~RHIMissShader() = default;
};

class RHIAnyHitShader : public RHIShader
{
public:
    RHIAnyHitShader() {
        ResourceType = ERHIResourceType::AnyHitShader;
        ShaderType = ERHIShaderType::AnyHit;
    }
    virtual ~RHIAnyHitShader() = default;
};

class RHIIntersectionShader : public RHIShader
{
public:
    RHIIntersectionShader() {
        ResourceType = ERHIResourceType::IntersectionShader;
        ShaderType = ERHIShaderType::Intersection;
    }
    virtual ~RHIIntersectionShader() = default;
};

class RHICallableShader : public RHIShader
{
public:
    RHICallableShader() {
        ResourceType = ERHIResourceType::CallableShader;
        ShaderType = ERHIShaderType::Callable;
    }
    virtual ~RHICallableShader() = default;
};

// 顶点描述状态
class RHIVertexDescState : public RHIResource
{
public:

    RHIVertexDescState(const RHIVertexDescStateDesc& desc) : RHIResource(ERHIResourceType::VertexDescState), desc(desc) {}
	// 获取顶点描述状态描述
	const RHIVertexDescStateDesc& GetDesc() const { return desc; }
    virtual ~RHIVertexDescState() = default;
private:
    RHIVertexDescStateDesc desc;
};

// 光栅化状态
class RHIRasterizerState : public RHIResource
{
public:


    RHIRasterizerState(const RHIRasterizerStateDesc& desc) : RHIResource(ERHIResourceType::RasterizerState), desc(desc) {}
	// 获取光栅化状态描述
	const RHIRasterizerStateDesc& GetDesc() const { return desc; }
    virtual ~RHIRasterizerState() = default;
private:
    RHIRasterizerStateDesc desc;
};

// 颜色混合状态
class RHIColorBlendState : public RHIResource
{
public:

    RHIColorBlendState(const RHIColorBlendStateDesc& desc) : RHIResource(ERHIResourceType::ColorBlendState), desc(desc) {}
	// 获取颜色混合状态描述
	const RHIColorBlendStateDesc& GetDesc() const { return desc; }
    virtual ~RHIColorBlendState() = default;
private:
    RHIColorBlendStateDesc desc;
};

// 深度测试和模板状态
class  RHIDepthStencilState : public RHIResource
{
public:

    RHIDepthStencilState(const RHIDepthStencilStateDesc& desc) : RHIResource(ERHIResourceType::DepthStencilState), desc(desc) {}
	// 获取深度模板状态描述
	const RHIDepthStencilStateDesc& GetDesc() const { return desc; }
    virtual ~RHIDepthStencilState() = default;
private:
    RHIDepthStencilStateDesc desc;
};

// Fence资源
class RHIFence : public RHIResource
{
public:
    RHIFence() : RHIResource(ERHIResourceType::Fence) {}
    virtual ~RHIFence() = default;
};

// 视口/交换链资源
class RHIVIewport : public RHIResource
{
public:


    RHIVIewport() : RHIResource(ERHIResourceType::Viewport) {}
    virtual ~RHIVIewport() = default;
    virtual void Present() = 0;
    virtual void Tick() = 0;

private:
    // 可扩展swapchain、backbuffer等管理接口
};

// 采样器资源
class RHISampler : public RHIResource
{
public:
    RHISampler(const RHISamplerDesc& desc) : RHIResource(ERHIResourceType::Sampler), desc(desc) {}
    virtual ~RHISampler() = default;

private:
    RHISamplerDesc desc;
    // 采样器相关接口
};






// 资源智能指针类型别名（可根据需要扩展到所有资源类型）
using RHIResourceSP = std::shared_ptr<RHIResource>;
using RHITextureSP = std::shared_ptr<RHITexture>;
using RHIBufferSP = std::shared_ptr<RHIBuffer>;


using RHIShaderSP = std::shared_ptr<RHIShader>;
using RHIVertexShaderSP = std::shared_ptr<RHIVertexShader>;
using RHIFragmentShaderSP = std::shared_ptr<RHIFragmentShader>;
using RHIComputeShaderSP = std::shared_ptr<RHIComputeShader>;
using RHIGeometryShaderSP = std::shared_ptr<RHIGeometryShader>;
using RHITessControlShaderSP = std::shared_ptr<RHITessControlShader>;
using RHITessEvalShaderSP = std::shared_ptr<RHITessEvalShader>;
using RHIMeshShaderSP = std::shared_ptr<RHIMeshShader>;
using RHITaskShaderSP = std::shared_ptr<RHITaskShader>;
using RHIRayGenShaderSP = std::shared_ptr<RHIRayGenShader>;
using RHICloseHitShaderSP = std::shared_ptr<RHICloseHitShader>;
using RHIMissShaderSP = std::shared_ptr<RHIMissShader>;
using RHIAnyHitShaderSP = std::shared_ptr<RHIAnyHitShader>;
using RHIIntersectionShaderSP = std::shared_ptr<RHIIntersectionShader>;
using RHICallableShaderSP = std::shared_ptr<RHICallableShader>;



using RHIFenceSP = std::shared_ptr<RHIFence>;
using RHIVIewportSP = std::shared_ptr<RHIVIewport>;
using RHISamplerSP = std::shared_ptr<RHISampler>;
using RHIVertexDescStateSP = std::shared_ptr<RHIVertexDescState>;
using RHIRasterizerStateSP = std::shared_ptr<RHIRasterizerState>;
using RHIColorBlendStateSP = std::shared_ptr<RHIColorBlendState>;
using RHIDepthStencilStateSP = std::shared_ptr<RHIDepthStencilState>;


// Shader Stage
struct RHIShaderStageDesc
{
    RHIShaderSP shader = nullptr; // 指向对应阶段的shader对象
    // 可扩展entry point等
};

// Graphics Pipeline
struct RHIGraphicsPipelineStateDesc
{
    std::vector<RHIShaderStageDesc> shaderStages;
    // 可扩展其它管线相关状态引用，如顶点描述、光栅化、混合、深度模板等
    RHIVertexDescStateSP vertexDescState = nullptr; // 顶点描述状态
    RHIRasterizerStateSP rasterizerState = nullptr; // 光栅化状态
    RHIColorBlendStateSP colorBlendState = nullptr; // 颜色混合状态
    RHIDepthStencilStateSP depthStencilState = nullptr; // 深度测试和模板状态

};

// 管线状态资源
class RHIGraphicsPipelineState : public RHIResource
{
public:
    RHIGraphicsPipelineState(const RHIGraphicsPipelineStateDesc& desc) : RHIResource(ERHIResourceType::GraphicPipelineState), desc(desc) {}

    virtual ~RHIGraphicsPipelineState() = default;

protected:
    RHIGraphicsPipelineStateDesc desc;
};

// Compute Pipeline
struct RHIComputePipelineStateDesc
{
    RHIShaderStageDesc shaderDesc;// 指向Compute Shader
    // 可扩展Compute管线特有参数
};

class RHIComputePipelineState : public RHIResource
{
public:
    RHIComputePipelineState(const RHIComputePipelineStateDesc& desc) : RHIResource(ERHIResourceType::ComputePipelineState), desc(desc) {}
    ~RHIComputePipelineState() override = default;

    // 可扩展Compute管线特有参数
protected:
    RHIComputePipelineStateDesc desc;
};

// RayTracing Pipeline
struct RHIRayTracingPipelineStateDesc
{
    // 可扩展RayTracing管线特有参数
};

class RHIRayTracingPipelineState : public RHIResource
{
public:
    RHIRayTracingPipelineState(const RHIRayTracingPipelineStateDesc& desc) : RHIResource(ERHIResourceType::RayTracingPipelineState), desc(desc) {}
    ~RHIRayTracingPipelineState() override = default;

    // 可扩展RayTracing管线特有参数
protected:
    RHIRayTracingPipelineStateDesc desc;
};



using RHIGraphicsPipelineStateSP = std::shared_ptr<RHIGraphicsPipelineState>;
using RHIComputePipelineStateSP = std::shared_ptr<RHIComputePipelineState>;
using RHIRayTracingPipelineStateSP = std::shared_ptr<RHIRayTracingPipelineState>;

}