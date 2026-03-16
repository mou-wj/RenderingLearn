#include "VulkanResource.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanPlatformSurport.h"
#include "VulkanRHIUtils.h"
#include "VulkanFuncWrapper.h"
#include "Log.h"
#include "VulkanSync.h"
#include "VulkanCommandContex.h"
#include "ThreadInfo.h"
#include "VulkanQueue.h"
using namespace Core;
namespace RHIVulkan{

    VkSharingMode DetermineImageSharingMode(ERHITextureCreateFlags flags, VulkanDevice* device, std::vector<uint32_t>& outQueueFamilyIndices)
    {
        outQueueFamilyIndices.clear();

        // Graphics 队列总是需要
        outQueueFamilyIndices.push_back(device->GetGraphicsQueueFamilyIndex());

        // 如果有 Compute 标志，加入 Compute 队列
        if (EnumHasAnyFlags(flags, ERHITextureCreateFlags::UAV) &&
            device->GetComputeQueueFamilyIndex() != device->GetGraphicsQueueFamilyIndex())
        {
            outQueueFamilyIndices.push_back(device->GetComputeQueueFamilyIndex());
        }

        // 如果有 TransferSrc 或 TransferDst，加入 Transfer 队列
        if (EnumHasAnyFlags(flags, ERHITextureCreateFlags::CopySrc) || EnumHasAnyFlags(flags, ERHITextureCreateFlags::CopyDest)  &&
            device->GetTransferQueueFamilyIndex() != device->GetGraphicsQueueFamilyIndex() &&
            (outQueueFamilyIndices.empty() || device->GetTransferQueueFamilyIndex() != outQueueFamilyIndices[0]))
        {
            outQueueFamilyIndices.push_back(device->GetTransferQueueFamilyIndex());
        }

        // 如果有 Present 标志，加入 Present 队列
        if (EnumHasAnyFlags(flags, ERHITextureCreateFlags::Presentable) &&
            device->GetPresentQueueFamilyIndex() != device->GetGraphicsQueueFamilyIndex() &&
            (std::find(outQueueFamilyIndices.begin(), outQueueFamilyIndices.end(),
                device->GetPresentQueueFamilyIndex()) == outQueueFamilyIndices.end()))
        {
            outQueueFamilyIndices.push_back(device->GetPresentQueueFamilyIndex());
        }

        // 根据队列族数量选择 sharing mode
        if (outQueueFamilyIndices.size() <= 1)
        {
            return VK_SHARING_MODE_EXCLUSIVE;
        }
        else
        {
            return VK_SHARING_MODE_CONCURRENT;
        }
    }
    VkSharingMode DetermineBufferSharingMode(ERHIBufferUsageFlags flags, VulkanDevice* device, std::vector<uint32_t>& outQueueFamilyIndices)
    {
        outQueueFamilyIndices.clear();

        auto AddQueue = [&](uint32_t queueFamily)
            {
                if (std::find(outQueueFamilyIndices.begin(), outQueueFamilyIndices.end(), queueFamily)
                    == outQueueFamilyIndices.end())
                {
                    outQueueFamilyIndices.push_back(queueFamily);
                }
            };

        // Graphics 队列默认需要（大多数 buffer 都会被 graphics 使用）
        AddQueue(device->GetGraphicsQueueFamilyIndex());

        // UAV / SRV 可能在 Compute 队列使用
        if (EnumHasAnyFlags(flags, ERHIBufferUsageFlags::UnorderedAccess) ||
            EnumHasAnyFlags(flags, ERHIBufferUsageFlags::ShaderResource))
        {
            if (device->GetComputeQueueFamilyIndex() != device->GetGraphicsQueueFamilyIndex())
            {
                AddQueue(device->GetComputeQueueFamilyIndex());
            }
        }

        // Transfer 队列
        if (EnumHasAnyFlags(flags, ERHIBufferUsageFlags::TransferSrc) ||
            EnumHasAnyFlags(flags, ERHIBufferUsageFlags::TransferDst))
        {
            if (device->GetTransferQueueFamilyIndex() != device->GetGraphicsQueueFamilyIndex())
            {
                AddQueue(device->GetTransferQueueFamilyIndex());
            }
        }

        // MapRead / MapWrite 不影响 queue family（CPU 访问）

        if (outQueueFamilyIndices.size() <= 1)
        {
            return VK_SHARING_MODE_EXCLUSIVE;
        }
        else
        {
            return VK_SHARING_MODE_CONCURRENT;
        }
    }

// Vulkan Texture
VulkanTexture::VulkanTexture(VulkanDevice* device, const RHITextureDesc& desc)
    : RHITexture(desc), Device(device) {
    VkDevice vkDevice = Device->GetHandle();
    VulkanMemoryManager* memoryManager = Device->GetMemoryManager();
    Format = TransformFormatFrom(desc.Format);
	ImageAspectFlags = GetImageAspectFlags(Format);
    // Create VkImage
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = TransformImageTypeFrom(desc.Type); // TODO: Support other image types
    imageInfo.format = Format; // TODO: Support other formats
    imageInfo.extent.width = desc.Width;
    imageInfo.extent.height = desc.Height;
    imageInfo.extent.depth = desc.Depth;
    imageInfo.mipLevels = desc.MipLevels;
    imageInfo.arrayLayers = desc.ArraySize;
    imageInfo.samples = TransformSampleCountFrom(desc.SampleCount);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = TransformTextureUsageFlagsFrom(desc.Usage); // TODO: Support other usages
	std::vector<uint32_t> queueFamilyIndices;
    imageInfo.sharingMode = DetermineImageSharingMode( desc.Usage, Device, queueFamilyIndices);
	imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
	imageInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    CreateImage(vkDevice, &imageInfo, &Image);

    // Get memory requirements
    VkMemoryRequirements memRequirements;
    GetImageMemoryRequirements(vkDevice, Image, &memRequirements);

    // Allocate memory
    if (!memoryManager->Allocate(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Allocation)) {
        DestroyImage(vkDevice, Image);
    }

    // Bind memory to image
    BindImageMemory(vkDevice, Image, Allocation.GetMemory(), Allocation.GetOffset());

    // Create VkImageView
    bool viewSuccess = DefaltView.Create(
        device,                                // fvulkan_device& device
        Image,                                 // VkImage
        TransformViewTypeFrom(desc.Type),      // VkImageViewType
        GetImageAspectFlags(TransformFormatFrom(desc.Format)), // VkImageAspectFlags
        TransformFormatFrom(desc.Format),       // VkFormat
        0,                                      // first mip level
        desc.MipLevels,                         // number of mips
        0,                                      // base array layer
        desc.ArraySize,                          // number of array layers
        true,                                   // use identity swizzle
        imageInfo.usage
    );

    if (!viewSuccess) {
        DestroyImage(vkDevice, Image);
        memoryManager->Free(Allocation);
    }

    DefaultLayout = DetermineDefaultLayout(desc.Usage);

    auto vkCommandContex = VulkanCommandContext::CastFrom(device->GetGlobalCommandContext());
    auto commandList = vkCommandContex->GetCommandList();

    //转换imagelayout
    if (!Core::IsInRenderThread()) {
        InitialImageState(vkCommandContex, DefaultLayout);
    }
    else {
        commandList.AddCommand<VulkanCommandInitializeImageState>(this, DefaultLayout,true);
    }

}

VulkanTexture::VulkanTexture(VulkanDevice* device, const RHITextureDesc& desc, VkImage image)
    : RHITexture(desc), Device(device), Image(image)
{
    owner = false;
    VkDevice vkDevice = Device->GetHandle();
    Format = TransformFormatFrom(desc.Format);
    ImageAspectFlags = GetImageAspectFlags(Format);
    auto usage = TransformTextureUsageFlagsFrom(desc.Usage);
    // Create VkImageView
    DefaltView.Create(device,                                // fvulkan_device& device
        Image,                                 // VkImage
        TransformViewTypeFrom(desc.Type),      // VkImageViewType
        GetImageAspectFlags(TransformFormatFrom(desc.Format)), // VkImageAspectFlags
        Format,       // VkFormat
        0,                                      // first mip level
        desc.MipLevels,                         // number of mips
        0,                                      // base array layer
        desc.ArraySize,                          // number of array layers
        true,                                    // use identity swizzle)
        usage
    );

    DefaultLayout = DetermineDefaultLayout(desc.Usage);
    auto vkCommandContex = VulkanCommandContext::CastFrom(device->GetGlobalCommandContext());
    auto commandList = vkCommandContex->GetCommandList();

    //转换imagelayout
    if (!Core::IsInRenderThread()) {
        InitialImageState(vkCommandContex, DefaultLayout);
    }
    else {
        commandList.AddCommand<VulkanCommandInitializeImageState>(this, DefaultLayout, false);
    }
}

VulkanTexture::~VulkanTexture() {
    VkDevice device = Device->GetHandle();
    VulkanMemoryManager* memoryManager = Device->GetMemoryManager();

	DefaltView.Destroy(Device);

    if (Image != VK_NULL_HANDLE && owner) {
        vkDestroyImage(device, Image, nullptr);
    }

    memoryManager->Free(Allocation);
}

void VulkanTexture::InitialImageState(VulkanCommandContext* context, VkImageLayout layout)
{
    VulkanCommandContext* vulkanContex = context;
    // 2. 获取一个可用命令缓冲区
    VulkanCommandBuffer* cmdBuffer = vulkanContex->GetCommandBufferManager()->BeginUploadCommandBuffer();
    auto vkFormat = TransformFormatFrom(Desc.Format);
    auto imageFlags = GetImageAspectFlags(vkFormat);
    //更新tracked image layout
    auto cmdImageLayoutMgr = cmdBuffer->GetImageLayoutManager();
    cmdImageLayoutMgr->GetOrCreate(Image, Desc.MipLevels, Desc.ArraySize, layout, imageFlags);
    VulkanImageBarrierBuilder barrierBuilder;
    VkImageSubresourceRange transientRegion = VulkanImageBarrierBuilder::MakeSubresourceRange(imageFlags,
        0,
        Desc.MipLevels, 
        0,
        Desc.ArraySize);
    barrierBuilder.TransitionLayout(
        Image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        layout,
        transientRegion
    );
    barrierBuilder.Execute(cmdBuffer);
    vulkanContex->GetCommandBufferManager()->EndAndSubmitUploadCommandBuffer(cmdBuffer);

}

VkImageLayout VulkanTexture::DetermineDefaultLayout(ERHITextureCreateFlags Flags)
{
    // 1️⃣ 优先考虑 Presentable (swapchain)
    if (EnumHasAnyFlags(Flags, ERHITextureCreateFlags::Presentable))
    {
        // Swapchain image 创建后默认 undefined
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    // 2️⃣ DepthStencil 图像
    if (EnumHasAnyFlags(Flags, ERHITextureCreateFlags::DepthStencil))
    {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    // 3️⃣ RenderTarget 图像
    if (EnumHasAnyFlags(Flags, ERHITextureCreateFlags::RenderTarget))
    {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    // 4️⃣ UAV / CPU 可访问 / CopySrc / CopyDst
    if (EnumHasAnyFlags(Flags,
        ERHITextureCreateFlags::UAV |
        ERHITextureCreateFlags::CPUReadback |
        ERHITextureCreateFlags::CopySrc |
        ERHITextureCreateFlags::CopyDest))
    {
        return VK_IMAGE_LAYOUT_GENERAL;
    }

    // 5️⃣ 默认 layout
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

// Vulkan Buffer
VulkanBuffer::VulkanBuffer(VulkanDevice* device, const RHIBufferDesc& desc)
    : RHIBuffer(desc), Device(device) {
    VkDevice vkDevice = Device->GetHandle();
    VulkanMemoryManager* memoryManager = Device->GetMemoryManager();

    // Create VkBuffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    Size = Desc.Size;
    bufferInfo.size = Desc.Size;
	auto usageFlags = TransformBufferUsageFlagsFrom(Desc.Usage);
    bufferInfo.usage = usageFlags; // TODO: Support other usages
    std::vector<uint32_t> queueFamilyIndices;
    bufferInfo.sharingMode = DetermineBufferSharingMode(desc.Usage, Device, queueFamilyIndices);
    bufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
    bufferInfo.pQueueFamilyIndices = queueFamilyIndices.data();

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
    VkDevice device = Device->GetHandle();
    VulkanMemoryManager* memoryManager = Device->GetMemoryManager();

    if (Buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, Buffer, nullptr);
    }

    memoryManager->Free(Allocation);
}


VulkanShaderResourceView::VulkanShaderResourceView(VulkanDevice* device,
    RHIViewableResource* Resource,
    const RHITexSRVCreateInfo& SRVInfo)
    : RHIShaderResourceView(Resource)
{
    ResourcePtr = Resource;
    Device = device;
    ResourceType = EResourceType::Texture;

    CreateTextureView(SRVInfo);
}

VulkanShaderResourceView::VulkanShaderResourceView(
    VulkanDevice* device,
    RHIViewableResource* Resource,
    const RHIBufferSRVCreateInfo& SRVInfo)
    : RHIShaderResourceView(Resource)
{
    ResourcePtr = Resource;
    Device = device;
    ResourceType = EResourceType::Buffer;

    CreateBufferView(SRVInfo);
}

void VulkanShaderResourceView::CreateTextureView(
    const RHITexSRVCreateInfo& SRVInfo)
{
    auto* texture = static_cast<VulkanTexture*>(ResourcePtr);

    Format = (SRVInfo.Format == ERHIFormat::Unknown)
        ? texture->GetFormat()
        : TransformFormatFrom(SRVInfo.Format);

    BaseMipLevel = SRVInfo.MostDetailedMip;
    MipLevelCount = SRVInfo.MipLevelCount;
    BaseArrayLayer = SRVInfo.FirstArraySlice;
    LayerCount = SRVInfo.ArraySize;

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture->GetImage();
    viewInfo.viewType =
        (LayerCount > 1) ?
        VK_IMAGE_VIEW_TYPE_2D_ARRAY :
        VK_IMAGE_VIEW_TYPE_2D;

    viewInfo.format = Format;

    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = BaseMipLevel;
    viewInfo.subresourceRange.levelCount = MipLevelCount;
    viewInfo.subresourceRange.baseArrayLayer = BaseArrayLayer;
    viewInfo.subresourceRange.layerCount = LayerCount;

    vkCreateImageView(
        Device->GetHandle(),
        &viewInfo,
        nullptr,
        &ImageView);

    DescriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
}

void VulkanShaderResourceView::CreateBufferView(
    const RHIBufferSRVCreateInfo& SRVInfo)
{
    auto* buffer = static_cast<VulkanBuffer*>(ResourcePtr);

    Format = TransformFormatFrom(SRVInfo.Format);

    VkBufferViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    viewInfo.buffer = buffer->GetHandle();
    viewInfo.format = Format;
    viewInfo.offset = SRVInfo.Offset;
    viewInfo.range =
        (SRVInfo.NumElements * SRVInfo.Stride);

    vkCreateBufferView(
        Device->GetHandle(),
        &viewInfo,
        nullptr,
        &BufferView);

    DescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
}

VulkanShaderResourceView::~VulkanShaderResourceView()
{
    DestroyView();
}

void VulkanShaderResourceView::DestroyView()
{
    if (ImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(
            Device->GetHandle(),
            ImageView,
            nullptr);

        ImageView = VK_NULL_HANDLE;
    }

    if (BufferView != VK_NULL_HANDLE)
    {
        vkDestroyBufferView(
            Device->GetHandle(),
            BufferView,
            nullptr);

        BufferView = VK_NULL_HANDLE;
    }
}

VulkanUnorderedAccessView::VulkanUnorderedAccessView(
    VulkanDevice* device,
    RHIViewableResource* Resource,
    const RHITexUAVCreateInfo& UAVInfo)
    : RHIUnorderedAccessView(Resource)
{
    ResourcePtr = Resource;
    Device = device;
    ResourceType = EResourceType::Texture;

    CreateTextureView(UAVInfo);
}

VulkanUnorderedAccessView::VulkanUnorderedAccessView(
    VulkanDevice* device,
    RHIViewableResource* Resource,
    const RHIBufferUAVCreateInfo& UAVInfo)
    : RHIUnorderedAccessView(Resource)
{
    ResourcePtr = Resource;
    Device = device;
    ResourceType = EResourceType::Buffer;

    CreateBufferView(UAVInfo);
}


void VulkanUnorderedAccessView::CreateTextureView(
    const RHITexUAVCreateInfo& UAVInfo)
{
    auto* texture = static_cast<VulkanTexture*>(ResourcePtr);

    Format = (UAVInfo.Format == ERHIFormat::Unknown)
        ? texture->GetFormat()
        : TransformFormatFrom(UAVInfo.Format);

    BaseMipLevel = UAVInfo.MipSlice;
    MipLevelCount = 1;
    BaseArrayLayer = UAVInfo.FirstArraySlice;
    LayerCount = UAVInfo.ArraySize;

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture->GetImage();
    viewInfo.viewType =
        (LayerCount > 1) ?
        VK_IMAGE_VIEW_TYPE_2D_ARRAY :
        VK_IMAGE_VIEW_TYPE_2D;

    viewInfo.format = Format;

    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = BaseMipLevel;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = BaseArrayLayer;
    viewInfo.subresourceRange.layerCount = LayerCount;

    vkCreateImageView(
        Device->GetHandle(),
        &viewInfo,
        nullptr,
        &ImageView);

    DescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
}

void VulkanUnorderedAccessView::CreateBufferView(
    const RHIBufferUAVCreateInfo& UAVInfo)
{
    auto* buffer = static_cast<VulkanBuffer*>(ResourcePtr);

    Format = TransformFormatFrom(UAVInfo.Format);

    VkBufferViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    viewInfo.buffer = buffer->GetHandle();
    viewInfo.format = Format;
    viewInfo.offset = UAVInfo.Offset;
    viewInfo.range =
        (UAVInfo.NumElements * UAVInfo.Stride);

    vkCreateBufferView(
        Device->GetHandle(),
        &viewInfo,
        nullptr,
        &BufferView);

    DescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
}

VulkanUnorderedAccessView::~VulkanUnorderedAccessView()
{
    DestroyView();
}

void VulkanUnorderedAccessView::DestroyView()
{
    if (ImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(Device->GetHandle(), ImageView, nullptr);
        ImageView = VK_NULL_HANDLE;
    }

    if (BufferView != VK_NULL_HANDLE)
    {
        vkDestroyBufferView(Device->GetHandle(), BufferView, nullptr);
        BufferView = VK_NULL_HANDLE;
    }
}



// Vulkan Viewport (Swapchain)
VulkanViewport::VulkanViewport(VulkanDevice* device, uint32_t width, uint32_t height, void* windowHandle, ERHIFormat format)
    : RHIViewport(), Device(device), WindowHandle(windowHandle),Width(width),Height(height),Format(format) 
{


    if (!WindowHandle)
    {
		LOG_ERROR("VulkanViewport: Invalid window handle");
    }
    CreateSwapchain();
    acquireSemaphores.resize(swapchainImages_.size());
    backBufferRenderDoneSemaphores.resize(swapchainImages_.size());
    for (int i = 0; i < swapchainImages_.size(); i++) {
        backBufferRenderDoneSemaphores[i] = device->GetSemaphoreManager()->Acquire();
        acquireSemaphores[i] = device->GetSemaphoreManager()->Acquire();
    }
}

VulkanViewport::~VulkanViewport() {
    VkDevice device = Device->GetHandle();

    DestroySwapchain();

}

void VulkanViewport::Present(VulkanCommandContext* context, VulkanCommandBuffer* commandBuffer, VulkanQueue* queue, VulkanQueue* presentQueue)
{
    assert(queue == presentQueue);//先假定相等
    //转换布局
    auto backBufferTexture = backBufferTextures[currentBackBufferIndex];
	auto layout = commandBuffer->GetImageLayoutManager()->Get(backBufferTexture->GetImage());
	VulkanImageBarrierBuilder barrierBuilder;
    VkImageSubresourceRange transientRegion = VulkanImageBarrierBuilder::MakeSubresourceRange(backBufferTexture->GetAspectFlags(),
        0,
        1,
        0,
        1);
    barrierBuilder.TransitionLayout(
        backBufferTexture->GetImage(),
        layout->GetMainLayout(),
        backBufferTexture->GetDefaultLayout(),
        transientRegion
    );
    barrierBuilder.Execute(commandBuffer);
    commandBuffer->AddWaitSemaphores(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { acquireSemaphores[currentIndex]});
    commandBuffer->AddSignalSemaphores({ backBufferRenderDoneSemaphores[currentIndex] });
    context->GetCommandBufferManager()->SubmitActiveCommandBuffer();
    Swapchain->Present(presentQueue,backBufferRenderDoneSemaphores[currentIndex]);
    currentIndex++;
	currentIndex %= swapchainImages_.size();
    currentBackBufferIndex = -1;

}

void VulkanViewport::Tick()
{
}

VulkanTextureSP VulkanViewport::GetBackTexture()
{
    if (currentBackBufferIndex != -1) {
        return backBufferTextures[currentBackBufferIndex];
    }
	//这里在获取下一个有效image之前必须要保证currentIndex对应的acquiredSemaphores已经执行完毕，这种必须需要等待fence，但是直接使用commandbuffer的fence会由于fence由其他地方管理而导致这里不能有效等待，所以这里先进行没绘制完所有image后统一等待一次队列，后续再考虑怎么修改 >>  后续可以改成commandbuffer不再单独持有fence，添加commandbuffer的状态管理，改成每次提交命令时由外部传入一个fence，提交后外部负责等待和重置，这样就可以在这里直接等待对应的fence了
    if (currentIndex == 0) {
		Device->GetGraphicsQueue()->WaitIdle();
    }

    Swapchain->AcquireNextImage(acquireSemaphores[currentIndex], &currentBackBufferIndex);
    if (currentBackBufferIndex == -1) {
        return nullptr;
    }
    return backBufferTextures[currentBackBufferIndex];
}

void VulkanViewport::CreateSwapchain()
{
    VulkanSwapchain::SwapchainDesc swapchainDesc = {};
    swapchainDesc.windowHandle = WindowHandle;
    swapchainDesc.width = Width;
    swapchainDesc.height = Height;
    swapchainDesc.preferredFormat = Format;
    Swapchain = new VulkanSwapchain(Device, swapchainDesc, swapchainImages_);

    RHITextureDesc textureDesc;
    textureDesc.Width = Width;                  // 纹理宽度
    textureDesc.Height = Height;                 // 纹理高度
    textureDesc.Depth = 1;                  // 纹理深度（3D纹理）
    textureDesc.MipLevels = 1;              // Mip层级数量
    textureDesc.ArraySize = 1;              // 数组大小
    textureDesc.Format = Format; // 像素格式
    ERHITextureType Type = ERHITextureType::Texture2D;   // 纹理类型
    uint32_t SampleCount = 1;            // 多重采样数量
    uint32_t SampleQuality = 0;          // 多重采样质量
    textureDesc.Usage = ERHITextureCreateFlags::RenderTarget | ERHITextureCreateFlags::Presentable; // 纹理用途
    auto imageCount = swapchainImages_.size();
    for (int i = 0; i < imageCount; i++) {

        auto texture = std::make_shared<VulkanTexture>(Device, textureDesc, swapchainImages_[i]);
        backBufferTextures.push_back(texture);
    }


}

void VulkanViewport::DestroySwapchain()
{
    delete Swapchain;
    backBufferTextures.clear();

}


VulkanVertexDescState::VulkanVertexDescState(VulkanDevice* device, const RHIVertexDescStateDesc& desc): RHIVertexDescState(desc), Device(device)
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
        attributeDescriptions[i].binding = attributeDesc.binding;  // 必须
        attributeDescriptions[i].format = TransformFormatFrom(attributeDesc.format);
        attributeDescriptions[i].offset = attributeDesc.offset;   // 必须
    }
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

}
VulkanVertexDescState::~VulkanVertexDescState()
{

}


// ----------------------------
// 光栅化状态 Vulkan派生
// ----------------------------
VulkanRasterizerState::VulkanRasterizerState(VulkanDevice* device, const RHIRasterizerStateDesc& desc) : RHIRasterizerState(desc), Device(device)
{
    VkDevice vkDevice = Device->GetHandle();
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

VulkanRasterizerState::~VulkanRasterizerState()
{
}

// ----------------------------
// 颜色混合状态 Vulkan派生
// ----------------------------
VulkanColorBlendState::VulkanColorBlendState(VulkanDevice* device, const RHIColorBlendStateDesc& desc) : RHIColorBlendState(desc), Device(device)
{
    VkDevice vkDevice = Device->GetHandle();
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

VulkanColorBlendState::~VulkanColorBlendState()
{
}

// ----------------------------
// 深度模板状态 Vulkan派生
// ----------------------------
VulkanDepthStencilState::VulkanDepthStencilState(VulkanDevice* device, const RHIDepthStencilStateDesc& desc) : RHIDepthStencilState(desc), Device(device)
{
    VkDevice vkDevice = Device->GetHandle();
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = desc.depthTestEnable;
    depthStencilInfo.depthWriteEnable = desc.depthWriteEnable;
    // TODO: map compare operation
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;
}

VulkanDepthStencilState::~VulkanDepthStencilState()
{
}


VulkanFence::VulkanFence(VulkanDevice* device)
{
	Device = device;
	VkDevice vkDevice = Device->GetHandle();
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = 0; // 初始状态为已信号
	if (vkCreateFence(vkDevice, &fenceInfo, nullptr, &Fence) != VK_SUCCESS) {

	}
    bool isSignaled = IsSignaled();
    int a = 10;
}

// ----------------------------
// Fence
// ----------------------------
bool VulkanFence::IsSignaled() const
{
    if (!Fence || !Device) return false;
    VkResult result = vkGetFenceStatus(Device->GetHandle(), Fence);
    return result == VK_SUCCESS;
}

void VulkanFence::Reset()
{
    if (!Fence || !Device) return;
    ResetFences(Device->GetHandle(), 1, &Fence);
}

void VulkanFence::Wait()
{
	if (!Fence || !Device) return;
	WaitForFences(Device->GetHandle(), 1, &Fence, VK_TRUE, UINT64_MAX);
}


VulkanFenceManager::VulkanFenceManager(VulkanDevice* deviceIn)
    : device(deviceIn)
{
}

VulkanFenceManager::~VulkanFenceManager()
{
    allFences.clear();
    availableFences.clear();
    pendingFences.clear();
}

VulkanFence* VulkanFenceManager::AcquireFence()
{
    // 优先复用空闲 fence
    if (!availableFences.empty())
    {
        VulkanFence* fence = availableFences.front();
        availableFences.pop_front();
        //fence->Reset();
        return fence;
    }

    // 没有空闲 fence，创建新的
    auto newFence = std::make_unique<VulkanFence>(device);
    VulkanFence* ptr = newFence.get();
    allFences.push_back(std::move(newFence));
    return ptr;
}

// 提交 fence 后标记为 pending
void VulkanFenceManager::ReleaseFence(VulkanFence* fence)
{
    if (fence)
    {
        pendingFences.push_back(fence);
    }
}

// 检查 fence 是否完成，回收空闲池
void VulkanFenceManager::GarbageCollect()
{
    size_t count = pendingFences.size();
    for (size_t i = 0; i < count; ++i)
    {
        VulkanFence* fence = pendingFences.front();
        pendingFences.pop_front();

        if (fence->IsSignaled())
        {
            // fence 已完成，回收到 available
            availableFences.push_back(fence);
        }
        else
        {
            // fence 还没完成，放回 pending
            pendingFences.push_back(fence);
        }
    }
}


}