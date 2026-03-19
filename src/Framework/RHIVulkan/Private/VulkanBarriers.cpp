#include "VulkanBarriers.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFuncWrapper.h"
#include "RHIDefine.h"
#include "VulkanResource.h"
namespace RHIVulkan {

	static void GetVulkanBarrierMasksByLayout(
		VkImageLayout InLayout,
		VkAccessFlags& OutAccessMask,
		VkPipelineStageFlags& OutStageMask)
	{
		switch (InLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			OutAccessMask = 0;
			OutStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			OutAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			OutStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			OutAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			OutStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			OutAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			OutStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			OutAccessMask = VK_ACCESS_SHADER_READ_BIT;
			// ��������ڶ�������ؽ׶ζ�ȡ��ȡ��������ȫ
			OutStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			OutAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			OutStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;

		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			OutAccessMask = 0;
			OutStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			break;

		default:
			OutAccessMask = 0;
			OutStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			break;
		}
	}
	
	void GetVulkanBarrierInfo(
		bool bIsTexture,
		ERHIResourceAccess InAccess,
		VkImageLayout& OutLayout,
		VkAccessFlags& OutAccessMask,
		VkPipelineStageFlags& OutStageMask)
	{
		OutLayout = VK_IMAGE_LAYOUT_GENERAL;
		OutAccessMask = 0;
		OutStageMask = 0;

		if (InAccess == ERHIResourceAccess::Unknown || InAccess == ERHIResourceAccess::Undefined)
		{
			OutLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			OutAccessMask = 0;
			OutStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			return;
		}

		// --- ֻ����֧ ---
		if (EnumHasAnyFlags(InAccess, ERHIResourceAccess::SRVGraphics))
		{
			OutLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			OutAccessMask |= VK_ACCESS_SHADER_READ_BIT;
			OutStageMask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		if (EnumHasAnyFlags(InAccess, ERHIResourceAccess::SRVCompute))
		{
			OutLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			OutAccessMask |= VK_ACCESS_SHADER_READ_BIT;
			OutStageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}
		if (EnumHasAnyFlags(InAccess, ERHIResourceAccess::CopySrc))
		{
			OutLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			OutAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
			OutStageMask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		if (EnumHasAnyFlags(InAccess, ERHIResourceAccess::DSVRead))
		{
			OutLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			OutAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			OutStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		}

		// --- д���֧ ---
		if (EnumHasAnyFlags(InAccess, ERHIResourceAccess::RTV))
		{
			OutLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			OutAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			OutStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		if (EnumHasAnyFlags(InAccess, ERHIResourceAccess::UAVMask))
		{
			OutLayout = VK_IMAGE_LAYOUT_GENERAL;
			OutAccessMask |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			OutStageMask |= EnumHasAnyFlags(InAccess, ERHIResourceAccess::UAVCompute) ?
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		if (EnumHasAnyFlags(InAccess, ERHIResourceAccess::DSVWrite))
		{
			OutLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			OutAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			OutStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		}
		if (EnumHasAnyFlags(InAccess, ERHIResourceAccess::CopyDest))
		{
			OutLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			OutAccessMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
			OutStageMask |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		}

		// --- Buffer ����״̬�Ƶ� ---
		if (EnumHasAnyFlags(InAccess, ERHIResourceAccess::VertexOrIndexBuffer))
		{
			OutAccessMask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;
			OutStageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
		}
		if (EnumHasAnyFlags(InAccess, ERHIResourceAccess::IndirectArgs))
		{
			OutAccessMask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
			OutStageMask |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
		}
		if (EnumHasAnyFlags(InAccess, ERHIResourceAccess::Present))
		{
			OutLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			OutStageMask |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}

		// ��Է�������Դ������
		if (!bIsTexture)
		{
			OutLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}


	VulkanImageLayout::VulkanImageLayout(VkImageLayout initial,
		uint32_t numMips,
		uint32_t numLayers,
		VkImageAspectFlags)
		: NumMips(numMips)
		, NumLayers(numLayers)
		, MainLayout(initial)
	{

	}


	bool VulkanImageLayout::IsUniform() const
	{
		return SubresourceLayouts.empty();
	}


	uint32_t VulkanImageLayout::Index(uint32_t mip, uint32_t layer) const
	{
		return layer * NumMips + mip;
	}


	VkImageLayout VulkanImageLayout::Get(uint32_t mip, uint32_t layer) const
	{
		if (SubresourceLayouts.empty())
			return MainLayout;


		return SubresourceLayouts[Index(mip, layer)];
	}


	void VulkanImageLayout::Set(VkImageLayout layout, const VkImageSubresourceRange& range)
	{
		uint32_t baseMip = range.baseMipLevel;
		uint32_t mipCount = range.levelCount == VK_REMAINING_MIP_LEVELS
			? NumMips - baseMip
			: range.levelCount;


		uint32_t baseLayer = range.baseArrayLayer;
		uint32_t layerCount = range.layerCount == VK_REMAINING_ARRAY_LAYERS
			? NumLayers - baseLayer
			: range.layerCount;


		if (SubresourceLayouts.empty())
		{
			SubresourceLayouts.resize(NumMips * NumLayers, MainLayout);
		}


		for (uint32_t l = 0; l < layerCount; ++l)
			for (uint32_t m = 0; m < mipCount; ++m)
				SubresourceLayouts[Index(baseMip + m, baseLayer + l)] = layout;


		CollapseIfPossible();
	}


	void VulkanImageLayout::CollapseIfPossible()
	{
		if (SubresourceLayouts.empty())
			return;


		VkImageLayout first = SubresourceLayouts[0];
		for (auto l : SubresourceLayouts)
		{
			if (l != first)
				return;
		}


		MainLayout = first;
		SubresourceLayouts.clear();
	}



	VulkanImageLayoutManager::VulkanImageLayoutManager(VulkanImageLayoutManager* fallback)
		: Fallback(fallback)
	{
	}


	const VulkanImageLayout* VulkanImageLayoutManager::GetFullLayout(VkImage image) const
	{
		auto it = Layouts.find(image);
		if (it != Layouts.end())
			return &it->second;

		return Fallback ? Fallback->GetFullLayout(image) : nullptr;
	}


	const VulkanImageLayout* VulkanImageLayoutManager::GetFullLayout(VulkanTexture* texture) const
	{
		return GetFullLayout(texture->GetImage());
	}


	void VulkanImageLayoutManager::SetFullLayout(VkImage image, const VulkanImageLayout& layout)
	{
		Layouts[image] = layout;
	}


	void VulkanImageLayoutManager::SetFullLayout(VulkanTexture* texture, const VulkanImageLayout& layout)
	{
		SetFullLayout(texture->GetImage(), layout);
	}


	void VulkanImageLayoutManager::SetFullLayout(VulkanTexture* texture, VkImageLayout layout)
	{
		// Create a full layout with the specified layout for all subresources
		const auto& desc = texture->GetDesc();
		VkImageAspectFlags aspect = texture->GetAspectFlags();
		uint32_t numMips = desc.MipLevels;
		uint32_t numLayers = desc.ArraySize;
		VulkanImageLayout fullLayout(layout, numMips, numLayers, aspect);
		SetFullLayout(texture, fullLayout);
	}


	void VulkanImageLayoutManager::SetLayout(VulkanTexture* texture,
		VkImageLayout layout,
		const VkImageSubresourceRange& range)
	{
		auto& imgLayout = Layouts[texture->GetImage()];
		imgLayout.Set(layout, range);
	}


	void VulkanImageLayoutManager::TransferTo(VulkanImageLayoutManager& dst)
	{
		for (auto& it : Layouts)
			dst.Layouts[it.first] = it.second;
	}


	void VulkanImageLayoutManager::Remove(VkImage image)
	{
		Layouts.erase(image);
	}

	void VulkanImageLayoutManager::NotifyDeletedImage(VkImage image)
	{
        Remove(image);
	}

	void VulkanImageBarrierBuilder::Push(const VkImageMemoryBarrier& barrier)
	{
		ImageBarriers.push_back(barrier);
	}

	void VulkanImageBarrierBuilder::TransitionLayout(
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		const VkImageSubresourceRange& range)
	{
		VkImageMemoryBarrier b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		b.image = image;
		b.oldLayout = oldLayout;
		b.newLayout = newLayout;
		b.subresourceRange = range;
		b.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
		b.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
		Push(b);
	}

	void VulkanImageBarrierBuilder::TransitionAccess(
		VkImage image,
		VkImageLayout layout,
		VkAccessFlags srcAccess,
		VkAccessFlags dstAccess,
		const VkImageSubresourceRange& range)
	{
		VkImageMemoryBarrier b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		b.image = image;
		b.oldLayout = layout;
		b.newLayout = layout;
		b.subresourceRange = range;
		b.srcAccessMask = srcAccess;
		b.dstAccessMask = dstAccess;
		Push(b);
	}

	void VulkanImageBarrierBuilder::TransitionLayoutAndAccess(
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkAccessFlags srcAccess,
		VkAccessFlags dstAccess,
		const VkImageSubresourceRange& range)
	{
		VkImageMemoryBarrier b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		b.image = image;
		b.oldLayout = oldLayout;
		b.newLayout = newLayout;
		b.subresourceRange = range;
		b.srcAccessMask = srcAccess;
		b.dstAccessMask = dstAccess;
		Push(b);
	}

	void VulkanImageBarrierBuilder::FullImageAccessBarrier(
		VkImage image,
		VkImageLayout layout,
		VkAccessFlags srcAccess,
		VkAccessFlags dstAccess,
		VkImageAspectFlags aspect)
	{
		VkImageSubresourceRange range{};
		range.aspectMask = aspect;
		range.baseMipLevel = 0;
		range.levelCount = VK_REMAINING_MIP_LEVELS;
		range.baseArrayLayer = 0;
		range.layerCount = VK_REMAINING_ARRAY_LAYERS;

		TransitionAccess(image, layout, srcAccess, dstAccess, range);
	}

	void VulkanImageBarrierBuilder::Execute(
		VulkanCommandBuffer* cmd)
	{
		if (ImageBarriers.empty())
			return;
		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		for (int i = 0; i < ImageBarriers.size(); i++) 
		{
			VkAccessFlags srcAccess;
			VkAccessFlags dstAccess;
			VkPipelineStageFlags srcCurStage;
            VkPipelineStageFlags dstCurStage;
				
			GetVulkanBarrierMasksByLayout(ImageBarriers[i].oldLayout, srcAccess, srcCurStage);
			GetVulkanBarrierMasksByLayout(ImageBarriers[i].newLayout, dstAccess, dstCurStage);
			srcStage |= srcCurStage;
            dstStage |= dstCurStage;
		}
		vkCmdPipelineBarrier(
			cmd->GetHandle(),
			srcStage,
			dstStage,
			0,
			0, nullptr,
			0, nullptr,
			static_cast<uint32_t>(ImageBarriers.size()),
			ImageBarriers.data());
		ImageBarriers.clear();
	}

	VkImageSubresourceRange VulkanImageBarrierBuilder::MakeSubresourceRange(VkImageAspectFlags aspectMask, uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
	{
		VkImageSubresourceRange Range;
		Range.aspectMask = aspectMask;
		Range.baseMipLevel = baseMip;
		Range.levelCount = mipCount;
		Range.baseArrayLayer = baseLayer;
		Range.layerCount = layerCount;
		return Range;
	}

}