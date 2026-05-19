#pragma once 
#include "Viewport.h"
#include "SlateViewport.h"
#include "Math.hpp"
namespace Engine {
    // Engine/SceneViewport.h
    class ENGINE_API SceneViewport final : public Viewport,public Slate::SlateViewport
    {
    public:
        SceneViewport(
            ViewportClient* InClient,
            Core::Int2 InSize
        );

        virtual ~SceneViewport();

        // Viewport
        int GetWidth() const override;
        int GetHeight() const override;

        void Resize(int Width, int Height) override;
        void* GetViewportRenderTargetTexture() const override;
        void Draw() override;

        void InitRHIResource();
        void ReleaseRHIResource();
    private:

    private:
		RHI::RHITextureSP ViewportTexture;
        int Width = 0;
        int Height = 0;

    };



}