#pragma once

#include <vector>
#include <unordered_map>
#include <cassert>
#include "VulkanDevice.h"  // 你的 VulkanDevice 头文件


namespace RHIVulkan{

    class VulkanDevice;
    class VulkanCommandContext;
    class VulkanCmdBuffer;

    // ---------------------------------------------------
    // VulkanRenderTargetLayout: 表示 RenderPass 附件布局
    // ---------------------------------------------------
    class VulkanRenderTargetLayout
    {
    public:
        VulkanRenderTargetLayout() = default;
        VulkanRenderTargetLayout(VulkanDevice& device, const RHIRenderPassInfo& rpInfo);

        uint32_t getCompatibleHash() const { return renderPassCompatibleHash; }
        uint32_t getFullHash() const { return renderPassFullHash; }

        const VkAttachmentReference* getColorAttachmentRefs() const { return colorRefs; }
        const VkAttachmentReference* getResolveAttachmentRefs() const { return hasResolveAttachments ? resolveRefs : nullptr; }
        const VkAttachmentReference* getDepthAttachmentRef() const { return hasDepthStencil ? &depthRef : nullptr; }
        const VkAttachmentDescription* getAttachmentDescriptions() const { return attachmentDescs; }
        uint32_t getAttachmentDescriptionCount() const { return attachmentDescCount; }
        uint32_t getNumColorAttachments() const { return numColorAttachments; }
        bool hasDepth() const { return hasDepthStencil; }
        bool hasResolve() const { return hasResolveAttachments; }

    private:
        void resetAttachments();
        void calculateHashes(const RHIRenderPassInfo& rpInfo);

    private:
        VkAttachmentReference colorRefs[MAX_RENDER_TARGETS];
        VkAttachmentReference depthRef;
        VkAttachmentReference resolveRefs[MAX_RENDER_TARGETS];

        VkAttachmentDescription attachmentDescs[MAX_RENDER_TARGETS * 2 + 2];
        uint32_t attachmentDescCount = 0;

        uint8_t numColorAttachments = 0;
        bool hasDepthStencil = false;
        bool hasResolveAttachments = false;

        uint32_t renderPassCompatibleHash = 0;
        uint32_t renderPassFullHash = 0;
    };

    // RenderPass 管理
    class VulkanRenderPass
    {
    public:
        VulkanRenderPass(VulkanDevice* device, const VulkanRenderTargetLayout& layout);
        ~VulkanRenderPass();

        VkRenderPass GetHandle() const { return RenderPass; }
        const VulkanRenderTargetLayout& GetLayout() const { return Layout; }

    private:
        VulkanDevice* Device = nullptr;
        VkRenderPass RenderPass = VK_NULL_HANDLE;
        VulkanRenderTargetLayout Layout;
    };

    // Framebuffer 管理
    class VulkanFramebuffer
    {
    public:
        VulkanFramebuffer(VulkanDevice* device, const RHIRenderPassInfo& passInfo, VulkanRenderPass* renderPass);
        ~VulkanFramebuffer();

        VkFramebuffer GetHandle() const { return Framebuffer; }
        bool Matches(const RHIRenderPassInfo& passInfo) const;

    private:
        friend class VulkanRenderPassManager;
        VulkanDevice* Device = nullptr;
        VkFramebuffer Framebuffer = VK_NULL_HANDLE;

        // 缓存 RenderTarget 的 VkImage
        std::vector<VkImage> ColorImages;
        VkImage DepthImage = VK_NULL_HANDLE;

        VkExtent2D Extent{};
        uint32_t NumColorAttachments = 0;
        bool HasDepth;
    };



    // RenderPassManager
    class VulkanRenderPassManager
    {
    public:
        VulkanRenderPassManager(VulkanDevice* device);
        ~VulkanRenderPassManager();

        // 获取或创建 RenderPass
        VulkanRenderPass* GetOrCreateRenderPass(const VulkanRenderTargetLayout& layout);

        // 获取或创建 Framebuffer
        VulkanFramebuffer* GetOrCreateFramebuffer(const RHIRenderPassInfo& passInfo, VulkanRenderPass* renderPass);

        // 清理被删除的 Image
        void NotifyDeletedImage(VkImage image);

    private:
        uint32_t CalcFramebufferHash(const RHIRenderPassInfo& passInfo, VulkanRenderPass* renderPass);
        VulkanDevice* Device = nullptr;


        std::mutex RenderPassMutex;
        std::mutex FramebufferMutex;

        std::unordered_map<uint32_t, VulkanRenderPass*> RenderPassCache;
        std::unordered_map<uint32_t, VulkanFramebuffer*> FramebufferCache;
    };

}