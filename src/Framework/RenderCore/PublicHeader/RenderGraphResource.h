#pragma once

#include "RHIResource.h" // Assuming this is where your RHI resource definitions are
#include "RenderResourceTracker.h"
#include <string>
#include <memory>

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

    RenderGraphResourceType GetType() const { return Type; }

    // Accessors
    const std::string& GetName() const { return Name; }
    bool IsImported() const { return bImported; }
    bool IsCreated() const { return bCreated; }

    // Flags
    void SetImported(bool imported) { bImported = imported; }
    void SetCreated(bool created) { bCreated = created; }

    // RHI Resource Management
    virtual RHI::RHIResource* GetRHIResource() const { return Resource; } // Returns the underlying RHI resource
    virtual void SetRHIResource(RHI::RHIResource* resource);


protected:
    // Resource Name (for debugging and identification)
    std::string Name;
    RenderGraphResourceType Type;
    // Flags to track resource state
    bool bImported = false;  // True if the resource was imported from outside the render graph
    bool bCreated = false;   // True if the resource has been created by the render graph

    RHI::RHIResource* Resource = nullptr; // Pointer to the underlying RHI resource
};

// -------------------------------------------------------------------------------------------------
//  Render Graph Texture
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphTextureDesc : public RHI::RHITextureDesc {

    static RenderGraphTextureDesc ConvertFrom(const RHI::RHITextureDesc& other);
};


class RENDERCORE_API RenderGraphTexture : public RenderGraphResource
{
public:
    RenderGraphTexture(const std::string& name, const RenderGraphTextureDesc& desc);
    ~RenderGraphTexture() override;

    const RenderGraphTextureDesc& GetDesc() const { return desc; }
    RHI::RHITexture* GetRHITexture() const { return innerTexture; } // Returns the underlying RHI texture

    void SetRHITexture(RHI::RHITexture* texture) { innerTexture = texture; } // Allows importing an existing RHI texture

private:
    friend class RenderGraphBuilder;
    RenderGraphTextureDesc desc;
    RHI::RHITexture* innerTexture; // The underlying RHI texture
    bool IsExternal = false;
};

// -------------------------------------------------------------------------------------------------
//  Render Graph Buffer
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphBufferDesc {
    std::string Name;          // Buffer name
    uint32_t Size = 0;         // Size of the buffer in bytes
    RHI::ERHIBufferUsageFlags Usage = RHI::ERHIBufferUsageFlag::None; // Buffer flags (e.g., vertex, index, constant)
	RHI::ERHIFormat Format = RHI::ERHIFormat::Unknown; // For structured buffers, the format of each element
	uint32_t Stride = 0;      // For structured buffers, the size of each element
    static RHI::RHIBufferDesc ConvertToRHIDesc(const RenderGraphBufferDesc& desc) {
		RHI::RHIBufferDesc rhiDesc;
		rhiDesc.Size = desc.Size;
		rhiDesc.Usage = desc.Usage;
		rhiDesc.Stride = desc.Stride;
		rhiDesc.DebugName = desc.Name.c_str();
		return rhiDesc;
    }

};

class RENDERCORE_API RenderGraphBuffer : public RenderGraphResource
{
public:
    RenderGraphBuffer(const std::string& name, const RenderGraphBufferDesc& desc);
    ~RenderGraphBuffer() override;

    const RenderGraphBufferDesc& GetDesc() const { return desc; }
    RHI::RHIBuffer* GetRHIBuffer() const { return innerBuffer; }

    void SetRHIBuffer(RHI::RHIBuffer* buffer) { innerBuffer = buffer; }

private:
    friend class RenderGraphBuilder;
    RenderGraphBufferDesc desc;
    RHI::RHIBuffer* innerBuffer;
    bool IsExternal = false;

};


using RenderGraphResourceRef = RenderGraphResource*;
using RenderGraphTextureRef = RenderGraphTexture*;
using RenderGraphBufferRef = RenderGraphBuffer*;

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
};


class RENDERCORE_API RenderGraphView
{
public:
    RenderGraphView(const std::string& name);
    virtual ~RenderGraphView();

    // 改为获取对应资源的SP
    virtual RenderGraphResource* GetResource() const ;

protected:
    std::string Name;
    RenderGraphResource*  Resource;//关联的资源
};

class RENDERCORE_API RenderGraphSRV : public RenderGraphView{
public:
    RenderGraphSRV(const std::string& name);
    ~RenderGraphSRV() override;
    RHI::RHIShaderResourceView* GetRHIShaderResourceView() const { return RHIShaderResourceView; }
	void SetRHIShaderResourceView(RHI::RHIShaderResourceView* view) { RHIShaderResourceView = view; }

protected:
    RHI::RHIShaderResourceView* RHIShaderResourceView = nullptr;

};

class RENDERCORE_API RenderGraphUAV : public RenderGraphView{
public:
    RenderGraphUAV(const std::string& name);
    ~RenderGraphUAV() override;
    RHI::RHIUnorderedAccessView* GetRHIUnorderedAccessView() const { return RHIUnorderedAccessView; }
    void SetRHIUnorderedAccessView(RHI::RHIUnorderedAccessView* view) { RHIUnorderedAccessView = view; }

protected:
    RHI::RHIUnorderedAccessView* RHIUnorderedAccessView = nullptr;

};


// -------------------------------------------------------------------------------------------------
//  Texture SRV
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphTextureSRVDesc : public RenderGraphViewDesc
{
    RenderGraphTextureSRVDesc() { ViewType = RenderGraphViewType::TextureSRV; }
    RenderGraphTexture* Texture; // 关联的纹理资源
    RHI::ERHIFormat Format = RHI::ERHIFormat::Unknown;
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

    RenderGraphResource* GetResource() const override { return Desc.Texture; }
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
    RenderGraphTexture* Texture; // 关联的纹理资源
    RHI::ERHIFormat Format = RHI::ERHIFormat::Unknown;
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

    RenderGraphResource* GetResource() const override { return Desc.Texture; }
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
    RenderGraphBuffer* Buffer; // 关联的缓冲区资源
	uint64_t Offset = 0;        // 起始字节偏移
	uint64_t Size = 0;   // 元素数量
};

class RENDERCORE_API RenderGraphBufferSRV : public RenderGraphSRV
{
public:
    RenderGraphBufferSRV(const std::string& name, const RenderGraphBufferSRVDesc& desc);
    ~RenderGraphBufferSRV() override;

    RenderGraphResource* GetResource() const override { return Desc.Buffer; }
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
    RenderGraphBuffer* Buffer; // 关联的缓冲区资源
    uint64_t Offset = 0;        // 起始字节偏移
    uint64_t Size = 0;   // 元素数量
};

class RENDERCORE_API RenderGraphBufferUAV : public RenderGraphUAV
{
public:
    RenderGraphBufferUAV(const std::string& name, const RenderGraphBufferUAVDesc& desc);
    ~RenderGraphBufferUAV() override;

    RenderGraphResource* GetResource() const override { return Desc.Buffer; }
    const RenderGraphBufferUAVDesc& GetDesc() const { return Desc; }
private:
    RenderGraphBufferUAVDesc Desc;

};

// 资源智能指针
using RenderGraphResourceRef = RenderGraphResource*;
using RenderGraphTextureRef = RenderGraphTexture*;
using RenderGraphBufferRef = RenderGraphBuffer*;
using RenderGraphViewRef = RenderGraphView*;
using RenderGraphTextureSRVRef = RenderGraphTextureSRV*;
using RenderGraphTextureUAVRef = RenderGraphTextureUAV*;
using RenderGraphBufferSRVRef = RenderGraphBufferSRV*;
using RenderGraphBufferUAVRef = RenderGraphBufferUAV*;


} // namespace WR::RenderCore