#pragma once
#include "RenderResource.h"
#include "EngineExport.h"

namespace Engine {
    class ViewportClient;
    class RenderTarget : public RenderCore::RenderResource {
	public:
		RHI::RHITexture* GetRenderTarget() {
			return RenderTarget.get();
		}

        RHI::RHITextureSP RenderTarget;

    };

    // Engine/Viewport.h
    class ENGINE_API Viewport : public RenderTarget
    {
    public:
        Viewport(ViewportClient* InClient);
        virtual ~Viewport() = default;


        // 渲染入口（由 SceneViewport 实现）
        virtual void Draw() = 0;

    protected:
        ViewportClient* Client = nullptr;
        
    };
}