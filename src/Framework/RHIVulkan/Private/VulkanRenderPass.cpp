#include "VulkanRenderPass.h"


namespace RHIVulkan {

    VulkanRenderPass::VulkanRenderPass(VulkanDevice* device)
        : m_device(device)
    {
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        VkDevice deviceHandle = m_device->GetDevice();
        if (m_renderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(deviceHandle, m_renderPass, nullptr);
            m_renderPass = VK_NULL_HANDLE;
        }
    }

    bool VulkanRenderPass::Create(const ::std::vector<RenderTargetInfo>& colorAttachments, const RenderTargetInfo* depthAttachment)
    {
        VkDevice deviceHandle = m_device->GetDevice();

        ::std::vector<VkAttachmentDescription> attachmentDescriptions;

        // Color attachments
        for (const auto& attachmentInfo : colorAttachments)
        {
            VkAttachmentDescription attachmentDesc{};
            attachmentDesc.format = attachmentInfo.format;
            attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Support multi-sampling
            attachmentDesc.loadOp = attachmentInfo.loadOp;
            attachmentDesc.storeOp = attachmentInfo.storeOp;
            attachmentDesc.stencilLoadOp = attachmentInfo.stencilLoadOp;
            attachmentDesc.stencilStoreOp = attachmentInfo.stencilStoreOp;
            attachmentDesc.initialLayout = attachmentInfo.initialLayout;
            attachmentDesc.finalLayout = attachmentInfo.finalLayout;
            attachmentDescriptions.push_back(attachmentDesc);
        }

        // Depth attachment
        VkAttachmentDescription depthAttachmentDesc{};
        if (depthAttachment)
        {
            depthAttachmentDesc.format = depthAttachment->format;
            depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachmentDesc.loadOp = depthAttachment->loadOp;
            depthAttachmentDesc.storeOp = depthAttachment->storeOp;
            depthAttachmentDesc.stencilLoadOp = depthAttachment->stencilLoadOp;
            depthAttachmentDesc.stencilStoreOp = depthAttachment->stencilStoreOp;
            depthAttachmentDesc.initialLayout = depthAttachment->initialLayout;
            depthAttachmentDesc.finalLayout = depthAttachment->finalLayout;
            attachmentDescriptions.push_back(depthAttachmentDesc);
        }

        ::std::vector<VkAttachmentReference> colorAttachmentRefs;
        for (uint32_t i = 0; i < colorAttachments.size(); ++i)
        {
            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = i;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentRefs.push_back(colorAttachmentRef);
        }

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = static_cast<uint32_t>(colorAttachments.size());
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
        subpass.pColorAttachments = colorAttachmentRefs.data();
        if (depthAttachment)
        {
            subpass.pDepthStencilAttachment = &depthAttachmentRef;
        }

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
        renderPassInfo.pAttachments = attachmentDescriptions.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        // Subpass dependencies for layout transitions
        ::std::vector<VkSubpassDependency> dependencies;
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies.push_back(dependency);

        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(deviceHandle, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        {
            return false;
        }

        return true;
    }

}