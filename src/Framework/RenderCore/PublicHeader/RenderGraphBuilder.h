#pragma once

#include "RenderGraphResource.h" // For RenderGraphResource
#include "RenderGraphPass.h"
#include <list>
#include <vector>
#include <unordered_map>

namespace RenderCore {
struct RENDERCORE_API GraphResourceAccessState {
    RenderGraphPass* LastVisitor = nullptr;
    RenderGraphResourceSP Resource;
    struct TextureAccess {
        ERHIResourceAccess Access;
        RHITextureRegion TextureRegion;
    };
    struct BufferAccess {
        ERHIResourceAccess Access;
        RHIBufferRegion BufferRegion;
    };
    std::vector<TextureAccess> TextureAccesses;
    std::vector<BufferAccess> BufferAccesses;
};


/**
 * 线性分配器：每帧统一分配，统一重置
 */
class RenderGraphAllocator {
public:
    RenderGraphAllocator(size_t InPageSize = 64 * 1024)
        : PageSize(InPageSize), CurrentPage(-1), CurrentOffset(0) {
    }

    ~RenderGraphAllocator() {
        // 销毁时必须彻底释放所有物理内存
        Reset();
        for (void* Page : Pages) {
            free(Page);
        }
    }

    /**
     * 最核心的分配接口
     */
    template<typename T, typename... Args>
    T* Allocate(Args&&... args) {
        // 1. 申请空间
        void* RawMem = AllocateRaw(sizeof(T), alignof(T));

        // 2. 构造对象 (Placement New)
        T* Object = new (RawMem) T(std::forward<Args>(args)...);

        // 3. 如果对象不是“平凡析构”的（比如含有 std::string），记录它的析构函数
        if constexpr (!std::is_trivially_destructible_v<T>) {
            DestructionStack.push_back({
                Object,
                [](void* Ptr) { static_cast<T*>(Ptr)->~T(); }
                });
        }
        return Object;
    }

    /**
     * 每一帧结束时调用
     */
    void Reset() {
        // 1. 按分配顺序的反序调用析构函数
        for (auto it = DestructionStack.rbegin(); it != DestructionStack.rend(); ++it) {
            it->Deleter(it->Ptr);
        }
        DestructionStack.clear();

        // 2. 重置指针，但不释放物理页内存，留给下一帧复用
        CurrentPage = 0;
        CurrentOffset = 0;
    }

private:
    void* AllocateRaw(size_t Size, size_t Alignment) {
        // 这里的对齐逻辑确保指针符合 CPU 要求
        size_t Padding = (Alignment - (CurrentOffset % Alignment)) % Alignment;

        if (CurrentPage == -1 || CurrentOffset + Padding + Size > PageSize) {
            MoveToNextPage();
            Padding = 0; // 新页通常是自然对齐的
        }

        void* Result = (uint8_t*)Pages[CurrentPage] + CurrentOffset + Padding;
        CurrentOffset += (Padding + Size);
        return Result;
    }

    void MoveToNextPage() {
        CurrentPage++;
        if (CurrentPage >= Pages.size()) {
            Pages.push_back(malloc(PageSize));
        }
        CurrentOffset = 0;
    }

    struct DestructionItem {
        void* Ptr;
        void (*Deleter)(void*);
    };

    size_t PageSize;
    int32_t CurrentPage;
    size_t CurrentOffset;

    std::vector<void*> Pages;                // 物理页池
    std::vector<DestructionItem> DestructionStack; // 析构函数链表
};


// -------------------------------------------------------------------------------------------------
//  Render Graph Builder (Now also managing passes and execution)
// -------------------------------------------------------------------------------------------------
class RENDERCORE_API RenderGraphBuilder
{
public:
    // Construction/Destruction
    RenderGraphBuilder();
    ~RenderGraphBuilder();

    // Pass Management
    /**
     * 改进后的 AddPass 模板
     */
    template <typename TParameterStruct, typename TExecuteLambda>
    RenderGraphPassRef AddPass(
        const std::string& InName,
        const ShaderParametersMetadata* InMetadata,
        TParameterStruct* InParameters, // 由 AllocateParameter 分配的参数指针
        EPassFlag InPassFlags,
        TExecuteLambda&& InExecuteLambda)
    {
        // 1. 将用户填充的 TParameterStruct 封装进 RenderGraphParameterStruct
        RenderGraphParameterStruct ParameterStruct(InParameters, InMetadata);

        // 2. 创建真正的 Pass 实例 (利用之前定义的 RenderGraphLambdaPass)
        auto Pass = new RenderGraphLambdaPass<TParameterStruct, TExecuteLambda>(
            InName,
            InPassFlags,
            ParameterStruct,
            std::forward<TExecuteLambda>(InExecuteLambda)
        );

        // 3. 核心：解析 TParameterStruct 中的资源引用，建立 DAG 依赖
        // 在 UE 中，这里会调用 SetupPass(Pass) 来分析资源 Read/Write 状态
        SetupPassInternal(Pass, InMetadata, InParameters);

        // 4. 加入执行队列
        Passes.push_back(Pass);
        return Pass;
    }
    void AddPassDependency(RenderGraphPass* pass, RenderGraphPass* passConsumer);

    // Resource Creation (Examples - Add more as needed)
    RenderGraphTextureSP CreateTexture(const std::string& name, const RenderGraphTextureDesc& desc);
    RenderGraphBufferSP CreateBuffer(const std::string& name, const RenderGraphBufferDesc& desc);

    RenderGraphTextureSRVSP CreateTextureSRV(const std::string& name, RenderGraphResourceSP resource);
    RenderGraphBufferSRVSP CreateBufferSRV(const std::string& name, RenderGraphResourceSP resource);
    RenderGraphTextureUAVSP CreateTextureUAV(const std::string& name, RenderGraphResourceSP resource);
    RenderGraphBufferUAVSP CreateBufferUAV(const std::string& name, RenderGraphResourceSP resource);

    RenderGraphTextureSP RegisterExternalTexture(const std::string& name, RHITexture* texture);
    RenderGraphTextureSP GetExternalTexture(const std::string& name);

    // Resource Access (Get RHI resources from RenderGraphResources)
    RHITexture* GetTexture(RenderGraphResourceSP resource);
    RHIBuffer* GetBuffer(RenderGraphResourceSP resource);

    RenderGraphTextureSP GetTexture(const std::string& name);
    RenderGraphBufferSP GetBuffer(const std::string& name);
    template<typename T>
    T* AllocateParameter(){
        T* ptr = new T();
        ParameterCache.push_back(ptr);
        return ptr;
    }



    // Execution
    void Execute(); // Executes all added passes
protected:
    void AnalyzePasses(); // Analyzes passes and groups them for execution
    void SetupPassInternal(
        RenderGraphPass* Pass,
        const ShaderParametersMetadata* Metadata,
        const void* Parameters);
private:
    using PassList = std::list<RenderGraphPass*>; // Using std::list for pass management
    using PassListGroup = std::list<PassList>; // Group of passes for execution
    PassList Passes; // List of passes to execute (using std::list)
    PassListGroup ParallelPasses; // Group of passes to execute in parallel (using std::list)
    // Resource Cache
    std::unordered_map<std::string, RenderGraphTextureSP> TextureCache; // Cache for textures
    std::unordered_map<std::string, RenderGraphBufferSP> BufferCache;   // Cache for buffers
    //Parameters Storage
    std::vector<void*> ParameterCache; // Cache for parameters
    //
    std::unordered_map<std::string, RenderGraphTextureSP> ExternalTextureCache; // Cache for resources
};

} // namespace WR::RenderCore