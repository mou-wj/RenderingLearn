#pragma once

#include <string>
#include <memory>
#include <atomic>

namespace RenderCore {

// 渲染资源生命周期管理基类
class RENDERCORE_API RenderResource
{
public:
    RenderResource();
    virtual ~RenderResource();

    virtual void InitResource();
    virtual void ReleaseResource();

    void SetName(const std::string& name);
    const std::string& GetName() const;

    bool IsInitialized() const;

protected:
    std::string Name;
    std::atomic<bool> bInitialized{ false };
};

// 纹理资源
class RENDERCORE_API RenderTexture : public RenderResource
{
public:
    RenderTexture();
    ~RenderTexture() override;

    void InitResource() override;
    void ReleaseResource() override;
    // 其他纹理相关接口
};

// 通用缓冲区资源
class RENDERCORE_API RenderBuffer : public RenderResource
{
public:
    RenderBuffer();
    ~RenderBuffer() override;

    void InitResource() override;
    void ReleaseResource() override;
    // 其他缓冲区相关接口
};

// 顶点缓冲区
class RENDERCORE_API RenderVertexBuffer : public RenderBuffer
{
public:
    RenderVertexBuffer();
    ~RenderVertexBuffer() override;

    void InitResource() override;
    void ReleaseResource() override;
    // 其他顶点缓冲区相关接口
};

// 索引缓冲区
class RENDERCORE_API RenderIndexBuffer : public RenderBuffer
{
public:
    RenderIndexBuffer();
    ~RenderIndexBuffer() override;

    void InitResource() override;
    void ReleaseResource() override;
    // 其他索引缓冲区相关接口
};

class RENDERCORE_API RenderTarget : public RenderResource {
public:
    virtual ~RenderTarget() {}

    virtual void InitResource() {}
    virtual void ReleaseResource() {}

};

// 智能指针类型
using RenderResourceSP = std::shared_ptr<RenderResource>;
using RenderTextureSP = std::shared_ptr<RenderTexture>;
using RenderBufferSP = std::shared_ptr<RenderBuffer>;
using RenderVertexBufferSP = std::shared_ptr<RenderVertexBuffer>;
using RenderIndexBufferSP = std::shared_ptr<RenderIndexBuffer>;

namespace GlobalResourceCache{

    bool Init();
    void Release();

}

} // namespace RenderCore