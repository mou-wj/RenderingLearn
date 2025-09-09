#include "RHICommandList.h"
#include "RHICommandContex.h"

namespace RHI {

// ---------------- RHICommandDispatch ----------------
RHICommandDispatch::RHICommandDispatch(uint32_t x, uint32_t y, uint32_t z)
    : X(x), Y(y), Z(z) {}

void RHICommandDispatch::Execute(RHICommandList& cmdList)
{
    auto* computeList = dynamic_cast<RHICommandList*>(&cmdList);
    if (computeList && computeList->GetCommandContex())
        computeList->GetCommandContex()->Dispatch(X, Y, Z);
}

// ---------------- RHICommandDraw ----------------
RHICommandDraw::RHICommandDraw(uint32_t v, uint32_t i, uint32_t fv, uint32_t fi)
    : VertexCount(v), InstanceCount(i), FirstVertex(fv), FirstInstance(fi) {}

void RHICommandDraw::Execute(RHICommandList& cmdList)
{
    auto* commandList = dynamic_cast<RHICommandList*>(&cmdList);
    if (commandList && commandList->GetCommandContex())
        commandList->GetCommandContex()->Draw(VertexCount, InstanceCount, FirstVertex, FirstInstance);
}

// ---------------- RHICommandTraceRays ----------------
RHICommandTraceRays::RHICommandTraceRays(uint32_t w, uint32_t h, uint32_t d)
    : Width(w), Height(h), Depth(d) {}

void RHICommandTraceRays::Execute(RHICommandList& cmdList)
{
    auto* commandList = dynamic_cast<RHICommandList*>(&cmdList);
    if (commandList && commandList->GetCommandContex())
        commandList->GetCommandContex()->TraceRays(Width, Height, Depth);
}


void RHICommandList::ExecuteAll()
{
    for (auto& cmd : commands)
    {
        if (cmd)
            cmd->Execute(*this);
    }
}

void RHICommandList::Clear()
{
    commands.clear();
}

void RHICommandList::SetImmediate(bool bImmediate)
{
    immediate = bImmediate;
}

bool RHICommandList::IsImmediate() const
{
    return immediate;
}

void RHICommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z)
{
    if (CommandContex)
        CommandContex->Dispatch(x, y, z);
}

// ---------------- RHICommandList ----------------
RHICommandList::RHICommandList(RHICommandContex* contex)
    : CommandContex(contex)
{
}

RHICommandList::~RHICommandList()
{
}

RHICommandContex* RHICommandList::GetCommandContex() const
{
    return CommandContex;
}

void RHICommandList::SetRenderTarget(const RHIRenderTargetsInfo& renderTargets)
{
    if (CommandContex)
        CommandContex->SetRenderTarget(renderTargets);
}

void RHICommandList::SetStreamSource(uint32_t streamIndex, RHIBuffer* VertexBuffer, uint32_t Offset)
{
    if (CommandContex)
        CommandContex->SetStreamSource(streamIndex, VertexBuffer, Offset);
}

void RHICommandList::SetGraphicPipelineState(const RHIGraphicsPipelineStateSP& pipelineState)
{
    if (CommandContex)
        CommandContex->SetGraphicPipelineState(pipelineState);
}

void RHICommandList::ViewportPresent(const RHIVIewportSP& viewport, const RHITextureSP& presentRenderTarget)
{
    if (CommandContex)
        CommandContex->ViewportPresent(viewport, presentRenderTarget);
}

void RHICommandList::SetViewPortRect(const RHIIntRect& viewport)
{
    if (CommandContex)
        CommandContex->SetViewPortRect(viewport);
}

void RHICommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    if (CommandContex)
        CommandContex->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void RHICommandList::TraceRays(uint32_t width, uint32_t height, uint32_t depth)
{
    if (CommandContex)
        CommandContex->TraceRays(width, height, depth);
}

} //