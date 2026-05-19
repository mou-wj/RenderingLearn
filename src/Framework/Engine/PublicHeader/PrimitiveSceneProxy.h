// PrimitiveSceneProxy.h
#pragma once
#include "EngineExport.h"
#include <cstdint>
#include <vector>
#include "BoxSphereBounds.h"

// 前置声明底层及核心层的渲染资源
namespace RenderCore {
    class RenderBuffer;
    class VertexFactory;
}
namespace RHI {
    class RHIBuffer;
}

namespace Engine {
    struct SceneView;
    class MaterialRenderProxy;

    /*
    ===============================================================================
        MeshBatchElement (完全基于指针的原子绘制单元)
    ===============================================================================
    */
    struct MeshBatchElement {
        // 直接持有当前物体在渲染线程对应的常量缓冲区（Uniform Buffer）指针
        // 用于给 Shader 传递物体的 LocalToWorld 矩阵、自定义裁剪数据等
        void* PrimitiveUniformBufferData = nullptr;

        // 几何绘制区间控制
        uint32_t FirstIndex = 0;
        uint32_t NumPrimitives = 0;
        int32_t BaseVertexIndex = 0;
        uint32_t StartInstance = 0;
        uint32_t NumInstances = 1;

        // 间接绘制命令缓冲（面向 GPU Driven Pipeline 的预留设计）
        RenderCore::RenderBuffer* IndirectArgsBuffer = nullptr;
        uint32_t IndirectArgsOffset = 0;

        const void* UserData = nullptr;
    };

    using MeshBatchElementList = std::vector<MeshBatchElement>;

    /*
    ===============================================================================
        MeshBatch (状态一致的批次集合)
    ===============================================================================
    */
    struct MeshBatch {
        // 内嵌的元素数组
        std::vector<MeshBatchElement> Elements;

        // 核心渲染状态（一个批次内必须严格一致，触发同一个 PSO）
        RenderCore::VertexFactory* VertexFactory = nullptr;
        const MaterialRenderProxy* MaterialProxy = nullptr;

        // 几何数据源指针挪到了 Batch 状态层，完美适配你的 RenderCore 资源
        RenderCore::RenderBuffer* IndexBuffer = nullptr;

        // 管线状态标志位
        uint32_t CastShadow : 1;
        uint32_t bUseForDepthPass : 1;
        uint32_t ReverseCulling : 1;

        uint8_t LODIndex = 0;
        uint16_t MeshIdInPrimitive = 0;

        MeshBatch()
            : CastShadow(1)
            , bUseForDepthPass(1)
            , ReverseCulling(0)
        {
            Elements.emplace_back(); // 默认预留一个原子绘制单元
        }
    };

    using MeshBatchList = std::vector<MeshBatch>;

    /*
    ===============================================================================
        PrimitiveSceneProxy (基类)
    ===============================================================================
    */
    class ENGINE_API PrimitiveSceneProxy {
    public:
        PrimitiveSceneProxy();
        virtual ~PrimitiveSceneProxy();

        int32_t GetPrimitiveId() const { return PrimitiveId; }
        const Core::AABB& GetBounds() const { return ProxyBounds; }
        const float* GetLocalToWorld() const { return LocalToWorld; }

        virtual bool IsVisible() const { return bVisible; }
        virtual bool CastsShadow() const { return bCastShadow; }
        virtual bool IsOpaque() const { return bOpaque; }

        // 核心裁剪与 Batch 收集接口
        virtual void GetMeshBatches(const SceneView& View, MeshBatchList& OutBatches) const = 0;

        virtual bool HasStaticGeometry() const = 0;
        virtual bool IsDynamic() const = 0;

    protected:
        int32_t PrimitiveId;
        Core::AABB ProxyBounds;
        float LocalToWorld[16];

        // 常量数据资源
        void* PrimitiveUniformBufferData = nullptr;

        std::vector<const MaterialRenderProxy*> MaterialProxies;

        uint32_t bVisible : 1;
        uint32_t bCastShadow : 1;
        uint32_t bOpaque : 1;
        uint32_t RenderFlags : 29;
    };
} // namespace Engine