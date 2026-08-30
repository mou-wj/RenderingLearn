#include "DirectX12Queue.h"
#include "DirectX12Context.h"
#include "DirectX12Device.h"

#if defined(_WIN32)
#include <cassert>
#endif

namespace RHIDirectX12
{
    DirectX12SyncPoint::DirectX12SyncPoint(RHI::EQueueType inType, DirectX12Device* inDevice, ID3D12CommandQueue* inQueue)
        : Device(inDevice)
    {
        Type = inType;

#if defined(_WIN32)
        Queue = inQueue;
        if (Device && Device->GetNativeDevice())
        {
            const HRESULT hr = Device->GetNativeDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
            if (SUCCEEDED(hr))
            {
                FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            }
        }
#endif
    }

    uint64_t DirectX12SyncPoint::GetCurrentValue()
    {
#if defined(_WIN32)
        if (Fence)
        {
            const uint64_t gpuValue = Fence->GetCompletedValue();
            uint64_t cachedValue = TimelineValue.load(std::memory_order_acquire);
            while (gpuValue > cachedValue && !TimelineValue.compare_exchange_weak(cachedValue, gpuValue, std::memory_order_release, std::memory_order_relaxed))
            {
            }
        }
#endif
        return TimelineValue.load(std::memory_order_acquire);
    }

    void DirectX12SyncPoint::Wait(uint64_t value, uint64_t timeoutNS)
    {
#if !defined(_WIN32)
        (void)value;
        (void)timeoutNS;
#else
        if (!Fence || !FenceEvent)
        {
            return;
        }

        if (Fence->GetCompletedValue() >= value)
        {
            return;
        }

        if (FAILED(Fence->SetEventOnCompletion(value, FenceEvent)))
        {
            return;
        }

        DWORD timeoutMS = INFINITE;
        if (timeoutNS != UINT64_MAX)
        {
            timeoutMS = static_cast<DWORD>(timeoutNS / 1000000ull);
        }
        WaitForSingleObject(FenceEvent, timeoutMS);
#endif
    }

    uint64_t DirectX12SyncPoint::Signal()
    {
#if !defined(_WIN32)
        return TimelineValue.fetch_add(1, std::memory_order_acq_rel) + 1;
#else
        const uint64_t signalValue = TimelineValue.fetch_add(1, std::memory_order_acq_rel) + 1;
        if (Queue && Fence)
        {
            Queue->Signal(Fence.Get(), signalValue);
        }
        return signalValue;
#endif
    }

    DirectX12Queue::DirectX12Queue(RHI::EQueueType type, DirectX12Device* inDevice, ID3D12CommandQueue* inQueue)
        : Device(inDevice)
#if defined(_WIN32)
        , NativeQueue(inQueue)
#endif
        , QueueType(type)
        , SyncPoint(type, inDevice, inQueue)
    {
    }

    RHI::EQueueType DirectX12Queue::GetType() const
    {
        return QueueType;
    }

    RHI::RHIContextBase* DirectX12Queue::AcquireCommandContext()
    {
        if (QueueType == RHI::EQueueType::Compute)
        {
            return new DirectX12ComputeContext(this);
        }

        return new DirectX12GraphicContext(this);
    }

    RHI::RHIContextBase* DirectX12Queue::ReleaseCommandContext(RHI::RHIContextBase* context)
    {
        delete context;
        return nullptr;
    }

    uint64_t DirectX12Queue::ExecuteContext(RHI::RHIContextBase* context)
    {
        if (context)
        {
            context->End();
        }

#if defined(_WIN32)
        if (NativeQueue)
        {
            auto* dxContext = dynamic_cast<DirectX12CommandContext*>(context);
            if (dxContext && dxContext->GetNativeCommandList())
            {
                ID3D12CommandList* commandLists[] = { dxContext->GetNativeCommandList() };
                NativeQueue->ExecuteCommandLists(1, commandLists);
            }
        }
#endif

        RHI::RHIFence fence{};
        fence.QueueType = QueueType;
        fence.Value = SyncPoint.Signal();
        return fence.Value;
    }

    uint64_t DirectX12Queue::ExecuteContext(const std::vector<RHI::RHIContextBase*>& cmds, const std::vector<RHI::RHIWaitInfo>& waitInfos)
    {
        for (const auto& waitInfo : waitInfos)
        {
            if (waitInfo.SyncPoint)
            {
                waitInfo.SyncPoint->Wait(waitInfo.Value);
            }
        }

        for (auto* cmd : cmds)
        {
            if (cmd)
            {
                cmd->End();
            }
        }

#if defined(_WIN32)
        if (NativeQueue)
        {
            std::vector<ID3D12CommandList*> commandLists;
            commandLists.reserve(cmds.size());
            for (auto* cmd : cmds)
            {
                auto* dxContext = dynamic_cast<DirectX12CommandContext*>(cmd);
                if (dxContext && dxContext->GetNativeCommandList())
                {
                    commandLists.push_back(dxContext->GetNativeCommandList());
                }
            }

            if (!commandLists.empty())
            {
                NativeQueue->ExecuteCommandLists(static_cast<UINT>(commandLists.size()), commandLists.data());
            }
        }
#endif

        RHI::RHIFence fence{};
        fence.QueueType = QueueType;
        fence.Value = SyncPoint.Signal();
        return fence.Value;
    }

    void DirectX12Queue::WaitValue(uint64_t fenceValue)
    {
        SyncPoint.Wait(fenceValue);
    }

    void DirectX12Queue::WaitIdle()
    {
        const uint64_t value = SyncPoint.Signal();
        SyncPoint.Wait(value);
    }

    uint64_t DirectX12Queue::GetCurrentTimelineValue()
    {
        return SyncPoint.GetCurrentValue();
    }

    RHI::RHISyncPoint* DirectX12Queue::GetSyncPoint()
    {
        return &SyncPoint;
    }

    DirectX12PresentExecutor::DirectX12PresentExecutor(DirectX12Queue* queue)
        : Queue(queue)
    {
    }

    void DirectX12PresentExecutor::Present(RHI::RHISwapchain* swapchain, const RHI::RHIWaitInfo& waitInfo)
    {
        (void)swapchain;

        if (waitInfo.SyncPoint)
        {
            waitInfo.SyncPoint->Wait(waitInfo.Value);
        }

        if (Queue)
        {
            Queue->WaitIdle();
        }
    }
}
