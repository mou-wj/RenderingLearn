#include "RHICommandList.h"
#include "RHICommandContex.h"
#include "RHIApi.h"

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
void RHICommandList::CopyTexture(RHITexture* src, RHITexture* dst, const RHICopyTextureDesc& copyDesc)
{
    if (CommandContex)
        CommandContex->CopyTexture(src, dst, copyDesc);
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


void RHICommandList::SetStreamSource(uint32_t streamIndex, RHIBufferSP VertexBuffer, uint32_t Offset)
{
    if (CommandContex)
        CommandContex->SetStreamSource(streamIndex, VertexBuffer, Offset);
}

void RHICommandList::SetGraphicPipelineState(RHIGraphicsPipelineState* pipelineState)
{
    if (CommandContex)
        CommandContex->SetGraphicPipelineState(pipelineState);
}


void RHICommandList::SetBatchedShaderParameters(RHIShader* shader, const RHIBatchedShaderParameters& bacthedShaderParameter) {
    if (CommandContex)
        CommandContex->SetBatchedShaderParameters(shader, bacthedShaderParameter);
}
void RHICommandList::UpdateTexture(RHITexture* texture, const void* data, const RHITextureRegion& size) {
    if (GRHIApi) {
        GRHIApi->UpdateTexture(*this, texture, data, size);
    }
}
void RHICommandList::UpdateBuffer(RHIBuffer* buffer, const void* data, const RHIBufferRegion& region) {
    if (GRHIApi) {
        GRHIApi->UpdateBuffer(*this, buffer, data, region);
    }
}

void RHICommandList::SetViewport(float x, float y, float w, float h, float minDepth, float maxDepth)
{
    if (CommandContex)
        CommandContex->SetViewport(x,y, w,h, minDepth, maxDepth);
}
void RHICommandList::SetScissor(int32_t x, int32_t y, uint32_t w, uint32_t h) {
	if (CommandContex)
		CommandContex->SetScissor(x, y, w, h);
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

void RHICommandList::BeginDrawingViewport(RHIViewport* Viewport, RHITexture* RenderTargetRHI)
{
    CommandContex->BeginDrawingViewport(Viewport, RenderTargetRHI);
}
void RHICommandList::EndDrawingViewport(RHIViewport* Viewport, bool bPresent)
{
    CommandContex->EndDrawingViewport(Viewport, bPresent);
}
void RHICommandList::BeginFrame()
{
    CommandContex->BeginFrame();
}
void RHICommandList::EndFrame()
{
    CommandContex->EndFrame();
}
void RHICommandList::BeginRenderPass(const RHIRenderPassInfo& renderPassInfo) {
    CommandContex->BeginRenderPass(renderPassInfo);
}
void RHICommandList::EndRenderPass() {
    CommandContex->EndRenderPass();
}


void RHICommandList::Merge(std::shared_ptr<RHICommandList> other) {
    commands.insert(commands.end(), other->commands.begin(), other->commands.end());
}

} //