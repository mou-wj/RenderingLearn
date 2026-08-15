#pragma once

#include "RenderGraphResource.h" // For RenderGraphResource
#include "RenderGraphPass.h"
#include "RenderResource.h"
#include <list>
#include <vector>
#include <unordered_map>

namespace RenderCore {



/**
 * ���Է�������ÿ֡ͳһ���䣬ͳһ����
 */
class RenderGraphAllocator {
public:
    RenderGraphAllocator(size_t InPageSize = 64 * 1024)
        : PageSize(InPageSize), CurrentPage(-1), CurrentOffset(0) {
    }

    ~RenderGraphAllocator() {
        // ����ʱ���볹���ͷ����������ڴ�
        Reset();
        for (void* Page : Pages) {
            free(Page);
        }
    }

    /**
     * ����ĵķ���ӿ�
     */
    template<typename T, typename... Args>
    T* Allocate(Args&&... args) {
        std::lock_guard<std::recursive_mutex> lock(Mutex_);
        // 1. ����ռ�
        void* RawMem = AllocateRaw(sizeof(T), alignof(T));

        // 2. ������� (Placement New)
        T* Object = new (RawMem) T(std::forward<Args>(args)...);

        // 3. ��������ǡ�ƽ���������ģ����纬�� std::string������¼������������
        if constexpr (!std::is_trivially_destructible_v<T>) {
            DestructionStack.push_back({
                Object,
                [](void* Ptr) { static_cast<T*>(Ptr)->~T(); }
                });
        }
        return Object;
    }

    void* AllocateBytes(size_t Size, size_t Alignment = alignof(std::max_align_t))
    {
        return AllocateRaw(Size, Alignment);
    }

    /**
     * ÿһ֡����ʱ����
     */
    void Reset() {
        // 1. ������˳��ķ��������������
        for (auto it = DestructionStack.rbegin(); it != DestructionStack.rend(); ++it) {
            it->Deleter(it->Ptr);
        }
        DestructionStack.clear();

        // 2. ����ָ�룬�����ͷ�����ҳ�ڴ棬������һ֡����
        CurrentPage = 0;
        CurrentOffset = 0;
    }

private:
    void* AllocateRaw(size_t Size, size_t Alignment) {
        std::lock_guard<std::recursive_mutex> lock(Mutex_);
        // ����Ķ����߼�ȷ��ָ����� CPU Ҫ��
        size_t Padding = (Alignment - (CurrentOffset % Alignment)) % Alignment;

        if (CurrentPage == -1 || CurrentOffset + Padding + Size > PageSize) {
            MoveToNextPage();
            Padding = 0; // ��ҳͨ������Ȼ�����
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
    std::recursive_mutex Mutex_;
    std::vector<void*> Pages;                // ����ҳ��
    std::vector<DestructionItem> DestructionStack; // ������������
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
     * �Ľ���� AddPass ģ��
     */
    template <typename TParameterStruct, typename TExecuteLambda>
    RenderGraphPassRef AddPass(
        const std::string& InName,
        const ShaderParametersMetadata* InMetadata,
        TParameterStruct* InParameters, // �� AllocateParameter ����Ĳ���ָ��
        EPassFlag InPassFlags,
        TExecuteLambda&& InExecuteLambda)
    {
        // 1. ���û����� TParameterStruct ��װ�� RenderGraphParameterStruct
        RenderGraphParameterStruct ParameterStruct(InParameters, InMetadata);

        // 2. ���������� Pass ʵ�� (����֮ǰ����� RenderGraphLambdaPass)
        auto Pass = new RenderGraphLambdaPass<TParameterStruct, TExecuteLambda>(
            InName,
            InPassFlags,
            ParameterStruct,
            std::forward<TExecuteLambda>(InExecuteLambda)
        );

        // 3. ���ģ����� TParameterStruct �е���Դ���ã����� DAG ����
        // �� UE �У��������� SetupPass(Pass) ��������Դ Read/Write ״̬
        SetupPassInternal(Pass, InMetadata, InParameters);

        // 4. ����ִ�ж���
        Passes.push_back(Pass);
        return Pass;
    }
    void AddPassDependency(RenderGraphPass* pass, RenderGraphPass* passConsumer);
    enum class EUploadPolicy {
		Immediate, // 立即插入一个upload pass
		Deferred   // 延迟插入upload pass，直到 Execute() 被调用是收集所有的upload buffer，然后统一插入upload pass
    };
    struct UploadBufferDesc {
        RenderGraphBufferRef buffer;
        void* Data;
		size_t Size;
    };
    void AddUploadBuffers(const std::vector<UploadBufferDesc>& desc, EUploadPolicy policy = EUploadPolicy::Deferred);
    void AddUploadBuffer(RenderGraphBufferRef buffer, const void* data, size_t size, EUploadPolicy policy = EUploadPolicy::Deferred);

    // Resource Creation (Examples - Add more as needed)
    RenderGraphTextureRef CreateTexture(const std::string& name, const RenderGraphTextureDesc& desc);
    RenderGraphBufferRef CreateBuffer(const std::string& name, const RenderGraphBufferDesc& desc);

    RenderGraphTextureSRVRef CreateTextureSRV(const std::string& name, const RenderGraphTextureSRVDesc& desc);
    RenderGraphBufferSRVRef CreateBufferSRV(const std::string& name, const RenderGraphBufferSRVDesc& desc);
    RenderGraphTextureUAVRef CreateTextureUAV(const std::string& name, const RenderGraphTextureUAVDesc& desc);
    RenderGraphBufferUAVRef CreateBufferUAV(const std::string& name, const RenderGraphBufferUAVDesc& desc);

    RenderGraphTextureRef RegisterExternalTexture(const std::string& name, PooledRenderTarget* target);
    RenderGraphTextureRef RegisterExternalTexture(const std::string& name, RenderTexture* texture);
    RenderGraphBufferRef RegisterExternalBuffer(const std::string& name, RenderBuffer* buffer);

    // Resource Access (Get RHI resources from RenderGraphResources)
    RHI::RHITexture* GetTexture(RenderGraphResourceRef resource);
    RHI::RHIBuffer* GetBuffer(RenderGraphResourceRef resource);

    RenderGraphTextureRef GetTexture(const std::string& name);
    RenderGraphBufferRef GetBuffer(const std::string& name);

    TextureViewCache* GetExternalTextureViewCache(RenderGraphTextureRef tex);
	BufferViewCache* GetExternalBufferViewCache(RenderGraphBufferRef buf);
    template<typename T>
    T* AllocateParameter(){
        return Allocator.Allocate<T>();
    }



    // Execution
    void Execute(); // Executes all added passes
protected:
    void AnalyzePasses(); // Analyzes passes and groups them for execution
    void AllocateResources();
    void SetupPassInternal(
        RenderGraphPass* Pass,
        const ShaderParametersMetadata* Metadata,
        const void* Parameters);
    void ApplyFinalStates();
    void ExecutaPasses();
private:
    void AnalyzeUpload();
    struct PendingBufferUpload;
    void AddUploadPass(std::vector<PendingBufferUpload>&& uploadInfo,RHI::EQueueType queueType);


    using PassList = std::list<RenderGraphPass*>; // Using std::list for pass management
    using PassListGroup = std::list<PassList>; // Group of passes for execution
    PassList Passes; // List of passes to execute (using std::list)
    PassListGroup ParallelPasses; // Group of passes to execute in parallel (using std::list)
    // Resource Cache
    std::unordered_map<std::string, RenderGraphTextureRef> TextureCache; // Cache for textures
    std::unordered_map<std::string, RenderGraphBufferRef> BufferCache;   // Cache for buffers
    std::unordered_map<std::string, RenderGraphTextureSRVRef> TextureSRVCache; // Cache for texture SRVs
    std::unordered_map<std::string, RenderGraphBufferSRVRef> BufferSRVCache;   // Cache for buffer SRVs
	std::unordered_map<std::string, RenderGraphTextureUAVRef> TextureUAVCache; // Cache for texture UAVs
    std::unordered_map<std::string, RenderGraphBufferUAVRef> BufferUAVCache;   // Cache for buffer UAVs

    // 新增：Builder 内部状态缓存
    struct TextureKey
    {
        RenderGraphTexture* Texture;
        RHI::RHISubresourceRange Range;

        bool operator==(const TextureKey& o) const
        {
            return Texture == o.Texture && Range == o.Range;
        }
    };

    struct TextureKeyHasher
    {
        size_t operator()(const TextureKey& k) const
        {
            size_t h = std::hash<void*>()(k.Texture);
            h ^= (size_t(k.Range.MipIndex) << 1);
            h ^= (size_t(k.Range.ArraySlice) << 2);
            h ^= (size_t(k.Range.PlaneSlice) << 3);
            return h;
        }
    };

    struct BufferKey
    {
        RenderGraphBuffer* Buffer;

        bool operator==(const BufferKey& o) const
        {
            return Buffer == o.Buffer ;
        }
    };

    struct BufferKeyHasher
    {
        size_t operator()(const BufferKey& k) const
        {
            size_t h = std::hash<void*>()(k.Buffer);
            return h;
        }
    };
    std::unordered_map<TextureKey, RHI::ERHIResourceAccess, TextureKeyHasher> FinalTextureStates;

    std::unordered_map<BufferKey, RHI::ERHIResourceAccess, BufferKeyHasher>   FinalBufferStates;

    //
// Texture
    struct ExternalTextureEntry
    {
        RenderGraphTextureRef RDGTexture;
        RenderTextureTracker* Tracker = nullptr;
        TextureViewCache* ViewCache = nullptr;
		RHI::RHITexture* RHITexture = nullptr;
    };

    std::unordered_map<RHI::RHITexture*, ExternalTextureEntry> ExternalTextures;


    // Buffer
    struct ExternalBufferEntry
    {
        RenderGraphBufferRef RDGBuffer;
        RenderBufferTracker* Tracker = nullptr;
        BufferViewCache* ViewCache = nullptr;
		RHI::RHIBuffer* RHIBuffer = nullptr;
    };

    std::unordered_map<RHI::RHIBuffer*, ExternalBufferEntry> ExternalBuffers;

    //
    std::unordered_map<RenderGraphTexture*, RHI::RHITexture*> RDGToRHITexture;
    std::unordered_map<RenderGraphBuffer*, RHI::RHIBuffer*>  RDGToRHIBuffer;

    std::unordered_map<RHI::RHITexture*, PooledTransientRenderTarget*> RHITextureToTransientTarget;
    std::unordered_map<RHI::RHIBuffer*, PooledTransientBuffer*>  RHIBufferTransientBuffer;
    RenderGraphAllocator Allocator;
    TransientResourceAllocator* TransientAllocator;

    struct ResourceLifetime
    {
        RenderGraphPass* FirstPass = nullptr;
        RenderGraphPass* LastPass = nullptr;

        uint32_t BeginPassIndex = UINT32_MAX;
        uint32_t EndPassIndex = 0;
    };

    std::unordered_map<RenderGraphResource*, ResourceLifetime> ResourceLifetimes;
    std::unordered_map<RenderGraphPass*, std::vector<RenderGraphResource*>> PassLastUseResources;

	struct PendingBufferUpload
	{
		RenderGraphBufferRef Buffer;
		const void* Data;
		size_t Size;
        RHI::EQueueType QueueType;
	};
    std::vector<PendingBufferUpload> PendingBufferUploads;
};

} // namespace WR::RenderCore