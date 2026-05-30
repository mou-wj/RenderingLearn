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
		RenderCore::RenderTexture* GetRenderTarget() override;
        void* GetViewportRenderTargetTexture() const override;
        void Draw() override;

        void InitRHIResource();
        void ReleaseRHIResource();
    public:
        // Input
        bool OnMouseMove(
            const Slate::MouseMoveEvent&
            Event) override;

        bool OnMouseButton(
            const Slate::MouseButtonEvent&
            Event) override;

        bool OnMouseWheel(
            const Slate::MouseWheelEvent&
            Event) override;

        bool OnKeyDown(
            const Slate::KeyEvent&
            Event) override;

        bool OnKeyUp(
            const Slate::KeyEvent&
            Event) override;

        bool OnFocusReceived()
            override;

        bool OnFocusLost()
            override;

        bool OnResize(
            uint32_t Width,
            uint32_t Height)
            override;
    private:

    private:
		RenderCore::RenderTextureSP ViewportTexture;
        int Width = 0;
        int Height = 0;

    };



}