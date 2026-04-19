#pragma once

#include "RenderGraphResource.h"

#include <string>
#include <vector>
#include <functional> // For std::function
#include <list>

#include "RHICommandList.h"
#include "ShaderParameter.h"
#include "RHITransition.h"
#include "ShaderParameter.h"

namespace RenderCore {
    // Forward Declarations
class RenderGraph;
class RenderGraphBuilder;

class RENDERCORE_API BarrierBatchBegin {
public:
    void AddTransition(const RHITransitionInfo& transientInfo) {
        transitions.push_back(transientInfo);
    }

    void Execute(RHI::RHICommandListBase& commandList);

    const std::vector<RHITransitionInfo>& GetTransitions() const { return transitions; }

private:
    std::vector<RHITransitionInfo> transitions;
};

class RENDERCORE_API BarrierBatchEnd {
public:
    void AddTransition(const RHITransitionInfo& transientInfo) {
        transitions.push_back(transientInfo);
    }

    void Execute(RHI::RHICommandListBase& commandList);

    const std::vector<RHITransitionInfo>& GetTransitions() const { return transitions; }

private:
    std::vector<RHITransitionInfo> transitions;
};

enum class EPassFlag {
    None,
    Graphic,
    Compute,
    Transfer
};

/**
 * 对应 UE 的 FRDGParameterStruct
 * 封装了 Pass 所需的所有数据（常量、RDG资源、RenderTarget 绑定）
 */
class RENDERCORE_API RenderGraphParameterStruct
{
public:
    RenderGraphParameterStruct() = default;

    /**
     * @param InContents 指向由 Builder 分配并由用户填充的内存块
     * @param InMetadata 该内存块对应的反射元数据
     */
    RenderGraphParameterStruct(const void* InContents, const ShaderParametersMetadata* InMetadata)
        : Contents(reinterpret_cast<const uint8_t*>(InContents))
        , Metadata(InMetadata)
    {
        assert(Contents && Metadata);
    }

    // 获取原始内存指针
    const uint8_t* GetContents() const { return Contents; }

    // 获取反射元数据
    const ShaderParametersMetadata* GetMetadata() const { return Metadata; }


private:


    const uint8_t* Contents = nullptr;
    const ShaderParametersMetadata* Metadata = nullptr;
};


//  Render Graph Pass Base Class
// -------------------------------------------------------------------------------------------------
class RENDERCORE_API RenderGraphPass
{
public:
    // Construction/Destruction
    RenderGraphPass(const std::string& name, EPassFlag passFlag, const RenderGraphParameterStruct& parameter) ;
    virtual ~RenderGraphPass() = default;

    // Accessors
    const std::string& GetName() const { return Name; }

    // Execution
    virtual void Execute(RHI::RHICommandListBase& commandList) = 0;

    // Pass Info
    const RenderGraphParameterStruct& GetParameterStruct() const { return ParameterStruct; }

protected:
    // Pass Name (for debugging and identification)
    std::string Name;
    // Pass Info (metadata about the pass)
    RenderGraphParameterStruct ParameterStruct;
    using PassList = std::list<RenderGraphPass*>;
    PassList PassConsumers;//依赖当前pass的pass集合
    PassList PassProducers;//当前pass依赖的pass集合
    BarrierBatchBegin BeginBarrier;//pass执行开始前的barrier
    BarrierBatchEnd EndBarrier;//pass执行结束后的barrier
    /** 
     * Pass 内部持有的访问意向
     */
    struct RenderGraphTextureIntent
    {
        RenderGraphTexture* Texture = nullptr;

        // 该 Pass 对纹理的访问需求
        ERHIResourceAccess RequiredAccess = ERHIResourceAccess::Unknown;

        // 访问的子资源范围（重要：解决你之前担心的 Subresource 问题）
        // 如果是全资源访问，可以使用之前定义的 kAllSubresources
        RHISubresourceRange SubresourceRange;
        
    };

    /**
     * Pass 内部持有的 Buffer 访问意向
     */
    struct RenderGraphBufferIntent
    {
        RenderGraphBuffer* Buffer = nullptr;

        // 该 Pass 需要的访问权限 (如：ShaderResource, UnorderedAccess, VertexBuffer 等)
        ERHIResourceAccess RequiredAccess = ERHIResourceAccess::Unknown;

        // 虽然 Buffer 通常视为整体，但为了严谨，可以预留偏移和大小
        // 某些高级优化（如 Buffer Aliasing）可能会用到
        uint64_t Offset = 0;
        uint64_t Size = 0; // 0 表示整个 Buffer
    };

    std::vector<RenderGraphTextureIntent> TextureIntents;
    std::vector<RenderGraphBufferIntent>  BufferStates;
    friend class RenderGraphBuilder;
};

// -------------------------------------------------------------------------------------------------
//  Render Graph Lambda Pass (For simple passes defined inline)
// -------------------------------------------------------------------------------------------------

/**
 * 模板化的 Lambda Pass
 * TParameterStruct: 用户定义的参数结构体类型 (例如 FMyPassParams)
 * TExecuteLambda:   用户编写的执行逻辑 Lambda 类型
 */
template <typename TParameterStruct, typename TExecuteLambda>
class RenderGraphLambdaPass final : public RenderGraphPass
{
public:
    RenderGraphLambdaPass(
        const std::string& InName,
        EPassFlag InPassFlag,
        const RenderGraphParameterStruct& InParams,
        TExecuteLambda&& InExecuteLambda)
        : RenderGraphPass(InName, InPassFlag, InParams)
        , ExecuteLambda(std::forward<TExecuteLambda>(InExecuteLambda))
    {
    }

    /**
     * 实现基类的 Execute 接口
     */
    void Execute(RHI::RHICommandListBase& RHICmdList) override
    {
        ExecuteLambda(RHICmdList);
    }

private:
    // 直接存储 Lambda 对象，避免 std::function 的堆分配和虚函数调用开销
    TExecuteLambda ExecuteLambda;
};


using RenderGraphPassRef = RenderGraphPass*;


} // namespace WR::RenderCore