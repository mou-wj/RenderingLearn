#include "VulkanResource.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanPlatformSurport.h"

#include "VulkanRHIUtils.h"

namespace RHIVulkan{

// Vulkan Texture
VulkanTexture::VulkanTexture(VulkanDevice* device, const RHITextureDesc& desc)
    : RHITexture(desc), Device(device) {
    VkDevice vkDevice = Device->GetDevice();
    VulkanMemoryManager* memoryManager = Device->GetMemoryManager();

    // Create VkImage
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D; // TODO: Support other image types
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // TODO: Support other formats
    imageInfo.extent.width = Desc.Width;
    imageInfo.extent.height = Desc.Height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT; // TODO: Support other usages
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(vkDevice, &imageInfo, nullptr, &Image) != VK_SUCCESS) {
    }

    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vkDevice, Image, &memRequirements);

    // Allocate memory
    if (!memoryManager->Allocate(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Allocation)) {
        vkDestroyImage(vkDevice, Image, nullptr);
    }

    // Bind memory to image
    vkBindImageMemory(vkDevice, Image, Allocation.GetMemory(), Allocation.GetOffset());

    // Create VkImageView
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // TODO: Support other view types
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // TODO: Support other formats
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.components = VkComponentMapping{
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A
    };

    if (vkCreateImageView(vkDevice, &viewInfo, nullptr, &ImageView) != VK_SUCCESS) {
        vkDestroyImage(vkDevice, Image, nullptr);
        memoryManager->Free(Allocation);
    }
}

VulkanTexture::VulkanTexture(VulkanDevice* device, const RHITextureDesc& desc, VkImage image)
    : RHITexture(desc), Device(device), Image(image)
{
    VkDevice vkDevice = Device->GetDevice();

    // Create VkImageView
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = Image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // TODO: Support other view types
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // TODO: Support other formats
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.components = VkComponentMapping{
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A
    };

    if (vkCreateImageView(vkDevice, &viewInfo, nullptr, &ImageView) != VK_SUCCESS) {

    }
}

VulkanTexture::~VulkanTexture() {
    VkDevice device = Device->GetDevice();
    VulkanMemoryManager* memoryManager = Device->GetMemoryManager();

    if (ImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, ImageView, nullptr);
    }

    if (Image != VK_NULL_HANDLE) {
        vkDestroyImage(device, Image, nullptr);
    }

    memoryManager->Free(Allocation);
}

// Vulkan Buffer
VulkanBuffer::VulkanBuffer(VulkanDevice* device, const RHIBufferDesc& desc)
    : RHIBuffer(desc), Device(device) {
    VkDevice vkDevice = Device->GetDevice();
    VulkanMemoryManager* memoryManager = Device->GetMemoryManager();

    // Create VkBuffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = Desc.Size;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; // TODO: Support other usages
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &Buffer) != VK_SUCCESS) {
    }

    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vkDevice, Buffer, &memRequirements);

    // Allocate memory
    if (!memoryManager->Allocate(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Allocation)) {
        vkDestroyBuffer(vkDevice, Buffer, nullptr);
    }

    // Bind memory to buffer
    vkBindBufferMemory(vkDevice, Buffer, Allocation.GetMemory(), Allocation.GetOffset());
}

VulkanBuffer::~VulkanBuffer() {
    VkDevice device = Device->GetDevice();
    VulkanMemoryManager* memoryManager = Device->GetMemoryManager();

    if (Buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, Buffer, nullptr);
    }

    memoryManager->Free(Allocation);
}

// Vulkan Viewport (Swapchain)
VulkanViewport::VulkanViewport(VulkanDevice* device, uint32_t width, uint32_t height, void* windowHandle)
    : RHIVIewport(), Device(device), WindowHandle(windowHandle) {


    if (!WindowHandle)
    {
    }


    VulkanSwapchain::SwapchainDesc swapchainDesc = {};
    swapchainDesc.windowHandle = WindowHandle;
    swapchainDesc.width = width;
    swapchainDesc.height = height;

    Swapchain = new VulkanSwapchain(Device, swapchainDesc);

}

VulkanViewport::~VulkanViewport() {
    VkDevice device = Device->GetDevice();

    delete Swapchain;

#ifdef _WIN32
    glfwDestroyWindow((GLFWwindow*)WindowHandle);
    glfwTerminate();
#endif
}

void VulkanViewport::Present()
{

	if (Swapchain) {
		Swapchain->Present(curPresentImageIndex,VK_NULL_HANDLE);
	}
	else {
		
	}

}

void VulkanViewport::Tick()
{
}


VulkanRHIVertexDescState::VulkanRHIVertexDescState(VulkanDevice* device, const RHIVertexDescStateDesc& desc): RHIVertexDescState(desc), Device(device)
{
    // Initialize vertex input state
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(desc.bindings.size());
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(desc.attributes.size());
    bindingDescriptions.resize(desc.bindings.size());
    attributeDescriptions.resize(desc.attributes.size());
    for (size_t i = 0; i < desc.bindings.size(); ++i) {
        const auto& bindingDesc = desc.bindings[i];
        bindingDescriptions[i].binding = bindingDesc.binding;
        bindingDescriptions[i].stride = bindingDesc.stride;
        bindingDescriptions[i].inputRate = (bindingDesc.inputRate == ERHIInputRate::PerVertex) ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
    }
    for (size_t i = 0; i < desc.attributes.size(); ++i) {
        const auto& attributeDesc = desc.attributes[i];
        attributeDescriptions[i].location = attributeDesc.location;
        //attributeDescriptions[i].binding = attributeDesc.binding;
        attributeDescriptions[i].format = TransforFormatFrom(attributeDesc.format); // TODO: Support other formats
        attributeDescriptions[i].offset = 0; // TODO: Support other offsets
    }
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

}
VulkanRHIVertexDescState::~VulkanRHIVertexDescState()
{

}


// ----------------------------
// 光栅化状态 Vulkan派生
// ----------------------------
VulkanRHIRasterizerState::VulkanRHIRasterizerState(VulkanDevice* device, const RHIRasterizerStateDesc& desc) : RHIRasterizerState(desc), Device(device)
{
    VkDevice vkDevice = Device->GetDevice();
    rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerInfo.depthClampEnable = desc.depthClampEnable;
    rasterizerInfo.rasterizerDiscardEnable = desc.rasterizerDiscardEnable;

    switch (desc.polygonMode) {
    case ERHIPolygonMode::Fill:
        rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
        break;
    case ERHIPolygonMode::Line:
        rasterizerInfo.polygonMode = VK_POLYGON_MODE_LINE;
        break;
    case ERHIPolygonMode::Point:
        rasterizerInfo.polygonMode = VK_POLYGON_MODE_POINT;
        break;
    default:
        rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
        break;
    }

    switch (desc.cullMode) {
    case ERHICullMode::None:
        rasterizerInfo.cullMode = VK_CULL_MODE_NONE;
        break;
    case ERHICullMode::Front:
        rasterizerInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
        break;
    case ERHICullMode::Back:
        rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        break;
    case ERHICullMode::FrontAndBack:
        rasterizerInfo.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
        break;
    default:
        rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        break;
    }

    switch (desc.frontFace) {
    case ERHIFrontFace::CounterClockwise:
        rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        break;
    case ERHIFrontFace::Clockwise:
        rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        break;
    default:
        rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        break;
    }

    rasterizerInfo.depthBiasEnable = desc.depthBiasEnable;
    rasterizerInfo.depthBiasConstantFactor = desc.depthBiasConstantFactor;
    rasterizerInfo.depthBiasClamp = desc.depthBiasClamp;
    rasterizerInfo.depthBiasSlopeFactor = desc.depthBiasSlopeFactor;
    rasterizerInfo.lineWidth = desc.lineWidth;
}

VulkanRHIRasterizerState::~VulkanRHIRasterizerState()
{
}

// ----------------------------
// 颜色混合状态 Vulkan派生
// ----------------------------
VulkanRHIColorBlendState::VulkanRHIColorBlendState(VulkanDevice* device, const RHIColorBlendStateDesc& desc) : RHIColorBlendState(desc), Device(device)
{
    VkDevice vkDevice = Device->GetDevice();
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = desc.logicOpEnable;
    colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // TODO: Support other logic ops
    colorBlendInfo.attachmentCount = static_cast<uint32_t>(desc.attachments.size());
    attachmentStates.resize(desc.attachments.size());

    for (size_t i = 0; i < desc.attachments.size(); ++i) {
        auto& attachmentDesc = desc.attachments[i];
        auto& attachmentState = attachmentStates[i];

        attachmentState.blendEnable = attachmentDesc.blendEnable;
        attachmentState.colorWriteMask = attachmentDesc.colorWriteMask; // TODO: Support other write masks
        // TODO: Support other blend factors and operations
    }

    colorBlendInfo.pAttachments = attachmentStates.data();
    colorBlendInfo.blendConstants[0] = desc.blendConstants[0];
    colorBlendInfo.blendConstants[1] = desc.blendConstants[1];
    colorBlendInfo.blendConstants[2] = desc.blendConstants[2];
    colorBlendInfo.blendConstants[3] = desc.blendConstants[3];
}

VulkanRHIColorBlendState::~VulkanRHIColorBlendState()
{
}

// ----------------------------
// 深度模板状态 Vulkan派生
// ----------------------------
VulkanRHIDepthStencilState::VulkanRHIDepthStencilState(VulkanDevice* device, const RHIDepthStencilStateDesc& desc) : RHIDepthStencilState(desc), Device(device)
{
    VkDevice vkDevice = Device->GetDevice();
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = desc.depthTestEnable;
    depthStencilInfo.depthWriteEnable = desc.depthWriteEnable;
    // TODO: map compare operation
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;
}

VulkanRHIDepthStencilState::~VulkanRHIDepthStencilState()
{
}


VulkanFence::VulkanFence(VulkanDevice* device)
{
	Device = device;
	VkDevice vkDevice = Device->GetDevice();
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // 初始状态为已信号
	if (vkCreateFence(vkDevice, &fenceInfo, nullptr, &Fence) != VK_SUCCESS) {

	}
}

// ----------------------------
// Fence
// ----------------------------
bool VulkanFence::IsSignaled() const
{
    if (!Fence || !Device) return false;
    VkResult result = vkGetFenceStatus(Device->GetDevice(), Fence);
    return result == VK_SUCCESS;
}

}