#include "OpenGLTransientResource.h"

namespace RHIOpenGL
{
    OpenGLTransientTexture::OpenGLTransientTexture(const std::shared_ptr<RHI::RHITexture>& inTexture)
        : RHI::RHITransientTexture(inTexture)
    {
    }

    OpenGLTransientBuffer::OpenGLTransientBuffer(const std::shared_ptr<RHI::RHIBuffer>& inBuffer)
        : RHI::RHITransientBuffer(inBuffer)
    {
    }

    OpenGLTransientResourceManager::OpenGLTransientResourceManager() = default;

    RHI::RHITransientTextureSP OpenGLTransientResourceManager::CreateTransientTexture(
        const RHI::RHITextureDesc& desc,
        uint32_t beginIndex,
        uint32_t endIndex)
    {
        auto texture = std::make_shared<OpenGLTexture>(desc);
        return std::make_shared<OpenGLTransientTexture>(texture);
    }

    RHI::RHITransientBufferSP OpenGLTransientResourceManager::CreateTransientBuffer(
        const RHI::RHIBufferDesc& desc,
        uint32_t beginIndex,
        uint32_t endIndex)
    {
        auto buffer = std::make_shared<OpenGLBuffer>(desc);
        return std::make_shared<OpenGLTransientBuffer>(buffer);
    }

    void OpenGLTransientResourceManager::ReleaseTransientTexture(const RHI::RHITransientTexture* texture)
    {
        (void)texture;
    }

    void OpenGLTransientResourceManager::ReleaseTransientBuffer(const RHI::RHITransientBuffer* buffer)
    {
        (void)buffer;
    }
}
