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
class RenderGraphResource
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
    virtual void Create(RenderGraphBuilder& builder) = 0;

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
struct RenderGraphTextureDesc {
	std::string Name;          // Texture name
	uint32_t Width = 0;        // Texture width
	uint32_t Height = 0;       // Texture height
	ERHIFormat Format = ERHIFormat::Unknown; // Texture format (e.g., RGBA8, Depth24)
	ERHITextureFlags Flags = ERHITextureFlags::None; // Texture flags (e.g., render target, shader resource)
	ERHITextureType Type = ERHITextureType::Texture2D; // Texture type (e.g., 2D, 3D, Cube)
	uint32_t MipLevels = 1;    // Number of mip levels
	uint32_t ArraySize = 1;    // Array size for texture arrays



};


class RenderGraphTexture : public RenderGraphResource
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
struct RenderGraphBufferDesc {
    std::string Name;          // Buffer name
    uint32_t Size = 0;         // Size of the buffer in bytes
    ERHIBufferFlags Flags = ERHIBufferFlags::None; // Buffer flags (e.g., vertex, index, constant)
    ERHIBufferType Type = ERHIBufferType::Vertex; // Buffer type (e.g., vertex, index, constant)
};

class RenderGraphBuffer : public RenderGraphResource
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


} // namespace WR::RenderCore