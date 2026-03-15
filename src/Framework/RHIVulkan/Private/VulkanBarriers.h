#pragma once
#include "vulkan/vulkan.h"
#include <unordered_map>
namespace RHIVulkan {
	class VulkanCommandBuffer;
	class VulkanImageLayout
	{
	public:
		VulkanImageLayout() = default;
		VulkanImageLayout(VkImageLayout initial,
			uint32_t numMips,
			uint32_t numLayers,
			VkImageAspectFlags aspectMask);


		bool IsUniform() const;
		VkImageLayout Get(uint32_t mip, uint32_t layer) const;


		void Set(VkImageLayout layout, const VkImageSubresourceRange& range);
		void CollapseIfPossible();


		uint32_t GetNumMips() const { return NumMips; }
		uint32_t GetNumLayers() const { return NumLayers; }
		VkImageLayout GetMainLayout() const { return MainLayout; }


	private:
		uint32_t NumMips = 0;
		uint32_t NumLayers = 0;


		VkImageLayout MainLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		std::vector<VkImageLayout> SubresourceLayouts;


		uint32_t Index(uint32_t mip, uint32_t layer) const;
	};



	class VulkanImageLayoutManager
	{
	public:
		explicit VulkanImageLayoutManager(VulkanImageLayoutManager* fallback = nullptr);
		void Clear() { Layouts.clear(); }

		const VulkanImageLayout* Get(VkImage image) const;
		VulkanImageLayout* GetOrCreate(VkImage image,
			uint32_t mips,
			uint32_t layers,
			VkImageLayout initial,
			VkImageAspectFlags aspect);


		void Set(VkImage image, const VulkanImageLayout& layout);
		void Set(VkImage image,
			VkImageLayout layout,
			const VkImageSubresourceRange& range);


		void TransferTo(VulkanImageLayoutManager& dst);
		void Remove(VkImage image);


	private:
		std::unordered_map<VkImage, VulkanImageLayout> Layouts;
		VulkanImageLayoutManager* Fallback = nullptr;
	};



	class VulkanImageBarrierBuilder
	{
	public:
		void TransitionLayout(
			VkImage image,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			const VkImageSubresourceRange& range);


		void TransitionAccess(
			VkImage image,
			VkImageLayout layout,
			VkAccessFlags srcAccess,
			VkAccessFlags dstAccess,
			const VkImageSubresourceRange& range);


		void TransitionLayoutAndAccess(
			VkImage image,
			VkImageLayout oldLayout,
			VkImageLayout newLayout,
			VkAccessFlags srcAccess,
			VkAccessFlags dstAccess,
			const VkImageSubresourceRange& range);


		void FullImageAccessBarrier(
			VkImage image,
			VkImageLayout layout,
			VkAccessFlags srcAccess,
			VkAccessFlags dstAccess,
			VkImageAspectFlags aspect);


		void Execute(VulkanCommandBuffer* cmd);

		static VkImageSubresourceRange MakeSubresourceRange(VkImageAspectFlags aspectMask, uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount);

	private:
		void Push(const VkImageMemoryBarrier& barrier);
		std::vector<VkImageMemoryBarrier> ImageBarriers;
		std::vector<VkMemoryBarrier> MemoryBarriers;
		std::vector<VkBufferMemoryBarrier> BufferBarriers;
	};

}