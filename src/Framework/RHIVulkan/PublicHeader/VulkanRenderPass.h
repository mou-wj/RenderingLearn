#pragma once

#include <vector>
#include <unordered_map>
#include <cassert>
#include "VulkanDevice.h"  // 你的 VulkanDevice 头文件


namespace RHIVulkan{
// 渲染目标信息，描述单个附件
    struct RenderTargetInfo
    {
        VkFormat format = VK_FORMAT_UNDEFINED;

        VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // 是否是深度附件
        bool bIsDepth = false;

        RenderTargetInfo() = default;

        RenderTargetInfo(
            VkFormat inFormat,
            VkAttachmentLoadOp inLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            VkAttachmentStoreOp inStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
            VkAttachmentLoadOp inStencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VkAttachmentStoreOp inStencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VkImageLayout inInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            VkImageLayout inFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            bool inIsDepth = false)
            : format(inFormat)
            , loadOp(inLoadOp)
            , storeOp(inStoreOp)
            , stencilLoadOp(inStencilLoadOp)
            , stencilStoreOp(inStencilStoreOp)
            , initialLayout(inInitialLayout)
            , finalLayout(inFinalLayout)
            , bIsDepth(inIsDepth)
        {}
        bool operator==(const RenderTargetInfo& other) const
        {
            return format == other.format &&
                   loadOp == other.loadOp &&
                   storeOp == other.storeOp &&
                   stencilLoadOp == other.stencilLoadOp &&
                   stencilStoreOp == other.stencilStoreOp &&
                   initialLayout == other.initialLayout &&
                   finalLayout == other.finalLayout &&
                   bIsDepth == other.bIsDepth;
        }
    };

    // 渲染通道的键，用于哈希表
    struct RenderPassKey
    {
        ::std::vector<RenderTargetInfo> colorAttachments;
        ::std::unique_ptr<RenderTargetInfo> depthAttachment;
        RenderPassKey() = default;
        RenderPassKey(const RenderPassKey& other) :
            colorAttachments(other.colorAttachments)
        {
            if (other.depthAttachment)
            {
                depthAttachment = ::std::make_unique<RenderTargetInfo>(*other.depthAttachment);
            }
        }
        bool operator==(const RenderPassKey& other) const;
    };


class VulkanDevice;

class VulkanRenderPass
{
public:
    explicit VulkanRenderPass(VulkanDevice* device);
    ~VulkanRenderPass();

    // 创建 RenderPass
    bool Create(const ::std::vector<RenderTargetInfo>& colorAttachments, const RenderTargetInfo* depthAttachment = nullptr);

    VkRenderPass GetVkRenderPass() const { return m_renderPass; }

private:
    VulkanDevice* m_device = nullptr;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
};


}