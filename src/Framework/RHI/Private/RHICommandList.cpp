#include "RHICommandList.h"
#include "RHICommandContex.h"
#include "RHIApi.h"

namespace RHI {

RHICommandDispatch::RHICommandDispatch(uint32_t x, uint32_t y, uint32_t z)
    : X(x), Y(y), Z(z) {}

void RHICommandDispatch::Execute(RHICommandListBase& cmdList)
{
    auto* computeContext = dynamic_cast<RHIComputeContex*>(cmdList.GetContext());
    if (computeContext)
    {
        computeContext->Dispatch(X, Y, Z);
    }
}

RHICommandDraw::RHICommandDraw(uint32_t v, uint32_t i, uint32_t fv, uint32_t fi)
    : VertexCount(v), InstanceCount(i), FirstVertex(fv), FirstInstance(fi) {}

void RHICommandDraw::Execute(RHICommandListBase& cmdList)
{
    auto* graphicsContext = dynamic_cast<RHIGraphicContex*>(cmdList.GetContext());
    if (graphicsContext)
    {
        graphicsContext->Draw(VertexCount, InstanceCount, FirstVertex, FirstInstance);
    }
}

RHICommandTraceRays::RHICommandTraceRays(uint32_t w, uint32_t h, uint32_t d)
    : Width(w), Height(h), Depth(d) {}

void RHICommandTraceRays::Execute(RHICommandListBase& cmdList)
{
    auto* graphicsContext = dynamic_cast<RHIGraphicContex*>(cmdList.GetContext());
    if (graphicsContext)
    {
        graphicsContext->TraceRays(Width, Height, Depth);
    }
}

RHICommandCopyTexture::RHICommandCopyTexture(RHITexture* src, RHITexture* dst, const RHICopyTextureDesc& copyDesc)
    : Src(src), Dst(dst), CopyDesc(copyDesc)
{
}

void RHICommandCopyTexture::Execute(RHICommandListBase& cmdList)
{
    auto* context = dynamic_cast<RHIContextBase*>(cmdList.GetContext());
    if (context)
    {
        context->CopyTexture(Src, Dst, CopyDesc);
    }
}

RHICommandBlitTexture::RHICommandBlitTexture(RHITexture* src, RHITexture* dst, const RHIBlitTextureDesc& blitDesc)
    : Src(src), Dst(dst), BlitDesc(blitDesc)
{
}

void RHICommandBlitTexture::Execute(RHICommandListBase& cmdList)
{
    auto* context = dynamic_cast<RHIContextBase*>(cmdList.GetContext());
    if (context)
    {
        context->BlitTexture(Src, Dst, BlitDesc);
    }
}
RHICommandUpdateTexture::RHICommandUpdateTexture(RHITexture* texture, const void* data, const RHIUpdateTextureRegion& region)
    : texture(texture), data(data), region(region)
{
}
void RHICommandUpdateTexture::Execute(RHICommandListBase& cmdList)
{
    GRHIApi->UpdateTexture(cmdList,texture, data, region);
}
RHICommandUpdateBuffer::RHICommandUpdateBuffer(RHIBuffer* buffer, const void* data, const RHIBufferRegion& region)
    : buffer(buffer), data(data), region(region)
{
}
void RHICommandUpdateBuffer::Execute(RHICommandListBase& cmdList)
{
    GRHIApi->UpdateBuffer(cmdList, buffer, data, region);
}

RHICommandSetComputePipelineState::RHICommandSetComputePipelineState(RHIComputePipelineState* pipelineState)
    : PipelineState(pipelineState)
{
}

void RHICommandSetComputePipelineState::Execute(RHICommandListBase& cmdList)
{
    auto* computeContext = dynamic_cast<RHIComputeContex*>(cmdList.GetContext());
    if (computeContext)
    {
        computeContext->SetComputePipelineState(PipelineState);
    }
}

RHICommandSetComputeShaderParameters::RHICommandSetComputeShaderParameters(RHIComputeShader* shader, const RHIBatchedShaderParameters& parameters)
    : Shader(shader), Parameters(parameters)
{
}

void RHICommandSetComputeShaderParameters::Execute(RHICommandListBase& cmdList)
{
    auto* computeContext = dynamic_cast<RHIComputeContex*>(cmdList.GetContext());
    if (computeContext)
    {
        computeContext->SetBatchedShaderParameters(Shader, Parameters);
    }
}

RHICommandSetGraphicShaderParameters::RHICommandSetGraphicShaderParameters(RHIGraphicShader* shader, const RHIBatchedShaderParameters& parameters)
    : Shader(shader), Parameters(parameters)
{
}

void RHICommandSetGraphicShaderParameters::Execute(RHICommandListBase& cmdList)
{
    auto* graphicContext = dynamic_cast<RHIGraphicContex*>(cmdList.GetContext());
    if (graphicContext)
    {
        graphicContext->SetBatchedShaderParameters(Shader, Parameters);
    }
}

RHICommandSetStreamSource::RHICommandSetStreamSource(uint32_t streamIndex, RHIBufferSP vertexBuffer, uint32_t offset)
    : StreamIndex(streamIndex), VertexBuffer(std::move(vertexBuffer)), Offset(offset)
{
}

void RHICommandSetStreamSource::Execute(RHICommandListBase& cmdList)
{
    auto* graphicContext = dynamic_cast<RHIGraphicContex*>(cmdList.GetContext());
    if (graphicContext)
    {
        graphicContext->SetStreamSource(StreamIndex, VertexBuffer, Offset);
    }
}

RHICommandSetGraphicPipelineState::RHICommandSetGraphicPipelineState(RHIGraphicsPipelineState* pipelineState)
    : PipelineState(pipelineState)
{
}

void RHICommandSetGraphicPipelineState::Execute(RHICommandListBase& cmdList)
{
    auto* graphicContext = dynamic_cast<RHIGraphicContex*>(cmdList.GetContext());
    if (graphicContext)
    {
        graphicContext->SetGraphicPipelineState(PipelineState);
    }
}

RHICommandSetViewport::RHICommandSetViewport(float x, float y, float w, float h, float minDepth, float maxDepth)
    : X(x), Y(y), W(w), H(h), MinDepth(minDepth), MaxDepth(maxDepth)
{
}

void RHICommandSetViewport::Execute(RHICommandListBase& cmdList)
{
    auto* graphicContext = dynamic_cast<RHIGraphicContex*>(cmdList.GetContext());
    if (graphicContext)
    {
        graphicContext->SetViewport(X, Y, W, H, MinDepth, MaxDepth);
    }
}

RHICommandSetScissor::RHICommandSetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h)
    : X(x), Y(y), W(w), H(h)
{
}

void RHICommandSetScissor::Execute(RHICommandListBase& cmdList)
{
    auto* graphicContext = dynamic_cast<RHIGraphicContex*>(cmdList.GetContext());
    if (graphicContext)
    {
        graphicContext->SetScissor(X, Y, W, H);
    }
}

RHICommandBeginRenderPass::RHICommandBeginRenderPass(const RHIRenderPassInfo& renderPassInfo)
    : RenderPassInfo(renderPassInfo)
{
}

void RHICommandBeginRenderPass::Execute(RHICommandListBase& cmdList)
{
    auto* graphicContext = dynamic_cast<RHIGraphicContex*>(cmdList.GetContext());
    if (graphicContext)
    {
        graphicContext->BeginRenderPass(RenderPassInfo);
    }
}

void RHICommandEndRenderPass::Execute(RHICommandListBase& cmdList)
{
    auto* graphicContext = dynamic_cast<RHIGraphicContex*>(cmdList.GetContext());
    if (graphicContext)
    {
        graphicContext->EndRenderPass();
    }
}

RHICommandSetRayTracingPipelineState::RHICommandSetRayTracingPipelineState(RHIRayTracingPipelineState* pipelineState)
    : PipelineState(pipelineState)
{
}

void RHICommandSetRayTracingPipelineState::Execute(RHICommandListBase& cmdList)
{
    auto* graphicContext = dynamic_cast<RHIGraphicContex*>(cmdList.GetContext());
    if (graphicContext)
    {
        graphicContext->SetRayTracingPipelineState(PipelineState);
    }
}

void RHICommandSetShaderTable::Execute(RHICommandListBase& cmdList)
{
    auto* graphicContext = dynamic_cast<RHIGraphicContex*>(cmdList.GetContext());
    if (graphicContext)
    {
        graphicContext->SetShaderTable();
    }
}

RHICommandListBase::RHICommandListBase(RHIContextBase* context)
    : Context(context)
{
}

RHICommandListBase::~RHICommandListBase() = default;

void RHICommandListBase::ExecuteAll()
{
    for (auto& cmd : commands)
    {
        if (cmd)
        {
            cmd->Execute(*this);
        }
    }
}

void RHICommandListBase::Clear()
{
    commands.clear();
}

void RHICommandListBase::SetImmediate(bool bImmediate)
{
    immediate = bImmediate;
}

bool RHICommandListBase::IsImmediate() const
{
    return immediate;
}

RHIContextBase* RHICommandListBase::GetContext() const
{
    return Context;
}

RHIContextBase* RHICommandListBase::GetCommandContex() const
{
    return Context;
}

void RHICommandListBase::Begin()
{
    if (Context)
    {
        Context->Begin();
    }
}

void RHICommandListBase::End()
{
    if (Context)
    {
        Context->End();
    }
}

void RHICommandListBase::BeginTransitions(std::vector<const RHITransition*> Transitions)
{
    if (Context)
    {
        Context->BeginTransitions(std::move(Transitions));
    }
}

void RHICommandListBase::EndTransitions(std::vector<const RHITransition*> Transitions)
{
    if (Context)
    {
        Context->EndTransitions(std::move(Transitions));
    }
}
void RHICommandListBase::CopyTexture(RHITexture* src, RHITexture* dst, const RHICopyTextureDesc& copyDesc)
{
    AddCommand<RHICommandCopyTexture>(src, dst, copyDesc);
}
void RHICommandListBase::BlitTexture(RHITexture* src, RHITexture* dst, const RHIBlitTextureDesc& blitDesc) {
    AddCommand<RHICommandBlitTexture>(src, dst, blitDesc);
}


void RHICommandListBase::Merge(const RHICommandListBase& other)
{
    commands.insert(commands.end(), other.commands.begin(), other.commands.end());
}



RHIComputeCommandList::RHIComputeCommandList(RHIComputeContex* context)
    : RHICommandListBase(context)
{
}

RHIComputeContex* RHIComputeCommandList::GetComputeContext() const
{
    return dynamic_cast<RHIComputeContex*>(Context);
}

void RHIComputeCommandList::SetComputePipelineState(RHIComputePipelineState* pipelineState)
{
    AddCommand<RHICommandSetComputePipelineState>(pipelineState);
}

void RHIComputeCommandList::SetBatchedShaderParameters(RHIComputeShader* shader, const RHIBatchedShaderParameters& batchedShaderParameter)
{
    AddCommand<RHICommandSetComputeShaderParameters>(shader, batchedShaderParameter);
}

void RHIComputeCommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z)
{
    AddCommand<RHICommandDispatch>(x, y, z);
}

RHIGraphicCommandList::RHIGraphicCommandList(RHIGraphicContex* context)
    : RHICommandListBase(context)
{
}

RHIGraphicContex* RHIGraphicCommandList::GetGraphicContext() const
{
    return dynamic_cast<RHIGraphicContex*>(Context);
}

void RHIGraphicCommandList::SetBatchedShaderParameters(RHIGraphicShader* shader, const RHIBatchedShaderParameters& batchedShaderParameter)
{
    AddCommand<RHICommandSetGraphicShaderParameters>(shader, batchedShaderParameter);
}

void RHIGraphicCommandList::SetStreamSource(uint32_t streamIndex, RHIBufferSP VertexBuffer, uint32_t Offset)
{
    AddCommand<RHICommandSetStreamSource>(streamIndex, std::move(VertexBuffer), Offset);
}

void RHIGraphicCommandList::SetGraphicPipelineState(RHIGraphicsPipelineState* pipelineState)
{
    AddCommand<RHICommandSetGraphicPipelineState>(pipelineState);
}

void RHIGraphicCommandList::SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth)
{
    AddCommand<RHICommandSetViewport>(x, y, w, h, minDepth, maxDepth);
}

void RHIGraphicCommandList::SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h)
{
    AddCommand<RHICommandSetScissor>(x, y, w, h);
}

void RHIGraphicCommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    AddCommand<RHICommandDraw>(vertexCount, instanceCount, firstVertex, firstInstance);
}

void RHIGraphicCommandList::BeginRenderPass(const RHIRenderPassInfo& renderPassInfo)
{
    AddCommand<RHICommandBeginRenderPass>(renderPassInfo);
}

void RHIGraphicCommandList::EndRenderPass()
{
    AddCommand<RHICommandEndRenderPass>();
}

void RHIGraphicCommandList::SetRayTracingPipelineState(RHIRayTracingPipelineState* pipelineState)
{
    AddCommand<RHICommandSetRayTracingPipelineState>(pipelineState);
}

void RHIGraphicCommandList::SetShaderTable()
{
    AddCommand<RHICommandSetShaderTable>();
}

void RHIGraphicCommandList::TraceRays(uint32_t width, uint32_t height, uint32_t depth)
{
    AddCommand<RHICommandTraceRays>(width, height, depth);
}

} // namespace RHI
