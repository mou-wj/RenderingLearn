#pragma once
#include "RHIDefine.h"
#include "RHIResource.h"  // Assuming this includes necessary types like FRHIResource, etc.
#include <atomic>

namespace RHI {

struct RHI_API RHISubresourceRange
{
	static const uint32_t kDepthPlaneSlice = 0;
	static const uint32_t kStencilPlaneSlice = 1;
	static const uint32_t kAllSubresources = 0xFFFFFFFF;

	uint32_t MipIndex = kAllSubresources;
	uint32_t ArraySlice = kAllSubresources;
	uint32_t PlaneSlice = kAllSubresources;

	RHISubresourceRange() = default;

	RHISubresourceRange(
		uint32_t InMipIndex,
		uint32_t InArraySlice,
		uint32_t InPlaneSlice)
		: MipIndex(InMipIndex)
		, ArraySlice(InArraySlice)
		, PlaneSlice(InPlaneSlice)
	{}

	inline bool IsAllMips() const
	{
		return MipIndex == kAllSubresources;
	}

	inline bool IsAllArraySlices() const
	{
		return ArraySlice == kAllSubresources;
	}

	inline bool IsAllPlaneSlices() const
	{
		return PlaneSlice == kAllSubresources;
	}

	inline bool IsWholeResource() const
	{
		return IsAllMips() && IsAllArraySlices() && IsAllPlaneSlices();
	}

	inline bool IgnoreDepthPlane() const
	{
		return PlaneSlice == kStencilPlaneSlice;
	}

	inline bool IgnoreStencilPlane() const
	{
		return PlaneSlice == kDepthPlaneSlice;
	}

	inline bool operator == (RHISubresourceRange const& RHS) const
	{
		return MipIndex == RHS.MipIndex
			&& ArraySlice == RHS.ArraySlice
			&& PlaneSlice == RHS.PlaneSlice;
	}

	inline bool operator != (RHISubresourceRange const& RHS) const
	{
		return !(*this == RHS);
	}
};

struct RHI_API RHITransitionInfo : public RHISubresourceRange
{
	union
	{
		class RHIResource* Resource = nullptr;
		class RHIViewableResource* ViewableResource;
		class RHITexture* Texture;
		class RHIBuffer* Buffer;
		class RHIUnorderedAccessView* UAV;
		class RHIRayTracingAccelerationStructure* BVH;
	};
	enum class EType {
		Unknown,
		Texture,
		Buffer,
		UAV,
		BVH
	};
	EType Type = EType::Unknown;

	ERHIResourceAccess AccessBefore = ERHIResourceAccess::Unknown;
	ERHIResourceAccess AccessAfter = ERHIResourceAccess::Unknown;
	EResourceTransitionFlags Flags = EResourceTransitionFlags::None;

	RHITransitionInfo() = default;

	RHITransitionInfo(
		class RHITexture* InTexture,
		ERHIResourceAccess InPreviousState,
		ERHIResourceAccess InNewState,
		EResourceTransitionFlags InFlags = EResourceTransitionFlags::None,
		uint32_t InMipIndex = kAllSubresources,
		uint32_t InArraySlice = kAllSubresources,
		uint32_t InPlaneSlice = kAllSubresources)
		: RHISubresourceRange(InMipIndex, InArraySlice, InPlaneSlice)
		, Texture(InTexture)
		, Type(EType::Texture)
		, AccessBefore(InPreviousState)
		, AccessAfter(InNewState)
		, Flags(InFlags)
	{}

	RHITransitionInfo(class RHIUnorderedAccessView* InUAV, ERHIResourceAccess InPreviousState, ERHIResourceAccess InNewState, EResourceTransitionFlags InFlags = EResourceTransitionFlags::None)
		: UAV(InUAV)
		, Type(EType::UAV)
		, AccessBefore(InPreviousState)
		, AccessAfter(InNewState)
		, Flags(InFlags)
	{}

	RHITransitionInfo(class RHIBuffer* InRHIBuffer, ERHIResourceAccess InPreviousState, ERHIResourceAccess InNewState, EResourceTransitionFlags InFlags = EResourceTransitionFlags::None)
		: Buffer(InRHIBuffer)
		, Type(EType::Buffer)
		, AccessBefore(InPreviousState)
		, AccessAfter(InNewState)
		, Flags(InFlags)
	{}

	RHITransitionInfo(class RHIRayTracingAccelerationStructure* InBVH, ERHIResourceAccess InPreviousState, ERHIResourceAccess InNewState, EResourceTransitionFlags InFlags = EResourceTransitionFlags::None)
		: BVH(InBVH)
		, Type(EType::BVH)
		, AccessBefore(InPreviousState)
		, AccessAfter(InNewState)
		, Flags(InFlags)
	{}

	RHITransitionInfo(class RHITexture* InTexture, ERHIResourceAccess InNewState)
		: Texture(InTexture)
		, Type(EType::Texture)
		, AccessAfter(InNewState)
	{}

	RHITransitionInfo(class RHIUnorderedAccessView* InUAV, ERHIResourceAccess InNewState)
		: UAV(InUAV)
		, Type(EType::UAV)
		, AccessAfter(InNewState)
	{}

	RHITransitionInfo(class RHIBuffer* InRHIBuffer, ERHIResourceAccess InNewState)
		: Buffer(InRHIBuffer)
		, Type(EType::Buffer)
		, AccessAfter(InNewState)
	{}

	inline bool operator == (RHITransitionInfo const& RHS) const
	{
		return Resource == RHS.Resource
			&& Type == RHS.Type
			&& AccessBefore == RHS.AccessBefore
			&& AccessAfter == RHS.AccessAfter
			&& Flags == RHS.Flags
			&& RHISubresourceRange::operator==(RHS);
	}

	inline bool operator != (RHITransitionInfo const& RHS) const
	{
		return !(*this == RHS);
	}
};


struct RHI_API RHITransitionCreateInfo
{
	RHITransitionCreateInfo() = default;

	RHITransitionCreateInfo(
		ERHIPipeline InSrcPipelines,
		ERHIPipeline InDstPipelines,
		ERHITransitionCreateFlags InFlags = ERHITransitionCreateFlags::None,
		std::vector<RHITransitionInfo> InTransitionInfos = {})
		: SrcPipelines(InSrcPipelines)
		, DstPipelines(InDstPipelines)
		, Flags(InFlags)
		, TransitionInfos(std::move(InTransitionInfos))
	{}

	ERHIPipeline SrcPipelines = ERHIPipeline::None;
	ERHIPipeline DstPipelines = ERHIPipeline::None;
	ERHITransitionCreateFlags Flags = ERHITransitionCreateFlags::None;
	std::vector<RHITransitionInfo> TransitionInfos;
};

struct RHI_API RHITrackedAccessInfo
{
	RHITrackedAccessInfo() = default;

	RHITrackedAccessInfo(RHIViewableResource* InResource, ERHIResourceAccess InAccess)
		: Resource(InResource)
		, Access(InAccess)
	{}

	RHIViewableResource* Resource = nullptr;
	ERHIResourceAccess Access = ERHIResourceAccess::Unknown;
};

// 全局变量，由 RHI 在初始化时填充
extern RHI_API uint32_t G_RHITransition_TotalSize;
extern RHI_API uint32_t G_RHITransition_PrivateDataOffset;

struct RHI_API RHITransition
{
public:
	// 禁止拷贝和移动
	RHITransition(const RHITransition&) = delete;
	RHITransition& operator=(const RHITransition&) = delete;

	// 只有特定的 Factory (RHI/RDG) 能调用构造
	RHITransition(uint32_t SrcPipelines = static_cast<uint32_t>(ERHIPipeline::Graphics), uint32_t DstPipelines = static_cast<uint32_t>(ERHIPipeline::Graphics))
		: PendingBegin(SrcPipelines)
		, PendingEnd(DstPipelines)
	{
	}

	// 获取私有数据的唯一安全入口
	template <typename T>
	inline T* GetPrivateData() const
	{
		// 直接根据初始化时计算好的偏移量跳转，性能最高
		return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + G_RHITransition_PrivateDataOffset);
	}

	// 返回 true 表示当前线程是该阶段最后一个到达的，负责触发物理 Barrier
	inline bool MarkBegin(uint32_t PipelineBit) const
	{
		uint32_t Previous = PendingBegin.fetch_and(~PipelineBit, std::memory_order_acq_rel);
		return (Previous == PipelineBit);
	}

	inline bool MarkEnd(uint32_t PipelineBit) const
	{
		uint32_t Previous = PendingEnd.fetch_and(~PipelineBit, std::memory_order_acq_rel);
		return (Previous == PipelineBit);
	}

private:
	// 使用 32 位掩码以支持复杂的异步 Pipeline 组合
	mutable std::atomic<uint32_t> PendingBegin;
	mutable std::atomic<uint32_t> PendingEnd;
};

} // namespace RHI


