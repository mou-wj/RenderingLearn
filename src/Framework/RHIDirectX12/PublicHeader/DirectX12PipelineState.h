#pragma once

#include "RHIResource.h"
#include <memory>

#if defined(_WIN32)
#include <d3d12.h>
#include <wrl/client.h>
#endif

namespace RHIDirectX12
{
    class DirectX12Device;

    class DirectX12GraphicsPipelineState : public RHI::RHIGraphicsPipelineState
    {
    public:
        DirectX12GraphicsPipelineState(DirectX12Device* inDevice, const RHI::RHIGraphicsPipelineStateDesc& desc);
        ~DirectX12GraphicsPipelineState() override = default;

        DirectX12Device* GetDevice() const { return Device; }

#if defined(_WIN32)
        ID3D12RootSignature* GetRootSignature() const { return RootSignature.Get(); }
        ID3D12PipelineState* GetPipelineState() const { return PipelineState.Get(); }
#endif

    private:
        DirectX12Device* Device = nullptr;

#if defined(_WIN32)
        Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;
#endif
    };
    using DirectX12GraphicsPipelineStateSP = std::shared_ptr<DirectX12GraphicsPipelineState>;

    class DirectX12ComputePipelineState : public RHI::RHIComputePipelineState
    {
    public:
        DirectX12ComputePipelineState(DirectX12Device* inDevice, const RHI::RHIComputePipelineStateDesc& desc);
        ~DirectX12ComputePipelineState() override = default;

        DirectX12Device* GetDevice() const { return Device; }

#if defined(_WIN32)
        ID3D12RootSignature* GetRootSignature() const { return RootSignature.Get(); }
        ID3D12PipelineState* GetPipelineState() const { return PipelineState.Get(); }
#endif

    private:
        DirectX12Device* Device = nullptr;

#if defined(_WIN32)
        Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState;
#endif
    };
    using DirectX12ComputePipelineStateSP = std::shared_ptr<DirectX12ComputePipelineState>;
}
