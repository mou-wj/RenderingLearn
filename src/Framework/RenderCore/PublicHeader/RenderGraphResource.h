#pragma once

#include "RHIResource.h" // Assuming this is where your RHI resource definitions are
#include "RenderResourceTracker.h"
#include <string>
#include <memory>

namespace RenderCore {

enum class RenderGraphResourceType
{
    Unknown,
    Texture,
    Buffer,
    // Add other resource types as needed
	TextureSRV,
	TextureUAV,
	BufferSRV,
	BufferUAV,

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
    RenderGraphResource(const std::string& name, RenderGraphResourceType type);
    virtual ~RenderGraphResource();

    RenderGraphResourceType GetType() const { return Type; }

    // Accessors
    const std::string& GetName() const { return Name; }

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
    
};


class RENDERCORE_API RenderGraphTexture : public RenderGraphResource
{
public:
    RenderGraphTexture(const std::string& name, const RenderGraphTextureDesc& desc);
    ~RenderGraphTexture() override;

    const RenderGraphTextureDesc& GetDesc() const { return desc; }
    RHI::RHITexture* GetRHITexture() const { return dynamic_cast<RHI::RHITexture*>(Resource); } // Returns the underlying RHI texture
    RenderTextureTracker& GetTracker() { return tracker; }
private:
    RenderTextureTracker tracker;
    friend class RenderGraphBuilder;
    RenderGraphTextureDesc desc;
    bool IsExternal = false;
};

// -------------------------------------------------------------------------------------------------
//  Render Graph Buffer
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphBufferDesc : public RHI::RHIBufferDesc {


};

class RENDERCORE_API RenderGraphBuffer : public RenderGraphResource
{
public:
    RenderGraphBuffer(const std::string& name, const RenderGraphBufferDesc& desc);
    ~RenderGraphBuffer() override;
    RHI::RHIBuffer* GetRHIBuffer() const { return dynamic_cast<RHI::RHIBuffer*>(Resource); }
    const RenderGraphBufferDesc& GetDesc() const { return desc; }
    RenderBufferTracker& GetTracker() { return tracker; }
private:
    RenderBufferTracker tracker;
    friend class RenderGraphBuilder;
    RenderGraphBufferDesc desc;
    bool IsExternal = false;

};


using RenderGraphResourceRef = RenderGraphResource*;
using RenderGraphTextureRef = RenderGraphTexture*;
using RenderGraphBufferRef = RenderGraphBuffer*;

class RENDERCORE_API RenderGraphView : public RenderGraphResource
{
public:
    RenderGraphView(const std::string& name,RenderGraphResourceType Type);
    virtual ~RenderGraphView();

protected:
    friend class RenderGraphBuilder;
    std::string Name;
};

class RENDERCORE_API RenderGraphSRV : public RenderGraphView{
public:
    RenderGraphSRV(const std::string& name, RenderGraphResourceType Type);
    ~RenderGraphSRV() override;
    RHI::RHIShaderResourceView* GetRHIShaderResourceView() const { return dynamic_cast<RHI::RHIShaderResourceView*>(Resource); }
protected:
    friend class RenderGraphBuilder;

};

class RENDERCORE_API RenderGraphUAV : public RenderGraphView{
public:
    RenderGraphUAV(const std::string& name, RenderGraphResourceType Type);
    ~RenderGraphUAV() override;
    RHI::RHIUnorderedAccessView* GetRHIShaderResourceView() const { return dynamic_cast<RHI::RHIUnorderedAccessView*>(Resource); }
protected:
    friend class RenderGraphBuilder;

};


// -------------------------------------------------------------------------------------------------
//  Texture SRV
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphTextureSRVDesc : public RHI::RHITexSRVCreateInfo
{
    RenderGraphTextureSRVDesc() { }
    RenderGraphTexture* Texture; // 关联的纹理资源
};

class RENDERCORE_API RenderGraphTextureSRV : public RenderGraphSRV
{
public:
    RenderGraphTextureSRV(const std::string& name,const RenderGraphTextureSRVDesc& desc);
    ~RenderGraphTextureSRV() override;

private:
    friend class RenderGraphBuilder;
    RenderGraphTextureSRVDesc Desc;
};

// -------------------------------------------------------------------------------------------------
//  Texture UAV
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphTextureUAVDesc : public RHI::RHITexUAVCreateInfo
{
    RenderGraphTextureUAVDesc() { }
    RenderGraphTexture* Texture; // 关联的纹理资源
    
};

class RENDERCORE_API RenderGraphTextureUAV : public RenderGraphUAV
{
public:
    RenderGraphTextureUAV(const std::string& name, const RenderGraphTextureUAVDesc& desc);
    ~RenderGraphTextureUAV() override;

private:
    friend class RenderGraphBuilder;
    RenderGraphTextureUAVDesc Desc;
};

// -------------------------------------------------------------------------------------------------
//  Buffer SRV
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphBufferSRVDesc : public RHI::RHIBufferSRVCreateInfo
{
    RenderGraphBufferSRVDesc() { }
    RenderGraphBuffer* Buffer; // 关联的缓冲区资源
};

class RENDERCORE_API RenderGraphBufferSRV : public RenderGraphSRV
{
public:
    RenderGraphBufferSRV(const std::string& name, const RenderGraphBufferSRVDesc& desc);
    ~RenderGraphBufferSRV() override;
private:
    friend class RenderGraphBuilder;
    RenderGraphBufferSRVDesc Desc;
};

// -------------------------------------------------------------------------------------------------
//  Buffer UAV
// -------------------------------------------------------------------------------------------------
struct RENDERCORE_API RenderGraphBufferUAVDesc : public RHI::RHIBufferUAVCreateInfo
{
    RenderGraphBufferUAVDesc() {  }
    RenderGraphBuffer* Buffer; // 关联的缓冲区资源
};

class RENDERCORE_API RenderGraphBufferUAV : public RenderGraphUAV
{
public:
    RenderGraphBufferUAV(const std::string& name, const RenderGraphBufferUAVDesc& desc);
    ~RenderGraphBufferUAV() override;
private:
    friend class RenderGraphBuilder;
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