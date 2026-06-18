#pragma once
#include "RHIDefine.h"
#include "RHIResource.h"  // Assuming this includes necessary types like FRHIResource, etc.
#include <atomic>

namespace RHI {


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
	EQueueType QueueTypeBefore = EQueueType::Graphics;
	EQueueType QueueTypeAfter = EQueueType::Graphics;
	EResourceTransitionFlags Flags = EResourceTransitionFlags::None;

	RHITransitionInfo() = default;

	RHITransitionInfo(
		class RHITexture* InTexture,
		ERHIResourceAccess InPreviousState,
		ERHIResourceAccess InNewState,
		EQueueType InQueueTypeBefore = EQueueType::Graphics,
		EQueueType InQueueTypeAfter = EQueueType::Graphics,
		EResourceTransitionFlags InFlags = EResourceTransitionFlags::None,
		uint32_t InMipIndex = kAllSubresources,
		uint32_t InArraySlice = kAllSubresources,
		uint32_t InPlaneSlice = kAllSubresources)
		: RHISubresourceRange(InMipIndex, InArraySlice, InPlaneSlice)
		, Texture(InTexture)
		, Type(EType::Texture)
		, AccessBefore(InPreviousState)
		, AccessAfter(InNewState)
		, QueueTypeBefore(InQueueTypeBefore)
        , QueueTypeAfter(InQueueTypeAfter)
		, Flags(InFlags)
	{}

	RHITransitionInfo(class RHIUnorderedAccessView* InUAV, ERHIResourceAccess InPreviousState, ERHIResourceAccess InNewState, EQueueType InQueueTypeBefore = EQueueType::Graphics,EQueueType InQueueTypeAfter = EQueueType::Graphics, EResourceTransitionFlags InFlags = EResourceTransitionFlags::None)
		: UAV(InUAV)
		, Type(EType::UAV)
		, AccessBefore(InPreviousState)
		, AccessAfter(InNewState)
		, QueueTypeBefore(InQueueTypeBefore)
		, QueueTypeAfter(InQueueTypeAfter)
		, Flags(InFlags)
	{}

	RHITransitionInfo(class RHIBuffer* InRHIBuffer, ERHIResourceAccess InPreviousState, ERHIResourceAccess InNewState, EQueueType InQueueTypeBefore = EQueueType::Graphics, EQueueType InQueueTypeAfter = EQueueType::Graphics, EResourceTransitionFlags InFlags = EResourceTransitionFlags::None)
		: Buffer(InRHIBuffer)
		, Type(EType::Buffer)
		, AccessBefore(InPreviousState)
		, AccessAfter(InNewState)
		, QueueTypeBefore(InQueueTypeBefore)
		, QueueTypeAfter(InQueueTypeAfter)
		, Flags(InFlags)
	{}

	RHITransitionInfo(class RHIRayTracingAccelerationStructure* InBVH, ERHIResourceAccess InPreviousState, ERHIResourceAccess InNewState, EQueueType InQueueTypeBefore = EQueueType::Graphics, EQueueType InQueueTypeAfter = EQueueType::Graphics, EResourceTransitionFlags InFlags = EResourceTransitionFlags::None)
		: BVH(InBVH)
		, Type(EType::BVH)
		, AccessBefore(InPreviousState)
		, AccessAfter(InNewState)
		, QueueTypeBefore(InQueueTypeBefore)
		, QueueTypeAfter(InQueueTypeAfter)
		, Flags(InFlags)
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
	RHITransitionCreateInfo(
		ERHITransitionCreateFlags InFlags = ERHITransitionCreateFlags::None,
		std::vector<RHITransitionInfo> InTransitionInfos = {})
		: Flags(InFlags)
		, TransitionInfos(std::move(InTransitionInfos))
	{}

	ERHITransitionCreateFlags Flags = ERHITransitionCreateFlags::None;
	std::vector<RHITransitionInfo> TransitionInfos;
};

// ȫ�ֱ������� RHI �ڳ�ʼ��ʱ���
extern RHI_API uint32_t G_RHITransition_TotalSize;
extern RHI_API uint32_t G_RHITransition_PrivateDataOffset;

struct RHI_API RHITransition
{
public:
	// ��ֹ�������ƶ�
	RHITransition(const RHITransition&) = delete;
	RHITransition& operator=(const RHITransition&) = delete;

	// ֻ���ض��� Factory (RHI/RDG) �ܵ��ù���
	RHITransition()
		: bPendingBegin(true)
		, bPendingEnd(true)
	{
	}

	// ��ȡ˽�����ݵ�Ψһ��ȫ���
	template <typename T>
	inline T* GetPrivateData() const
	{
		// ֱ�Ӹ��ݳ�ʼ��ʱ����õ�ƫ������ת���������
		return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + G_RHITransition_PrivateDataOffset);
	}

	// ���� true ��ʾ��ǰ�߳��Ǹý׶����һ������ģ����𴥷����� Barrier
	inline bool MarkBegin() const
	{
		return bPendingBegin.exchange(false, std::memory_order_acq_rel);
	}

	inline bool MarkEnd() const
	{
		return bPendingEnd.exchange(false, std::memory_order_acq_rel);
	}

private:
	// 单次 Begin/End 触发标记
	mutable std::atomic<bool> bPendingBegin;
	mutable std::atomic<bool> bPendingEnd;
};

} // namespace RHI


