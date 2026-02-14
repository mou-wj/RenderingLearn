#include "VulkanRenderPass.h"
#include "VulkanResource.h"

namespace RHIVulkan {
    // Helper: 将 ERenderTargetActions 转换为 VkAttachmentLoadOp / VkAttachmentStoreOp
    inline VkAttachmentLoadOp RTActionToLoadOp(ERenderTargetActions action)
    {
        uint8_t load = uint8_t(action) >> uint8_t(ERenderTargetActions::LoadOpShift);
        switch (load)
        {
        case uint8_t(ERenderTargetLoadOp::DontCare): return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case uint8_t(ERenderTargetLoadOp::Load):     return VK_ATTACHMENT_LOAD_OP_LOAD;
        case uint8_t(ERenderTargetLoadOp::Clear):    return VK_ATTACHMENT_LOAD_OP_CLEAR;
        default: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
    }

    inline VkAttachmentStoreOp RTActionToStoreOp(ERenderTargetActions action)
    {
        uint8_t store = uint8_t(action) & uint8_t(ERenderTargetActions::StoreOpMask);
        switch (store)
        {
        case uint8_t(ERenderTargetStoreOp::DontCare): return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case uint8_t(ERenderTargetStoreOp::Store):    return VK_ATTACHMENT_STORE_OP_STORE;
        case uint8_t(ERenderTargetStoreOp::Resolve):  return VK_ATTACHMENT_STORE_OP_STORE; // Resolve 交给 resolve attachment
        default: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
    }

    // ---------------------------------------------------
    // VulkanRenderTargetLayout
    // ---------------------------------------------------
    VulkanRenderTargetLayout::VulkanRenderTargetLayout(VulkanDevice& device,
        const RHIRenderPassInfo& rpInfo)
    {
        resetAttachments();

        numColorAttachments = rpInfo.NumColorAttachments;
        hasDepthStencil = rpInfo.HasDepth();
        hasResolveAttachments = false;

        for (uint32_t i = 0; i < numColorAttachments; ++i)
        {
            colorRefs[i].attachment = i;
            colorRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            if (rpInfo.ColorAttachments[i].ResolveTarget)
                hasResolveAttachments = true;
        }

        if (hasDepthStencil)
        {
            depthRef.attachment = numColorAttachments;
            depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        if (hasResolveAttachments)
        {
            for (uint32_t i = 0; i < numColorAttachments; ++i)
            {
                if (rpInfo.ColorAttachments[i].ResolveTarget)
                {
                    resolveRefs[i].attachment = numColorAttachments + 1 + i;
                    resolveRefs[i].layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                }
            }
        }

        calculateHashes(rpInfo);
    }

    void VulkanRenderTargetLayout::resetAttachments()
    {
        std::memset(colorRefs, 0, sizeof(colorRefs));
        std::memset(resolveRefs, 0, sizeof(resolveRefs));
        std::memset(&depthRef, 0, sizeof(depthRef));
        std::memset(attachmentDescs, 0, sizeof(attachmentDescs));

        numColorAttachments = 0;
        hasDepthStencil = false;
        hasResolveAttachments = false;
        renderPassCompatibleHash = 0;
        renderPassFullHash = 0;
    }

    void VulkanRenderTargetLayout::calculateHashes(const RHIRenderPassInfo& rpInfo)
    {
        uint32_t hash = 2166136261u;
        for (uint32_t i = 0; i < rpInfo.NumColorAttachments; ++i)
        {
            auto& c = rpInfo.ColorAttachments[i];
            if (c.Texture)
            {
                hash ^= uint32_t(c.Texture->GetDesc().Format);
                hash *= 16777619u;
            }
        }

        if (hasDepthStencil)
        {
            hash ^= uint32_t(rpInfo.DepthStencil.Texture->GetDesc().Format);
            hash *= 16777619u;
        }

        renderPassFullHash = hash;
        renderPassCompatibleHash = hash; // 简化示例
    }

    // ---------------------------------------------------
    // VulkanRenderPass
    // ---------------------------------------------------
    VulkanRenderPass::VulkanRenderPass(VulkanDevice* device, const VulkanRenderTargetLayout& layout)
        : Device(device), Layout(layout)
    {
        assert(Device);

        auto attachments = layout.getAttachmentDescriptions();
        auto attachmentCount = layout.getAttachmentDescriptionCount();


        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = layout.getNumColorAttachments();
        subpass.pColorAttachments = layout.getColorAttachmentRefs();
        if (layout.hasDepth())
            subpass.pDepthStencilAttachment = layout.getDepthAttachmentRef();

        VkRenderPassCreateInfo rpInfo{};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rpInfo.attachmentCount = static_cast<uint32_t>(attachmentCount);
        rpInfo.pAttachments = attachments;
        rpInfo.subpassCount = 1;
        rpInfo.pSubpasses = &subpass;

        VkResult result = vkCreateRenderPass(Device->GetHandle(), &rpInfo, nullptr, &RenderPass);
        assert(result == VK_SUCCESS);
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        if (RenderPass != VK_NULL_HANDLE && Device)
            vkDestroyRenderPass(Device->GetHandle(), RenderPass, nullptr);
    }

    // ---------------------------------------------------
    // VulkanFramebuffer
    // ---------------------------------------------------
    VulkanFramebuffer::VulkanFramebuffer(VulkanDevice* device, const RHIRenderPassInfo& passInfo, VulkanRenderPass* renderPass)
        : Device(device)
    {

        NumColorAttachments = passInfo.NumColorAttachments;
        Extent.width = passInfo.RenderArea.Width;
        Extent.height = passInfo.RenderArea.Height;

        std::vector<VkImageView> views;
        views.resize(NumColorAttachments + (passInfo.HasDepth() ? 1 : 0));

        // 遍历 ColorAttachments
        for (uint32_t i = 0; i < NumColorAttachments; ++i)
        {
            VulkanTexture* tex = static_cast<VulkanTexture*>(passInfo.ColorAttachments[i].Texture);
            assert(tex);

            ColorImages.push_back(tex->GetImage());
            views[i] = tex->GetImageView();
        }

        // DepthStencil
        if (passInfo.HasDepth())
        {
            VulkanTexture* depthTex = static_cast<VulkanTexture*>(passInfo.DepthStencil.Texture);
            assert(depthTex);

            DepthImage = depthTex->GetImage();
            views[NumColorAttachments] = depthTex->GetImageView();
        }

        // 创建 VkFramebuffer
        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = renderPass->GetHandle();
        fbInfo.attachmentCount = static_cast<uint32_t>(views.size());
        fbInfo.pAttachments = views.data();
        fbInfo.width = Extent.width;
        fbInfo.height = Extent.height;
        fbInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(Device->GetHandle(), &fbInfo, nullptr, &Framebuffer);
        assert(result == VK_SUCCESS);
    }

    VulkanFramebuffer::~VulkanFramebuffer()
    {
        if (Framebuffer != VK_NULL_HANDLE && Device)
            vkDestroyFramebuffer(Device->GetHandle(), Framebuffer, nullptr);
    }

    bool VulkanFramebuffer::Matches(const RHIRenderPassInfo& passInfo) const
    {

        if (NumColorAttachments != passInfo.NumColorAttachments)
            return false;

        if ((DepthImage != VK_NULL_HANDLE) != passInfo.HasDepth())
            return false;


        if (Extent.width != passInfo.RenderArea.Width || Extent.height != passInfo.RenderArea.Height)
            return false;

        for (uint32_t i = 0; i < NumColorAttachments; ++i)
        {
            VulkanTexture* tex = static_cast<VulkanTexture*>(passInfo.ColorAttachments[i].Texture);
            if (!tex)
                return false;

            if (ColorImages[i] != tex->GetImage())
                return false;
        }

        if (passInfo.HasDepth())
        {
            VulkanTexture* depthTex = static_cast<VulkanTexture*>(passInfo.DepthStencil.Texture);
            if (!depthTex)
                return false;

            if (DepthImage != depthTex->GetImage())
                return false;
        }

        return true;
    }

    // ---------------------------------------------------
    // VulkanRenderPassManager
    // ---------------------------------------------------
    VulkanRenderPassManager::VulkanRenderPassManager(VulkanDevice* device)
        : Device(device)
    {
    }

    VulkanRenderPassManager::~VulkanRenderPassManager()
    {
        for (auto& kv : RenderPassCache)
            delete kv.second;
        for (auto& kv : FramebufferCache)
            delete kv.second;
    }

    VulkanRenderPass* VulkanRenderPassManager::GetOrCreateRenderPass(const VulkanRenderTargetLayout& layout)
    {
        std::lock_guard<std::mutex> lg(RenderPassMutex);
        auto it = RenderPassCache.find(layout.getFullHash());
        if (it != RenderPassCache.end())
            return it->second;

        VulkanRenderPass* rp = new VulkanRenderPass(Device, layout);
        RenderPassCache[layout.getFullHash()] = rp;
        return rp;
    }

    VulkanFramebuffer* VulkanRenderPassManager::GetOrCreateFramebuffer(const RHIRenderPassInfo& passInfo, VulkanRenderPass* renderPass)
    {
        std::lock_guard<std::mutex> lg(FramebufferMutex);
        auto hash = CalcFramebufferHash(passInfo, renderPass);
        auto it = FramebufferCache.find(hash);
        if (it != FramebufferCache.end())
            return it->second;

        VulkanFramebuffer* fb = new VulkanFramebuffer(Device, passInfo,renderPass);
        FramebufferCache[hash] = fb;
        return fb;
    }

    void VulkanRenderPassManager::NotifyDeletedImage(VkImage image)
    {
        std::lock_guard<std::mutex> lg(FramebufferMutex);
        for (auto it = FramebufferCache.begin(); it != FramebufferCache.end();)
        {
            VulkanFramebuffer* fb = it->second;
            auto found = std::find(fb->ColorImages.begin(), fb->ColorImages.end(), image);
            if (found != fb->ColorImages.end() || fb->DepthImage == image)
            {
                delete fb;
                it = FramebufferCache.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }


    uint32_t VulkanRenderPassManager::CalcFramebufferHash(const RHIRenderPassInfo& passInfo, VulkanRenderPass* renderPass)
    {
        size_t hash = std::hash<VulkanRenderPass*>{}(renderPass);
        for (uint32_t i = 0; i < passInfo.NumColorAttachments; ++i)
            hash ^= std::hash<VkImage>{}(static_cast<VulkanTexture*>(passInfo.ColorAttachments[i].Texture)->GetImage());
        if (passInfo.HasDepth())
            hash ^= std::hash<VkImage>{}(static_cast<VulkanTexture*>(passInfo.DepthStencil.Texture)->GetImage());
        hash ^= passInfo.RenderArea.Width;
        hash ^= passInfo.RenderArea.Height;
        return hash;
    }
}