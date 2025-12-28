#pragma once

#include "RenderGraphResource.h"

#include <string>
#include <vector>
#include <functional> // For std::function
#include <list>
#include "RHIRenderTargetInfo.h"
#include "RHICommandList.h"
#include "ShaderParameter.h"
#include "RHITransientResource.h"

namespace RenderCore {
    // Forward Declarations
class RenderGraph;
class RenderGraphBuilder;

class RENDERCORE_API BarrierBatchBegin {
public:
    void AddTransition(const RHITransientInfo& transientInfo) {
        transitions.push_back(transientInfo);
    }

    void Execute(RHICommandList& commandList);

    const std::vector<RHITransientInfo>& GetTransitions() const { return transitions; }

private:
    std::vector<RHITransientInfo> transitions;
};

class RENDERCORE_API BarrierBatchEnd {
public:
    void AddTransition(const RHITransientInfo& transientInfo) {
        transitions.push_back(transientInfo);
    }

    void Execute(RHICommandList& commandList);

    const std::vector<RHITransientInfo>& GetTransitions() const { return transitions; }

private:
    std::vector<RHITransientInfo> transitions;
};

/**
 * RenderCore版本的渲染目标集合，基于RHIRenderTargetsInfo
 * 提供对RenderCore资源类型的支持
 */
struct RENDERCORE_API RenderGraphRenderTargetsInfo
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

enum class EPassFlag {
    None,
    Graphic,
    Compute,
};


struct RENDERCORE_API RenderGraphPassInfo
{
    std::string Name;                              // Pass name
    EPassFlag PassFlag;                         // Pass type
    ShaderParameterStruct ShaderParmeters;      // shader metadata (for binding slots, etc.)
    RenderGraphPassInfo(const std::string& name = "")
        : Name(name){}
};

// -------------------------------------------------------------------------------------------------
//  Render Graph Pass Base Class
// -------------------------------------------------------------------------------------------------
class RENDERCORE_API RenderGraphPass
{
public:
    // Construction/Destruction
    RenderGraphPass(const std::string& name,  const RenderGraphPassInfo& info) ;
    virtual ~RenderGraphPass() = default;

    // Accessors
    const std::string& GetName() const { return Name; }

    // Execution
    virtual void Execute(RHICommandList& commandList) = 0;

    // Pass Info
    const RenderGraphPassInfo& GetPassInfo() const { return PassInfo; }

protected:
    // Pass Name (for debugging and identification)
    std::string Name;
    // Pass Info (metadata about the pass)
    RenderGraphPassInfo PassInfo;
    using PassList = std::list<RenderGraphPass*>;
    PassList PassConsumers;//依赖当前pass的pass集合
    PassList PassProducers;//当前pass依赖的pass集合
    BarrierBatchBegin BeginBarrier;//pass开始前的barrier
    BarrierBatchEnd EndBarrier;//pass结束后的barrier
    RenderGraphBufferUAVSP ReadWriteBufferCache;//当前pass读写的buffer资源
    RenderGraphBufferSRVSP ReadOnlyBufferCache;//当前pass只读的buffer资源
    RenderGraphBufferSP ReadWriteBufferResourceCache;//当前pass读写的buffer资源
    RenderGraphTextureSP ReadTextureResourceCache;//当前pass读的texture资源
    RenderGraphTextureSRVSP ReadOnlyTextureCache;//当前pass只读的texture资源
    RenderGraphTextureUAVSP ReadWriteTextureCache;//当前pass读写的texture资源
    friend class RenderGraphBuilder;
};

// -------------------------------------------------------------------------------------------------
//  Render Graph Lambda Pass (For simple passes defined inline)
// -------------------------------------------------------------------------------------------------

class RENDERCORE_API RenderGraphLambdaPass : public RenderGraphPass
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

using RenderGraphPassSP = std::shared_ptr<RenderGraphPass>;


} // namespace WR::RenderCore