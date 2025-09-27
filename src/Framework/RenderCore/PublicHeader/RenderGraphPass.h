#pragma once

#include "RenderGraphResource.h"

#include <string>
#include <vector>
#include <functional> // For std::function
#include "RHIRenderTargetInfo.h"
#include "RHICommandList.h"
#include "ShaderParameter.h"

namespace RenderCore {
    // Forward Declarations
class RenderGraph;
class RenderGraphBuilder;

/**
 * RenderCore版本的渲染目标集合，基于RHIRenderTargetsInfo
 * 提供对RenderCore资源类型的支持
 */
struct RenderGraphRenderTargetsInfo
{
private:

    // 底层RHI渲染目标信息
    RHIRenderTargetsInfo RHIInfo;
public:
    RenderGraphRenderTargetsInfo(){


    }
    ~RenderGraphRenderTargetsInfo() {
        // 清理颜色附件
        for (auto& colorTarget : RHIInfo.ColorRenderTargets) {
            delete colorTarget;
            colorTarget = nullptr;
        }
        // 清理深度/模板附件
        delete RHIInfo.DepthStencilRenderTarget;
        RHIInfo.DepthStencilRenderTarget = nullptr;
    }
    // 设置颜色附件 (RenderCore资源版本)
    void SetColorRenderTarget(int32_t Index, RenderGraphTextureSP Texture, 
                            uint32_t ArraySlice = 0, uint32_t MipLevel = 0,
                            ERHILoadAction LoadAction = ERHILoadAction::Load,
                            ERHIStoreAction StoreAction = ERHIStoreAction::Store,
                            const RHIColor& ClearColor = RHIColor{})
    {
        RHIInfo.SetColorRenderTarget(Index, 
            new RHIRenderTargetInfo(Texture->GetRHITexture(),
                                  ArraySlice, MipLevel, LoadAction, StoreAction, ClearColor));
    }

    // 设置深度/模板附件 (RenderCore资源版本)
    void SetDepthStencilRenderTarget(RenderGraphTextureSP Texture,
                                   uint32_t ArraySlice = 0, uint32_t MipLevel = 0,
                                   ERHILoadAction LoadAction = ERHILoadAction::Load,
                                   ERHIStoreAction StoreAction = ERHIStoreAction::Store,
                                   float ClearDepth = 1.0f, uint32_t ClearStencil = 0)
    {
        RHIInfo.SetDepthStencilRenderTarget(
            new RHIRenderTargetInfo(Texture->GetRHITexture(),
                                  ArraySlice, MipLevel, LoadAction, StoreAction,
                                  ClearDepth, ClearStencil));
    }

    // 获取底层RHI信息
    const RHIRenderTargetsInfo& GetRHIInfo() const { return RHIInfo; }
    RHIRenderTargetsInfo& GetRHIInfo() { return RHIInfo; }
};


struct RenderGraphPassInfo
{
    std::string Name;                              // Pass name
    ERHIPipelineType PipelineType;                 // Pipeline type (e.g., Graphics, Compute)
    std::vector<ShaderMetaData> ShaderMetaDatas;      // shader metadata (for binding slots, etc.)
    RenderGraphRenderTargetsInfo RenderTargets; // Render target info (if applicable)
    RenderGraphPassInfo(const std::string& name = "", ERHIPipelineType pipelineType = ERHIPipelineType::Graphics)
        : Name(name), PipelineType(pipelineType) {}

};

// -------------------------------------------------------------------------------------------------
//  Render Graph Pass Base Class
// -------------------------------------------------------------------------------------------------
class RenderGraphPass
{
public:
    // Construction/Destruction
    RenderGraphPass(const std::string& name,  const RenderGraphPassInfo& info) : Name(name), PassInfo(info) {}
    virtual ~RenderGraphPass() = default;

    // Accessors
    const std::string& GetName() const { return Name; }

    // Execution
    virtual void Execute(RHICommandList& commandList) = 0;

    // Pipeline Type
    virtual ERHIPipelineType GetPipelineType() const { return PassInfo.PipelineType; }

    // Pass Info
    const RenderGraphPassInfo& GetPassInfo() const { return PassInfo; }

protected:
    // Pass Name (for debugging and identification)
    std::string Name;
    // Pass Info (metadata about the pass)
    RenderGraphPassInfo PassInfo;
};

// -------------------------------------------------------------------------------------------------
//  Render Graph Lambda Pass (For simple passes defined inline)
// -------------------------------------------------------------------------------------------------

class RenderGraphLambdaPass : public RenderGraphPass
{
public:
    RenderGraphLambdaPass(const std::string& name,  const RenderGraphPassInfo& info, std::function<void(RHICommandList&)>&& lambda) : RenderGraphPass(name, info), ExecuteFunction(std::forward<std::function<void(RHICommandList&)>>(lambda)) {}
    ~RenderGraphLambdaPass() override {}

    void Execute(RHICommandList& commandList) override {
        ExecuteFunction(commandList);
    }

private:
    std::function<void(RHICommandList&)> ExecuteFunction;
};



} // namespace WR::RenderCore