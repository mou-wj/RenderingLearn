#pragma once

#include "RHIResource.h" // Assuming this is where your RHI resource definitions are
#include <string>
#include <memory>

using namespace RHI;

namespace RenderCore {

enum class RenderGraphResourceType
{
    Texture,
    Buffer,
    // Add other resource types as needed

};



// Forward Declarations (to avoid circular dependencies)
class RenderGraph;
class RenderGraphBuilder;

// -------------------------------------------------------------------------------------------------
//  Render Graph Resource Base Class
// -------------------------------------------------------------------------------------------------
class RENDERCORE_API RenderGraphResource
{
public:
    // Construction/Destruction
    RenderGraphResource(const std::string& name);
    virtual ~RenderGraphResource();

    // Accessors
    const std::string& GetName() const { return Name; }
    bool IsImported() const { return bImported; }
    bool IsCreated() const { return bCreated; }

    // Flags
    void SetImported(bool imported) { bImported = imported; }
    void SetCreated(bool created) { bCreated = created; }

    // RHI Resource Management
    virtual RHIResourceSP GetRHIResource() const { return RHIResource; } // Returns the underlying RHI resource
    virtual void SetRHIResource(RHIResourceSP resource);

    // Virtual method to allow the render graph to create the underlying RHI resource
    virtual void Create(RenderGraphBuilder& builder) {};

protected:
    // Resource Name (for debugging and identification)
    std::string Name;

    // Flags to track resource state
    bool bImported = false;  // True if the resource was imported from outside the render graph
    bool bCreated = false;   // True if the resource has been created by the render graph

    RHIResourceSP RHIResource = nullptr; // Pointer to the underlying RHI resource
};

// -------------------------------------------------------------------------------------------------
//  Render Graph Texture
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphTextureDesc {
	std::string Name;          // Texture name
	uint32_t Width = 0;        // Texture width
	uint32_t Height = 0;       // Texture height
	ERHIFormat Format = ERHIFormat::Unknown; // Texture format (e.g., RGBA8, Depth24)
	ERHITextureFlags Flags = ERHITextureFlags::None; // Texture flags (e.g., render target, shader resource)
	ERHITextureType Type = ERHITextureType::Texture2D; // Texture type (e.g., 2D, 3D, Cube)
	uint32_t MipLevels = 1;    // Number of mip levels
	uint32_t ArraySize = 1;    // Array size for texture arrays



};


class RENDERCORE_API RenderGraphTexture : public RenderGraphResource
{
public:
    RenderGraphTexture(const std::string& name, const RenderGraphTextureDesc& desc);
    ~RenderGraphTexture() override;

    const RenderGraphTextureDesc& GetDesc() const { return desc; }
    RHITextureSP GetRHITexture() const { return innerTexture; } // Returns the underlying RHI texture

    void SetRHITexture(RHITextureSP texture) { innerTexture = texture; } // Allows importing an existing RHI texture

    void Create(RenderGraphBuilder& builder) override;

private:
    RenderGraphTextureDesc desc;
    RHITextureSP innerTexture; // The underlying RHI texture
};

// -------------------------------------------------------------------------------------------------
//  Render Graph Buffer
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphBufferDesc {
    std::string Name;          // Buffer name
    uint32_t Size = 0;         // Size of the buffer in bytes
    ERHIBufferFlags Flags = ERHIBufferFlags::None; // Buffer flags (e.g., vertex, index, constant)
    ERHIBufferType Type = ERHIBufferType::Vertex; // Buffer type (e.g., vertex, index, constant)
};

class RENDERCORE_API RenderGraphBuffer : public RenderGraphResource
{
public:
    RenderGraphBuffer(const std::string& name, const RenderGraphBufferDesc& desc);
    ~RenderGraphBuffer() override;

    const RenderGraphBufferDesc& GetDesc() const { return desc; }
    RHIBufferSP GetRHIBuffer() const { return innerBuffer; }

    void SetRHIBuffer(RHIBufferSP buffer) { innerBuffer = buffer; }

    void Create(RenderGraphBuilder& builder) override;

private:
    RenderGraphBufferDesc desc;
    RHIBufferSP innerBuffer;
};





using RenderGraphResourceSP = std::shared_ptr<RenderGraphResource>;
using RenderGraphTextureSP = std::shared_ptr<RenderGraphTexture>;
using RenderGraphBufferSP = std::shared_ptr<RenderGraphBuffer>;

// 新增：资源视图类型
enum class RenderGraphViewType
{
    TextureSRV,
    TextureUAV,
    BufferSRV,
    BufferUAV
};
struct RENDERCORE_API RenderGraphViewDesc
{
    RenderGraphViewType ViewType;
    // 视图格式
    ERHIFormat Format = ERHIFormat::Unknown;
    // mip/slice等可选参数
    uint32_t MipLevel = 0;
    uint32_t ArraySlice = 0;
};


class RENDERCORE_API RenderGraphView: public RenderGraphResource
{
public:
    RenderGraphView(const std::string& name);
    virtual ~RenderGraphView();

    // 改为获取对应资源的SP
    virtual RenderGraphResourceSP GetResource() const ;

protected:
    std::string Name;
    RenderGraphResourceSP  Resource;//关联的资源
};

class RENDERCORE_API RenderGraphSRV : public RenderGraphView{
public:
    RenderGraphSRV(const std::string& name);
    ~RenderGraphSRV() override;
    RHIShaderResourceView* GetRHIShaderResourceView() const { return RHIShaderResourceView; }

protected:
    RHIShaderResourceView* RHIShaderResourceView = nullptr;

};

class RENDERCORE_API RenderGraphUAV : public RenderGraphView{
public:
    RenderGraphUAV(const std::string& name);
    ~RenderGraphUAV() override;
    RHIUnorderedAccessView* GetRHIUnorderedAccessView() const { return RHIUnorderedAccessView; }

protected:
    RHIUnorderedAccessView* RHIUnorderedAccessView = nullptr;  

};


// -------------------------------------------------------------------------------------------------
//  Texture SRV
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphTextureSRVDesc : public RenderGraphViewDesc
{
    RenderGraphTextureSRVDesc() { ViewType = RenderGraphViewType::TextureSRV; }
    RenderGraphTextureSP Texture; // 关联的纹理资源
    uint32_t MipLevel = 0; // 关联的mip层
    uint32_t ArraySlice = 0; // 关联的数组切片
    uint32_t NumMipLevels = 1; // 关联的mip层数
    uint32_t NumArraySlices = 1; // 关联的数组切片数
};

class RENDERCORE_API RenderGraphTextureSRV : public RenderGraphSRV
{
public:
    RenderGraphTextureSRV(const std::string& name,const RenderGraphTextureSRVDesc& desc);
    ~RenderGraphTextureSRV() override;

    RenderGraphResourceSP GetResource() const override { return Desc.Texture; }
    const RenderGraphTextureSRVDesc& GetDesc() const { return Desc; }
private:
    RenderGraphTextureSRVDesc Desc;
};

// -------------------------------------------------------------------------------------------------
//  Texture UAV
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphTextureUAVDesc : public RenderGraphViewDesc
{
    RenderGraphTextureUAVDesc() { ViewType = RenderGraphViewType::TextureUAV; }
    RenderGraphTextureSP Texture; // 关联的纹理资源
    uint32_t MipLevel = 0; // 关联的mip层
    uint32_t ArraySlice = 0; // 关联的数组切片
    uint32_t NumMipLevels = 1; // 关联的mip层数
    uint32_t NumArraySlices = 1; // 关联的数组切片数
    
};

class RENDERCORE_API RenderGraphTextureUAV : public RenderGraphUAV
{
public:
    RenderGraphTextureUAV(const std::string& name, const RenderGraphTextureUAVDesc& desc);
    ~RenderGraphTextureUAV() override;

    RenderGraphResourceSP GetResource() const override { return Desc.Texture; }
    const RenderGraphTextureUAVDesc& GetDesc() const { return Desc; }
private:
    RenderGraphTextureUAVDesc Desc;
};

// -------------------------------------------------------------------------------------------------
//  Buffer SRV
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphBufferSRVDesc : public RenderGraphViewDesc
{
    RenderGraphBufferSRVDesc() { ViewType = RenderGraphViewType::BufferSRV; }
    RenderGraphBufferSP Buffer; // 关联的缓冲区资源
    RHIBufferRegion Region; // 缓冲区区域
};

class RENDERCORE_API RenderGraphBufferSRV : public RenderGraphSRV
{
public:
    RenderGraphBufferSRV(const std::string& name, const RenderGraphBufferSRVDesc& desc);
    ~RenderGraphBufferSRV() override;

    RenderGraphResourceSP GetResource() const override { return Desc.Buffer; }
    const RenderGraphBufferSRVDesc& GetDesc() const { return Desc; }
private:
    RenderGraphBufferSRVDesc Desc;
};

// -------------------------------------------------------------------------------------------------
//  Buffer UAV
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphBufferUAVDesc : public RenderGraphViewDesc
{
    RenderGraphBufferUAVDesc() { ViewType = RenderGraphViewType::BufferUAV; }
    RenderGraphBufferSP Buffer; // 关联的缓冲区资源
    RHIBufferRegion Region; // 缓冲区区域
};

class RENDERCORE_API RenderGraphBufferUAV : public RenderGraphUAV
{
public:
    RenderGraphBufferUAV(const std::string& name, const RenderGraphBufferUAVDesc& desc);
    ~RenderGraphBufferUAV() override;

    RenderGraphResourceSP GetResource() const override { return Desc.Buffer; }
    const RenderGraphBufferUAVDesc& GetDesc() const { return Desc; }
private:
    RenderGraphBufferUAVDesc Desc;

};

// 资源智能指针
using RenderGraphResourceSP = std::shared_ptr<RenderGraphResource>;
using RenderGraphTextureSP = std::shared_ptr<RenderGraphTexture>;
using RenderGraphBufferSP = std::shared_ptr<RenderGraphBuffer>;
using RenderGraphViewSP = std::shared_ptr<RenderGraphView>;
using RenderGraphTextureSRVSP = std::shared_ptr<RenderGraphTextureSRV>;
using RenderGraphTextureUAVSP = std::shared_ptr<RenderGraphTextureUAV>;
using RenderGraphBufferSRVSP = std::shared_ptr<RenderGraphBufferSRV>;
using RenderGraphBufferUAVSP = std::shared_ptr<RenderGraphBufferUAV>;


} // namespace WR::RenderCore