// PrimitiveSceneProxy.h
#pragma once
#include "EngineExport.h"
#include <cstdint>
#include <vector>
#include "BoxSphereBounds.h"
#include "Math.hpp"
#include "TypeIDCast.h"

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
        PrimitiveSceneProxy (基类)
    ===============================================================================
    */
    class ENGINE_API PrimitiveSceneProxy {
    public:
        PrimitiveSceneProxy();
        virtual ~PrimitiveSceneProxy();

        int32_t GetPrimitiveId() const { return PrimitiveId; }
        const Core::AABB& GetBounds() const { return ProxyBounds; }
        const Core::Mat4& GetLocalToWorld() const { return LocalToWorld; }
        const Core::Mat4& GetWorldToLocal() const { return WorldToLocal; }

        virtual bool IsVisible() const { return bVisible; }
        virtual bool CastsShadow() const { return bCastShadow; }
        virtual bool IsOpaque() const { return bOpaque; }

        virtual bool HasStaticGeometry() const = 0;
        virtual bool IsDynamic() const = 0;
        DECLARE_TYPE_ID_BASE_TYPE(PrimitiveSceneProxy)
    protected:
        int32_t PrimitiveId;
        Core::AABB ProxyBounds;
        Core::Mat4 LocalToWorld;
		Core::Mat4 WorldToLocal;

        // 常量数据资源
        void* PrimitiveUniformBufferData = nullptr;

        uint32_t bVisible : 1;
        uint32_t bCastShadow : 1;
        uint32_t bOpaque : 1;
        uint32_t RenderFlags : 29;
    };
} // namespace Engine