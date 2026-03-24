#pragma once

#include "RenderGraphResource.h"

#include <string>
#include <vector>
#include <functional> // For std::function
#include <list>

#include "RHICommandList.h"
#include "ShaderParameter.h"
#include "RHITransition.h"

namespace RenderCore {
    // Forward Declarations
class RenderGraph;
class RenderGraphBuilder;

class RENDERCORE_API BarrierBatchBegin {
public:
    void AddTransition(const RHITransitionInfo& transientInfo) {
        transitions.push_back(transientInfo);
    }

    void Execute(RHICommandList& commandList);

    const std::vector<RHITransitionInfo>& GetTransitions() const { return transitions; }

private:
    std::vector<RHITransitionInfo> transitions;
};

class RENDERCORE_API BarrierBatchEnd {
public:
    void AddTransition(const RHITransitionInfo& transientInfo) {
        transitions.push_back(transientInfo);
    }

    void Execute(RHICommandList& commandList);

    const std::vector<RHITransitionInfo>& GetTransitions() const { return transitions; }

private:
    std::vector<RHITransitionInfo> transitions;
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
    //ShaderParameterStruct ShaderParmeters;      // shader metadata (for binding slots, etc.)
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
    RenderGraphBufferUAV* ReadWriteBufferCache;//当前pass读写的buffer资源
    RenderGraphBufferSRV* ReadOnlyBufferCache;//当前pass只读的buffer资源
    RenderGraphBuffer* ReadWriteBufferResourceCache;//当前pass读写的buffer资源
    RenderGraphTexture* ReadTextureResourceCache;//当前pass读的texture资源
    RenderGraphTextureSRV* ReadOnlyTextureCache;//当前pass只读的texture资源
    RenderGraphTextureUAV* ReadWriteTextureCache;//当前pass读写的texture资源
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