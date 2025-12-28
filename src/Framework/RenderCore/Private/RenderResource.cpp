#include "RenderResource.h"

namespace RenderCore {

// RenderResource
RenderResource::RenderResource() = default;
RenderResource::~RenderResource() = default;

void RenderResource::InitResource() {
    bInitialized = true;
}

void RenderResource::ReleaseResource() {
    bInitialized = false;
}

void RenderResource::SetName(const std::string& name) {
    Name = name;
}

const std::string& RenderResource::GetName() const {
    return Name;
}

bool RenderResource::IsInitialized() const {
    return bInitialized.load();
}

// RenderTexture
RenderTexture::RenderTexture() = default;
RenderTexture::~RenderTexture() = default;

void RenderTexture::InitResource() {
    RenderResource::InitResource();
    // 纹理资源初始化逻辑
}

void RenderTexture::ReleaseResource() {
    // 纹理资源释放逻辑
    RenderResource::ReleaseResource();
}

// RenderBuffer
RenderBuffer::RenderBuffer() = default;
RenderBuffer::~RenderBuffer() = default;

void RenderBuffer::InitResource() {
    RenderResource::InitResource();
    // 缓冲区资源初始化逻辑
}

void RenderBuffer::ReleaseResource() {
    // 缓冲区资源释放逻辑
    RenderResource::ReleaseResource();
}

// RenderVertexBuffer
RenderVertexBuffer::RenderVertexBuffer() = default;
RenderVertexBuffer::~RenderVertexBuffer() = default;

void RenderVertexBuffer::InitResource() {
    RenderBuffer::InitResource();
    // 顶点缓冲区初始化逻辑
}

void RenderVertexBuffer::ReleaseResource() {
    // 顶点缓冲区释放逻辑
    RenderBuffer::ReleaseResource();
}

// RenderIndexBuffer
RenderIndexBuffer::RenderIndexBuffer() = default;
RenderIndexBuffer::~RenderIndexBuffer() = default;

void RenderIndexBuffer::InitResource() {
    RenderBuffer::InitResource();
    // 索引缓冲区初始化逻辑
}

void RenderIndexBuffer::ReleaseResource() {
    // 索引缓冲区释放逻辑
    RenderBuffer::ReleaseResource();
}

namespace GlobalResourceCache {

    bool Init(){
        return true;
    }
    void Release(){

    }

}

} // namespace RenderCore