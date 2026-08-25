#pragma once

#include "RHITransientResource.h"
#include "OpenGLResource.h"
#include <memory>

namespace RHIOpenGL
{
    class OpenGLTransientTexture : public RHI::RHITransientTexture
    {
    public:
        explicit OpenGLTransientTexture(const std::shared_ptr<RHI::RHITexture>& inTexture);
        ~OpenGLTransientTexture() override = default;
    };
    using OpenGLTransientTextureSP = std::shared_ptr<OpenGLTransientTexture>;

    class OpenGLTransientBuffer : public RHI::RHITransientBuffer
    {
    public:
        explicit OpenGLTransientBuffer(const std::shared_ptr<RHI::RHIBuffer>& inBuffer);
        ~OpenGLTransientBuffer() override = default;
    };
    using OpenGLTransientBufferSP = std::shared_ptr<OpenGLTransientBuffer>;

    class OpenGLTransientResourceManager : public RHI::RHITransientResourceManager
    {
    public:
        OpenGLTransientResourceManager();
        ~OpenGLTransientResourceManager() override = default;

        RHI::RHITransientTextureSP CreateTransientTexture(
            const RHI::RHITextureDesc& desc,
            uint32_t beginIndex,
            uint32_t endIndex) override;

        RHI::RHITransientBufferSP CreateTransientBuffer(
            const RHI::RHIBufferDesc& desc,
            uint32_t beginIndex,
            uint32_t endIndex) override;

        void ReleaseTransientTexture(const RHI::RHITransientTexture* texture) override;
        void ReleaseTransientBuffer(const RHI::RHITransientBuffer* buffer) override;
    };
    using OpenGLTransientResourceManagerSP = std::shared_ptr<OpenGLTransientResourceManager>;
}
