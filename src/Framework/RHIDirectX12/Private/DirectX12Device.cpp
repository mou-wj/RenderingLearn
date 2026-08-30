#include "DirectX12Device.h"

#if defined(_WIN32)
#include <d3d12sdklayers.h>
#include <iostream>
#endif

namespace RHIDirectX12
{
    DirectX12Device::~DirectX12Device()
    {
        Shutdown();
    }

    bool DirectX12Device::Init()
    {
        if (bValid)
        {
            return true;
        }

#if !defined(_WIN32)
        return false;
#else
#if defined(DEBUG_INFO)
        {
            Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
            }
        }
#endif

        if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&Factory))))
        {
            return false;
        }

        Microsoft::WRL::ComPtr<IDXGIAdapter1> candidate;
        for (UINT adapterIndex = 0; Factory->EnumAdapters1(adapterIndex, &candidate) != DXGI_ERROR_NOT_FOUND; ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc{};
            candidate->GetDesc1(&desc);
            if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
            {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(candidate.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&NativeDevice))))
            {
                Adapter = candidate;
                break;
            }
        }

        if (!NativeDevice)
        {
            return false;
        }

        D3D12_COMMAND_QUEUE_DESC graphicsQueueDesc{};
        graphicsQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        graphicsQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        graphicsQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        if (FAILED(NativeDevice->CreateCommandQueue(&graphicsQueueDesc, IID_PPV_ARGS(&GraphicsCommandQueue))))
        {
            Shutdown();
            return false;
        }

        D3D12_COMMAND_QUEUE_DESC computeQueueDesc{};
        computeQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        computeQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        computeQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        if (FAILED(NativeDevice->CreateCommandQueue(&computeQueueDesc, IID_PPV_ARGS(&ComputeCommandQueue))))
        {
            // Some systems may not expose dedicated compute queue; fallback to graphics queue.
            ComputeCommandQueue = GraphicsCommandQueue;
        }

        D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc{};
        cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        cbvSrvUavHeapDesc.NumDescriptors = 8192;
        cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        cbvSrvUavHeapDesc.NodeMask = 0;
        if (FAILED(NativeDevice->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&CbvSrvUavHeap))))
        {
            Shutdown();
            return false;
        }

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.NumDescriptors = 2048;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;
        if (FAILED(NativeDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&RtvHeap))))
        {
            Shutdown();
            return false;
        }

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.NumDescriptors = 2048;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NodeMask = 0;
        if (FAILED(NativeDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&DsvHeap))))
        {
            Shutdown();
            return false;
        }

        CbvSrvUavDescriptorSize = NativeDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        CbvSrvUavDescriptorCapacity = cbvSrvUavHeapDesc.NumDescriptors;
        CbvSrvUavDescriptorAllocated = 0;
        RtvDescriptorSize = NativeDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        RtvDescriptorCapacity = rtvHeapDesc.NumDescriptors;
        RtvDescriptorAllocated = 0;
        DsvDescriptorSize = NativeDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        DsvDescriptorCapacity = dsvHeapDesc.NumDescriptors;
        DsvDescriptorAllocated = 0;

        bValid = true;
        return true;
#endif
    }

    void DirectX12Device::Shutdown()
    {
#if defined(_WIN32)
        CbvSrvUavDescriptorAllocated = 0;
        CbvSrvUavDescriptorCapacity = 0;
        CbvSrvUavDescriptorSize = 0;
        RtvDescriptorAllocated = 0;
        RtvDescriptorCapacity = 0;
        RtvDescriptorSize = 0;
        DsvDescriptorAllocated = 0;
        DsvDescriptorCapacity = 0;
        DsvDescriptorSize = 0;
        DsvHeap.Reset();
        RtvHeap.Reset();
        CbvSrvUavHeap.Reset();
        ComputeCommandQueue.Reset();
        GraphicsCommandQueue.Reset();
        NativeDevice.Reset();
        Adapter.Reset();
        Factory.Reset();
#endif
        bValid = false;
    }

#if defined(_WIN32)
    bool DirectX12Device::CreateCommittedResource(
        const D3D12_HEAP_PROPERTIES& heapProperties,
        D3D12_HEAP_FLAGS heapFlags,
        const D3D12_RESOURCE_DESC& resourceDesc,
        D3D12_RESOURCE_STATES initialState,
        const D3D12_CLEAR_VALUE* clearValue,
        Microsoft::WRL::ComPtr<ID3D12Resource>& outResource) const
    {
        if (!NativeDevice)
        {
            return false;
        }

        outResource.Reset();
        return SUCCEEDED(NativeDevice->CreateCommittedResource(
            &heapProperties,
            heapFlags,
            &resourceDesc,
            initialState,
            clearValue,
            IID_PPV_ARGS(&outResource)));
    }

    bool DirectX12Device::AllocateCbvSrvUavDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE& outHandle, uint32_t count)
    {
        std::lock_guard<std::mutex> lock(CbvSrvUavMutex);

        if (!CbvSrvUavHeap || count == 0)
        {
            return false;
        }

        if (CbvSrvUavDescriptorAllocated + count > CbvSrvUavDescriptorCapacity)
        {
            return false;
        }

        outHandle = CbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
        outHandle.ptr += static_cast<SIZE_T>(CbvSrvUavDescriptorAllocated) * static_cast<SIZE_T>(CbvSrvUavDescriptorSize);
        CbvSrvUavDescriptorAllocated += count;
        return true;
    }

    bool DirectX12Device::AllocateRenderTargetDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE& outHandle, uint32_t count)
    {
        std::lock_guard<std::mutex> lock(RtvMutex);

        if (!RtvHeap || count == 0)
        {
            return false;
        }

        if (RtvDescriptorAllocated + count > RtvDescriptorCapacity)
        {
            return false;
        }

        outHandle = RtvHeap->GetCPUDescriptorHandleForHeapStart();
        outHandle.ptr += static_cast<SIZE_T>(RtvDescriptorAllocated) * static_cast<SIZE_T>(RtvDescriptorSize);
        RtvDescriptorAllocated += count;
        return true;
    }

    bool DirectX12Device::AllocateDepthStencilDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE& outHandle, uint32_t count)
    {
        std::lock_guard<std::mutex> lock(DsvMutex);

        if (!DsvHeap || count == 0)
        {
            return false;
        }

        if (DsvDescriptorAllocated + count > DsvDescriptorCapacity)
        {
            return false;
        }

        outHandle = DsvHeap->GetCPUDescriptorHandleForHeapStart();
        outHandle.ptr += static_cast<SIZE_T>(DsvDescriptorAllocated) * static_cast<SIZE_T>(DsvDescriptorSize);
        DsvDescriptorAllocated += count;
        return true;
    }
#endif
}
