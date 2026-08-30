#include "DirectX12Context.h"
#include "DirectX12Device.h"
#include "DirectX12PipelineState.h"
#include "DirectX12Queue.h"
#include "DirectX12Resource.h"

#if defined(_WIN32)
#include <algorithm>
#endif

namespace RHIDirectX12
{
#if defined(_WIN32)
    namespace
    {
        D3D12_COMMAND_LIST_TYPE ToCommandListType(RHI::EQueueType queueType)
        {
            switch (queueType)
            {
            case RHI::EQueueType::Compute:
                return D3D12_COMMAND_LIST_TYPE_COMPUTE;
            case RHI::EQueueType::Graphics:
            default:
                return D3D12_COMMAND_LIST_TYPE_DIRECT;
            }
        }

        D3D_PRIMITIVE_TOPOLOGY ToPrimitiveTopology(RHI::EPrimitiveTopology topology)
        {
            switch (topology)
            {
            case RHI::EPrimitiveTopology::PointList:
                return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            case RHI::EPrimitiveTopology::LineList:
                return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            case RHI::EPrimitiveTopology::LineStrip:
                return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case RHI::EPrimitiveTopology::TriangleStrip:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            case RHI::EPrimitiveTopology::LineListWithAdjacency:
                return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
            case RHI::EPrimitiveTopology::LineStripWithAdjacency:
                return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
            case RHI::EPrimitiveTopology::TriangleListWithAdjacency:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
            case RHI::EPrimitiveTopology::TriangleStripWithAdjacency:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
            case RHI::EPrimitiveTopology::PatchList_1:
                return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
            case RHI::EPrimitiveTopology::PatchList_2:
                return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
            case RHI::EPrimitiveTopology::PatchList_3:
                return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
            case RHI::EPrimitiveTopology::PatchList_4:
                return D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
            case RHI::EPrimitiveTopology::TriangleList:
            case RHI::EPrimitiveTopology::TriangleFan:
            case RHI::EPrimitiveTopology::Unknown:
            default:
                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            }
        }
    }
#endif

    DirectX12CommandContext::DirectX12CommandContext(DirectX12Queue* inQueue)
        : Queue(inQueue)
    {
    }

    void DirectX12CommandContext::Begin()
    {
#if defined(_WIN32)
        if (!Queue)
        {
            return;
        }

        DirectX12Device* device = Queue->GetDevice();
        if (!device || !device->GetNativeDevice())
        {
            return;
        }

        const D3D12_COMMAND_LIST_TYPE listType = ToCommandListType(Queue->GetType());
        if (!CommandAllocator)
        {
            if (FAILED(device->GetNativeDevice()->CreateCommandAllocator(listType, IID_PPV_ARGS(&CommandAllocator))))
            {
                return;
            }
        }

        if (!CommandList)
        {
            if (FAILED(device->GetNativeDevice()->CreateCommandList(0, listType, CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&CommandList))))
            {
                return;
            }
        }
        else
        {
            if (FAILED(CommandAllocator->Reset()))
            {
                return;
            }

            if (FAILED(CommandList->Reset(CommandAllocator.Get(), nullptr)))
            {
                return;
            }
        }

        bRecording = true;
#endif
    }

    void DirectX12CommandContext::End()
    {
#if defined(_WIN32)
        if (!bRecording || !CommandList)
        {
            return;
        }

        if (SUCCEEDED(CommandList->Close()))
        {
            bRecording = false;
        }
#endif
    }

    void DirectX12CommandContext::BeginTransitions(std::vector<const RHI::RHITransition*> transitions)
    {
        (void)transitions;
    }

    void DirectX12CommandContext::EndTransitions(std::vector<const RHI::RHITransition*> transitions)
    {
        (void)transitions;
    }

    void DirectX12CommandContext::CopyTexture(RHI::RHITexture* src, RHI::RHITexture* dst, const RHI::RHICopyTextureDesc& copyDesc)
    {
#if defined(_WIN32)
        auto* srcTexture = dynamic_cast<DirectX12Texture*>(src);
        auto* dstTexture = dynamic_cast<DirectX12Texture*>(dst);
        if (!srcTexture || !dstTexture || !CommandList)
        {
            return;
        }

        const auto& srcDesc = srcTexture->GetDesc();
        const auto& dstDesc = dstTexture->GetDesc();
        if (copyDesc.LayerCount == 0 || copyDesc.SrcRegion.Width == 0 || copyDesc.SrcRegion.Height == 0 || copyDesc.DstRegion.Width == 0 || copyDesc.DstRegion.Height == 0)
        {
            return;
        }

        D3D12_RESOURCE_BARRIER barriers[2]{};
        barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barriers[0].Transition.pResource = srcTexture->GetNativeResource();
        barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
        barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barriers[1].Transition.pResource = dstTexture->GetNativeResource();
        barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        CommandList->ResourceBarrier(2, barriers);

        D3D12_TEXTURE_COPY_LOCATION srcLocation{};
        srcLocation.pResource = srcTexture->GetNativeResource();
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLocation.SubresourceIndex = copyDesc.SrcMipIndex;

        D3D12_TEXTURE_COPY_LOCATION dstLocation{};
        dstLocation.pResource = dstTexture->GetNativeResource();
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = copyDesc.DstMipIndex;

        D3D12_BOX srcBox{};
        srcBox.left = copyDesc.SrcRegion.OffsetX;
        srcBox.top = copyDesc.SrcRegion.OffsetY;
        srcBox.front = copyDesc.SrcRegion.OffsetZ;
        srcBox.right = copyDesc.SrcRegion.OffsetX + static_cast<LONG>(copyDesc.SrcRegion.Width);
        srcBox.bottom = copyDesc.SrcRegion.OffsetY + static_cast<LONG>(copyDesc.SrcRegion.Height);
        srcBox.back = copyDesc.SrcRegion.OffsetZ + static_cast<LONG>(copyDesc.SrcRegion.Depth);

        D3D12_BOX dstBox{};
        dstBox.left = copyDesc.DstRegion.OffsetX;
        dstBox.top = copyDesc.DstRegion.OffsetY;
        dstBox.front = copyDesc.DstRegion.OffsetZ;
        dstBox.right = copyDesc.DstRegion.OffsetX + static_cast<LONG>(copyDesc.DstRegion.Width);
        dstBox.bottom = copyDesc.DstRegion.OffsetY + static_cast<LONG>(copyDesc.DstRegion.Height);
        dstBox.back = copyDesc.DstRegion.OffsetZ + static_cast<LONG>(copyDesc.DstRegion.Depth);

        CommandList->CopyTextureRegion(&dstLocation, static_cast<UINT>(copyDesc.DstRegion.OffsetX), static_cast<UINT>(copyDesc.DstRegion.OffsetY), static_cast<UINT>(copyDesc.DstRegion.OffsetZ),
            &srcLocation, &srcBox);

        D3D12_RESOURCE_BARRIER restoreBarriers[2]{};
        restoreBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        restoreBarriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        restoreBarriers[0].Transition.pResource = srcTexture->GetNativeResource();
        restoreBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
        restoreBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
        restoreBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        restoreBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        restoreBarriers[1].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        restoreBarriers[1].Transition.pResource = dstTexture->GetNativeResource();
        restoreBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        restoreBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
        restoreBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        CommandList->ResourceBarrier(2, restoreBarriers);
#else
        (void)src;
        (void)dst;
        (void)copyDesc;
#endif
    }

    void DirectX12CommandContext::BlitTexture(RHI::RHITexture* src, RHI::RHITexture* dst, const RHI::RHIBlitTextureDesc& blitDesc)
    {
#if defined(_WIN32)
        auto* srcTexture = dynamic_cast<DirectX12Texture*>(src);
        auto* dstTexture = dynamic_cast<DirectX12Texture*>(dst);
        if (!srcTexture || !dstTexture || !CommandList)
        {
            return;
        }

        RHI::RHICopyTextureDesc copyDesc{};
        copyDesc.SrcMipIndex = blitDesc.SrcMipIndex;
        copyDesc.DstMipIndex = blitDesc.DstMipIndex;
        copyDesc.SrcArraySlice = blitDesc.SrcArraySlice;
        copyDesc.DstArraySlice = blitDesc.DstArraySlice;
        copyDesc.LayerCount = blitDesc.LayerCount;
        copyDesc.SrcRegion = blitDesc.SrcRegion;
        copyDesc.DstRegion = blitDesc.DstRegion;
        CopyTexture(srcTexture, dstTexture, copyDesc);
#else
        (void)src;
        (void)dst;
        (void)blitDesc;
#endif
    }

    DirectX12ComputeContext::DirectX12ComputeContext(DirectX12Queue* inQueue)
        : DirectX12CommandContext(inQueue)
    {
    }

    void DirectX12ComputeContext::SetComputePipelineState(RHI::RHIComputePipelineState* pipelineState)
    {
#if defined(_WIN32)
        auto* dxPipeline = dynamic_cast<DirectX12ComputePipelineState*>(pipelineState);
        if (!dxPipeline || !CommandList)
        {
            return;
        }

        CommandList->SetComputeRootSignature(dxPipeline->GetRootSignature());
        CommandList->SetPipelineState(dxPipeline->GetPipelineState());
#else
        (void)pipelineState;
#endif
    }

    void DirectX12ComputeContext::SetBatchedShaderParameters(RHI::RHIComputeShader* shader, const RHI::RHIBatchedShaderParameters& parameter)
    {
        (void)shader;
        (void)parameter;
    }

    void DirectX12ComputeContext::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
#if defined(_WIN32)
        if (!CommandList)
        {
            return;
        }

        CommandList->Dispatch(groupCountX, groupCountY, groupCountZ);
#else
        (void)groupCountX;
        (void)groupCountY;
        (void)groupCountZ;
#endif
    }

    DirectX12GraphicContext::DirectX12GraphicContext(DirectX12Queue* inQueue)
        : DirectX12CommandContext(inQueue)
    {
    }

    void DirectX12GraphicContext::SetBatchedShaderParameters(RHI::RHIGraphicShader* shader, const RHI::RHIBatchedShaderParameters& parameter)
    {
        (void)shader;
        (void)parameter;
    }

    void DirectX12GraphicContext::SetStreamSource(uint32_t streamIndex, RHI::RHIBuffer* vertexBuffer, uint32_t offset)
    {
#if defined(_WIN32)
        auto* dxBuffer = dynamic_cast<DirectX12Buffer*>(vertexBuffer);
        if (!dxBuffer || !dxBuffer->GetNativeResource() || !CommandList)
        {
            return;
        }

        const auto& bufferDesc = dxBuffer->GetDesc();
        const uint32_t stride = bufferDesc.Stride;
        if (stride == 0 || offset >= bufferDesc.Size)
        {
            return;
        }

        D3D12_VERTEX_BUFFER_VIEW vbView{};
        vbView.BufferLocation = dxBuffer->GetNativeResource()->GetGPUVirtualAddress() + offset;
        vbView.SizeInBytes = static_cast<UINT>(bufferDesc.Size - offset);
        vbView.StrideInBytes = stride;
        CommandList->IASetVertexBuffers(streamIndex, 1, &vbView);
#else
        (void)streamIndex;
        (void)vertexBuffer;
        (void)offset;
#endif
    }

    void DirectX12GraphicContext::SetGraphicPipelineState(RHI::RHIGraphicsPipelineState* pipelineState)
    {
#if defined(_WIN32)
        auto* dxPipeline = dynamic_cast<DirectX12GraphicsPipelineState*>(pipelineState);
        if (!dxPipeline || !CommandList)
        {
            return;
        }

        CommandList->SetGraphicsRootSignature(dxPipeline->GetRootSignature());
        CommandList->SetPipelineState(dxPipeline->GetPipelineState());
        CommandList->IASetPrimitiveTopology(ToPrimitiveTopology(dxPipeline->GetDesc().primitiveTopology));
#else
        (void)pipelineState;
#endif
    }

    void DirectX12GraphicContext::SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth)
    {
#if defined(_WIN32)
        if (!CommandList)
        {
            return;
        }

        D3D12_VIEWPORT viewport{};
        viewport.TopLeftX = x;
        viewport.TopLeftY = y;
        viewport.Width = w;
        viewport.Height = h;
        viewport.MinDepth = minDepth;
        viewport.MaxDepth = maxDepth;
        CommandList->RSSetViewports(1, &viewport);
#else
        (void)x;
        (void)y;
        (void)w;
        (void)h;
        (void)minDepth;
        (void)maxDepth;
#endif
    }

    void DirectX12GraphicContext::SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h)
    {
#if defined(_WIN32)
        if (!CommandList)
        {
            return;
        }

        D3D12_RECT rect{};
        rect.left = x;
        rect.top = y;
        rect.right = x + static_cast<LONG>(w);
        rect.bottom = y + static_cast<LONG>(h);
        CommandList->RSSetScissorRects(1, &rect);
#else
        (void)x;
        (void)y;
        (void)w;
        (void)h;
#endif
    }

    void DirectX12GraphicContext::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
#if defined(_WIN32)
        if (!CommandList)
        {
            return;
        }

        CommandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
#else
        (void)vertexCount;
        (void)instanceCount;
        (void)firstVertex;
        (void)firstInstance;
#endif
    }

    void DirectX12GraphicContext::DrawIndexed(RHI::RHIBuffer* indexBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {
#if defined(_WIN32)
        auto* dxIndexBuffer = dynamic_cast<DirectX12Buffer*>(indexBuffer);
        if (!dxIndexBuffer || !dxIndexBuffer->GetNativeResource() || !CommandList)
        {
            return;
        }

        const auto& desc = dxIndexBuffer->GetDesc();
        D3D12_INDEX_BUFFER_VIEW ibView{};
        ibView.BufferLocation = dxIndexBuffer->GetNativeResource()->GetGPUVirtualAddress();
        ibView.SizeInBytes = static_cast<UINT>(desc.Size);
        ibView.Format = desc.Stride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        CommandList->IASetIndexBuffer(&ibView);
        CommandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
#else
        (void)indexBuffer;
        (void)indexCount;
        (void)instanceCount;
        (void)firstIndex;
        (void)vertexOffset;
        (void)firstInstance;
#endif
    }

    void DirectX12GraphicContext::BeginRenderPass(const RHI::RHIRenderPassInfo& renderPassInfo)
    {
#if defined(_WIN32)
        if (!CommandList || !Queue || !Queue->GetDevice())
        {
            return;
        }

        DirectX12Device* device = Queue->GetDevice();
        const auto& targets = renderPassInfo.RenderTargets;

        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
        rtvHandles.reserve(targets.NumColorAttachments);

        for (uint8_t i = 0; i < targets.NumColorAttachments; ++i)
        {
            const auto& colorAttachment = targets.ColorAttachments[i];
            auto* texture = dynamic_cast<DirectX12Texture*>(colorAttachment.Texture);
            if (!texture || !texture->GetNativeResource())
            {
                continue;
            }

            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
            if (!device->AllocateRenderTargetDescriptor(rtvHandle))
            {
                continue;
            }

            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
            rtvDesc.Format = texture->GetNativeFormat();
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = colorAttachment.MipIndex;
            device->GetNativeDevice()->CreateRenderTargetView(texture->GetNativeResource(), &rtvDesc, rtvHandle);

            const auto loadOp = static_cast<RHI::ERenderTargetLoadOp>(static_cast<uint8_t>(colorAttachment.Actions) >> static_cast<uint8_t>(RHI::ERenderTargetActions::LoadOpShift));
            if (loadOp == RHI::ERenderTargetLoadOp::Clear)
            {
                const float* color = colorAttachment.ClearBinding.Color;
                CommandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
            }

            rtvHandles.push_back(rtvHandle);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{};
        bool hasDepthStencil = false;
        if (targets.DepthStencil.Texture)
        {
            auto* depthTexture = dynamic_cast<DirectX12Texture*>(targets.DepthStencil.Texture);
            if (depthTexture && depthTexture->GetNativeResource())
            {
                if (device->AllocateDepthStencilDescriptor(dsvHandle))
                {
                    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
                    dsvDesc.Format = depthTexture->GetNativeFormat();
                    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                    dsvDesc.Texture2D.MipSlice = targets.DepthStencil.MipIndex;
                    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
                    device->GetNativeDevice()->CreateDepthStencilView(depthTexture->GetNativeResource(), &dsvDesc, dsvHandle);
                    hasDepthStencil = true;

                    const auto depthLoadOp = static_cast<RHI::ERenderTargetLoadOp>(static_cast<uint8_t>(targets.DepthStencil.Actions) >> static_cast<uint8_t>(RHI::ERenderTargetActions::LoadOpShift));
                    if (depthLoadOp == RHI::ERenderTargetLoadOp::Clear)
                    {
                        CommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                            targets.DepthStencil.ClearBinding.Depth, targets.DepthStencil.ClearBinding.Stencil, 0, nullptr);
                    }
                }
            }
        }

        if (renderPassInfo.RenderArea.Width > 0 && renderPassInfo.RenderArea.Height > 0)
        {
            D3D12_VIEWPORT viewport{};
            viewport.TopLeftX = static_cast<float>(renderPassInfo.RenderArea.X);
            viewport.TopLeftY = static_cast<float>(renderPassInfo.RenderArea.Y);
            viewport.Width = static_cast<float>(renderPassInfo.RenderArea.Width);
            viewport.Height = static_cast<float>(renderPassInfo.RenderArea.Height);
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;
            CommandList->RSSetViewports(1, &viewport);

            D3D12_RECT scissor{};
            scissor.left = renderPassInfo.RenderArea.X;
            scissor.top = renderPassInfo.RenderArea.Y;
            scissor.right = renderPassInfo.RenderArea.X + static_cast<LONG>(renderPassInfo.RenderArea.Width);
            scissor.bottom = renderPassInfo.RenderArea.Y + static_cast<LONG>(renderPassInfo.RenderArea.Height);
            CommandList->RSSetScissorRects(1, &scissor);
        }

        CommandList->OMSetRenderTargets(
            static_cast<UINT>(rtvHandles.size()),
            rtvHandles.empty() ? nullptr : rtvHandles.data(),
            FALSE,
            hasDepthStencil ? &dsvHandle : nullptr);
#else
        (void)renderPassInfo;
#endif
    }

    void DirectX12GraphicContext::EndRenderPass()
    {
#if defined(_WIN32)
        if (!CommandList)
        {
            return;
        }

        CommandList->OMSetRenderTargets(0, nullptr, FALSE, nullptr);
#endif
    }

    void DirectX12GraphicContext::SetBatchedShaderParameters(RHI::RHIRayTracingShader* shader, const RHI::RHIBatchedShaderParameters& parameter)
    {
        (void)shader;
        (void)parameter;
    }

    void DirectX12GraphicContext::SetRayTracingPipelineState(RHI::RHIRayTracingPipelineState* pipelineState)
    {
        (void)pipelineState;
    }

    void DirectX12GraphicContext::BuildAccelerationStructure(RHI::RHIRayTracingAccelerationStructure* accelerationStructure)
    {
        (void)accelerationStructure;
    }

    void DirectX12GraphicContext::UpdateAccelerationStructure(RHI::RHIRayTracingAccelerationStructure* accelerationStructure)
    {
        (void)accelerationStructure;
    }

    void DirectX12GraphicContext::TraceRays(uint32_t width, uint32_t height, uint32_t depth)
    {
        (void)width;
        (void)height;
        (void)depth;
    }
}
