#pragma once
#include "RHIDefine.h"
#include <vector>
#include <memory>

namespace RHI
{

// 资源基类
class RHI_API RHIResource
{
public:
    explicit RHIResource(ERHIResourceType type = ERHIResourceType::Unknown)
        : ResourceType(type) {}
    virtual ~RHIResource() = default;

    ERHIResourceType GetResourceType() const { return ResourceType; }

protected:
    ERHIResourceType ResourceType;
};
// --------------------------------------------------
// 可被视图访问的资源基类
// --------------------------------------------------
class RHI_API RHIViewableResource : public RHIResource
{
public:
    explicit RHIViewableResource(ERHIResourceType type,ERHIResourceAccess access = ERHIResourceAccess::Unknown)
        : RHIResource(type)
        , TrackedAccess(access)
    {
    }

    virtual ~RHIViewableResource() = default;


protected:
    // 可以存放一些用于View管理的内部信息，比如引用计数、GPU handle 等
    ERHIResourceAccess TrackedAccess;
};
// 纹理资源
class RHI_API RHITexture : public RHIViewableResource
{
public:

    RHITexture(const RHITextureDesc& desc) : RHIViewableResource(ERHIResourceType::Texture), Desc(desc) {}
    virtual ~RHITexture() = default;

    // 获取纹理描述
    const RHITextureDesc& GetDesc() const { return Desc; }

protected:
    RHITextureDesc Desc;
};

// 缓冲区资源
class RHI_API RHIBuffer : public RHIViewableResource
{
public:

    RHIBuffer(const RHIBufferDesc& desc) : RHIViewableResource(ERHIResourceType::Buffer), Desc(desc) {}
    virtual ~RHIBuffer() = default;

    // 获取缓冲区描述
    const RHIBufferDesc& GetDesc() const { return Desc; }

protected:
    RHIBufferDesc Desc;
};


class RHI_API RHIResourceView {
public:
    explicit RHIResourceView(RHIViewableResource* Resource) : Resource(Resource) {}
private:
    RHIViewableResource* Resource = nullptr;
};

class RHI_API RHIShaderResourceView : public RHIResourceView,public RHIResource  {
public:
    explicit RHIShaderResourceView(RHIViewableResource* Resource)
        : RHIResourceView(Resource),RHIResource(ERHIResourceType::ShaderResourceView) {}
};

class RHI_API RHIUnorderedAccessView : public RHIResourceView,public RHIResource  {
public:
    explicit RHIUnorderedAccessView(RHIViewableResource* Resource)
        : RHIResourceView(Resource),RHIResource(ERHIResourceType::UnorderedAccessView) {}
};


class RHIStagingBuffer
{
public:
    RHIStagingBuffer(uint32_t InSize) : Size(InSize) {}
    virtual void* Map(uint32_t Offset, uint32_t NumBytes) = 0;
    virtual void Unmap() = 0;
protected:
    uint32_t Size;
};


struct RHIUniformBufferResource
{
    uint16_t MemberOffset = 0;                // 在 UB 内偏移
    EShaderUniformBaseType MemberType = EShaderUniformBaseType::Unknown;

    bool operator==(const RHIUniformBufferResource& Other) const
    {
        return MemberOffset == Other.MemberOffset && MemberType == Other.MemberType;
    }
};
struct RHIUniformBufferLayout
{
    std::string Name;                                  // 调试用名字
    // Graph 和非 Graph 的分类资源
    std::vector<RHIUniformBufferResource> Resources;          // 外部资源
    std::vector<RHIUniformBufferResource> GraphResources;     // Graph 资源总表
    std::vector<RHIUniformBufferResource> GraphTextures;      // Graph 纹理
    std::vector<RHIUniformBufferResource> GraphBuffers;       // Graph Buffer
    std::vector<RHIUniformBufferResource> GraphUniformBuffers;// Graph UB
    std::vector<RHIUniformBufferResource> UniformBuffers;     // 外部非 Graph UB
    uint32_t ConstantBufferSize = 0;                  // 常量数据大小
    bool bNoEmulatedUniformBuffer = false;            // 是否强制 GPU 创建真实 UB

    // 比较布局是否一致
    bool operator==(const RHIUniformBufferLayout& Other) const
    {
        return ConstantBufferSize == Other.ConstantBufferSize && Resources == Other.Resources && GraphResources == Other.GraphResources && GraphTextures == Other.GraphTextures && GraphBuffers == Other.GraphBuffers && GraphUniformBuffers == Other.GraphUniformBuffers && UniformBuffers == Other.UniformBuffers;
    }
};

class RHIUniformBuffer : public RHIResource
{
    RHIUniformBuffer(const RHIUniformBufferLayout* InLayout)
        : RHIResource(ERHIResourceType::UniformBuffer),Layout(InLayout)
    {
        ConstantData.resize(Layout->ConstantBufferSize);
        // 资源表初始化为空
        ResourceTable.resize(Layout->Resources.size(), nullptr);
    }

    // UB 布局
    const RHIUniformBufferLayout* Layout = nullptr;

    // 常量数据存储
    std::vector<uint8_t> ConstantData;

    // 资源表
    std::vector<RHIResource*> ResourceTable;

    uint32_t GetSize() const { return ConstantData.size(); }
};

// Shader基类
class RHI_API RHIShader : public RHIResource
{
public:
    RHIShader() : RHIResource(ERHIResourceType::Shader) {}
    virtual ~RHIShader() = default;
    // 获取着色器类型
    ERHIShaderFrequency GetShaderType() const { return ShaderType; }
protected:
    ERHIShaderFrequency ShaderType = ERHIShaderFrequency::Unknown; // 着色器类型
};

// 顶点着色器
class RHI_API RHIVertexShader : public RHIShader
{
public:
    RHIVertexShader() {
        ResourceType = ERHIResourceType::VertexShader;
        ShaderType = ERHIShaderFrequency::Vertex;
    }
    virtual ~RHIVertexShader() = default;
};

// 片元/像素着色器
class RHI_API RHIFragmentShader : public RHIShader
{
public:
    RHIFragmentShader() {
        ResourceType = ERHIResourceType::FragmentShader;
        ShaderType = ERHIShaderFrequency::Fragment;
    }
    virtual ~RHIFragmentShader() = default;
};

// 几何着色器
class RHI_API RHIGeometryShader : public RHIShader
{
public:
    RHIGeometryShader() {
        ResourceType = ERHIResourceType::GeometryShader;
        ShaderType = ERHIShaderFrequency::Geometry;
    }
    virtual ~RHIGeometryShader() = default;
};

// 计算着色器
class RHI_API RHIComputeShader : public RHIShader
{
public:
    RHIComputeShader() {
        ResourceType = ERHIResourceType::ComputeShader;
        ShaderType = ERHIShaderFrequency::Compute;
    }
    virtual ~RHIComputeShader() = default;
};

// 细分控制着色器
class RHI_API RHITessControlShader : public RHIShader
{
public:
    RHITessControlShader() {
        ResourceType = ERHIResourceType::TessControlShader;
        ShaderType = ERHIShaderFrequency::TessControl;
    }
    virtual ~RHITessControlShader() = default;
};

// 细分评估着色器
class RHI_API RHITessEvalShader : public RHIShader
{
public:
    RHITessEvalShader() {
        ResourceType = ERHIResourceType::TessEvalShader;
        ShaderType = ERHIShaderFrequency::TessEvaluation;
    }
    virtual ~RHITessEvalShader() = default;
};

// Mesh Shader
class RHI_API RHIMeshShader : public RHIShader
{
public:
    RHIMeshShader() {
        ResourceType = ERHIResourceType::MeshShader;
        ShaderType = ERHIShaderFrequency::Mesh;
    }
    virtual ~RHIMeshShader() = default;
};

// Task Shader
class RHI_API RHITaskShader : public RHIShader
{
public:
    RHITaskShader() {
        ResourceType = ERHIResourceType::TaskShader;
        ShaderType = ERHIShaderFrequency::Task;
    }
    virtual ~RHITaskShader() = default;
};

// 光线追踪管线着色器
class RHI_API RHIRayGenShader : public RHIShader
{
public:
    RHIRayGenShader() {
        ResourceType = ERHIResourceType::RayGenShader;
        ShaderType = ERHIShaderFrequency::RayGen;
    }
    virtual ~RHIRayGenShader() = default;
};

class RHI_API RHICloseHitShader : public RHIShader
{
public:
    RHICloseHitShader() {
        ResourceType = ERHIResourceType::CloseHitShader;
        ShaderType = ERHIShaderFrequency::ClosestHit;
    }
    virtual ~RHICloseHitShader() = default;
};

class RHI_API RHIMissShader : public RHIShader
{
public:
    RHIMissShader() {
        ResourceType = ERHIResourceType::MissShader;
        ShaderType = ERHIShaderFrequency::Miss;
    }
    virtual ~RHIMissShader() = default;
};

class RHI_API RHIAnyHitShader : public RHIShader
{
public:
    RHIAnyHitShader() {
        ResourceType = ERHIResourceType::AnyHitShader;
        ShaderType = ERHIShaderFrequency::AnyHit;
    }
    virtual ~RHIAnyHitShader() = default;
};

class RHI_API RHIIntersectionShader : public RHIShader
{
public:
    RHIIntersectionShader() {
        ResourceType = ERHIResourceType::IntersectionShader;
        ShaderType = ERHIShaderFrequency::Intersection;
    }
    virtual ~RHIIntersectionShader() = default;
};

class RHI_API RHICallableShader : public RHIShader
{
public:
    RHICallableShader() {
        ResourceType = ERHIResourceType::CallableShader;
        ShaderType = ERHIShaderFrequency::Callable;
    }
    virtual ~RHICallableShader() = default;
};

// 顶点描述状态
class RHI_API RHIVertexDescState : public RHIResource
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
class RHI_API RHIRasterizerState : public RHIResource
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
class RHI_API RHIColorBlendState : public RHIResource
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
class RHI_API RHIDepthStencilState : public RHIResource
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
class RHI_API RHIFence : public RHIResource
{
public:
    RHIFence() : RHIResource(ERHIResourceType::Fence) {}
    virtual ~RHIFence() = default;
};

// 视口/交换链资源
class RHI_API RHIViewport : public RHIResource
{
public:


    RHIViewport() : RHIResource(ERHIResourceType::Viewport) {}
    virtual ~RHIViewport() = default;
    virtual void Tick() = 0;

private:
    // 可扩展swapchain、backbuffer等管理接口
};

// 采样器资源
class RHI_API RHISampler : public RHIResource
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
using RHIStagingBufferSP = std::shared_ptr<RHIStagingBuffer>;
using RHIShaderResourceViewSP = std::shared_ptr<RHIShaderResourceView>;
using RHIUnorderedAccessViewSP = std::shared_ptr<RHIUnorderedAccessView>;

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
using RHIViewportSP = std::shared_ptr<RHIViewport>;
using RHISamplerSP = std::shared_ptr<RHISampler>;
using RHIVertexDescStateSP = std::shared_ptr<RHIVertexDescState>;
using RHIRasterizerStateSP = std::shared_ptr<RHIRasterizerState>;
using RHIColorBlendStateSP = std::shared_ptr<RHIColorBlendState>;
using RHIDepthStencilStateSP = std::shared_ptr<RHIDepthStencilState>;


// Shader Stage
struct RHI_API RHIShaderStageDesc
{
    RHIShaderSP shader = nullptr; // 指向对应阶段的shader对象
    // 可扩展entry point等
};

// Graphics Pipeline
struct RHI_API RHIGraphicsPipelineStateDesc
{
    std::vector<RHIShaderStageDesc> shaderStages;
    // 可扩展其它管线相关状态引用，如顶点描述、光栅化、混合、深度模板等
    RHIVertexDescState* vertexDescState = nullptr; // 顶点描述状态
    RHIRasterizerState* rasterizerState = nullptr; // 光栅化状态
    RHIColorBlendState* colorBlendState = nullptr; // 颜色混合状态
    RHIDepthStencilState* depthStencilState = nullptr; // 深度测试和模板状态

};

// 管线状态资源
class RHI_API RHIGraphicsPipelineState : public RHIResource
{
public:
    RHIGraphicsPipelineState(const RHIGraphicsPipelineStateDesc& desc) : RHIResource(ERHIResourceType::GraphicPipelineState), desc(desc) {}

    virtual ~RHIGraphicsPipelineState() = default;

protected:
    RHIGraphicsPipelineStateDesc desc;
};

// Compute Pipeline
struct RHI_API RHIComputePipelineStateDesc
{
    RHIShaderStageDesc shaderDesc;// 指向Compute Shader
    // 可扩展Compute管线特有参数
};

class RHI_API RHIComputePipelineState : public RHIResource
{
public:
    RHIComputePipelineState(const RHIComputePipelineStateDesc& desc) : RHIResource(ERHIResourceType::ComputePipelineState), desc(desc) {}
    ~RHIComputePipelineState() override = default;

    // 可扩展Compute管线特有参数
protected:
    RHIComputePipelineStateDesc desc;
};

// RayTracing Pipeline
struct RHI_API RHIRayTracingPipelineStateDesc
{
    // 可扩展RayTracing管线特有参数
};

class RHI_API RHIRayTracingPipelineState : public RHIResource
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


constexpr int32_t MaxColorAttachments = 8;

struct RHIRenderPassInfo
{
    struct ColorAttachment
    {
        RHITexture* Texture = nullptr;
        RHITexture* ResolveTarget = nullptr;

        uint8_t  MipIndex = 0;
        int32_t  ArraySlice = -1;

        ERenderTargetActions  Actions = ERenderTargetActions::DontCare_DontCare;
    };

    struct DepthStencilAttachment
    {
        RHITexture* Texture = nullptr;
        RHITexture* ResolveTarget = nullptr;

        ERenderTargetActions  DepthActions = ERenderTargetActions::DontCare_DontCare;
        ERenderTargetActions  StencilActions = ERenderTargetActions::DontCare_DontCare;

        uint32_t MipIndex = 0;
        uint32_t ArraySlice = 0;

        bool bReadOnlyDepth = false;
        bool bReadOnlyStencil = false;
    };

    // === 核心数据 ===
    ColorAttachment       ColorAttachments[MaxColorAttachments];
    DepthStencilAttachment DepthStencil;

    uint8_t NumColorAttachments = 0;
    uint8_t NumSamples = 1;

    RHIIntRect RenderArea;

    bool HasDepth() const { return DepthStencil.Texture != nullptr; }
};


}