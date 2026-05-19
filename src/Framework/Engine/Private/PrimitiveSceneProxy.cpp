#include "PrimitiveSceneProxy.h"
#include <cstring>

namespace Engine {

    PrimitiveSceneProxy::PrimitiveSceneProxy()
        : PrimitiveId(0)
        , bVisible(true)
        , bCastShadow(true)
        , bOpaque(true)
        , RenderFlags(0)
    {
        std::memset(LocalToWorld, 0, sizeof(LocalToWorld));
        LocalToWorld[0] = LocalToWorld[5] = LocalToWorld[10] = LocalToWorld[15] = 1.0f;
    }

    PrimitiveSceneProxy::~PrimitiveSceneProxy() {
        // 注意：像 VertexBuffer、IndexBuffer、PrimitiveUniformBuffer 
        // 它们的生命周期都遵循 RenderResource 系统的延迟释放（如你实现的 ReleaseRHIResource）。
        // Proxy 内部仅作为只读方持有指针，不需要、也不能在这里执行 delete。
    }

} // namespace Engine