#pragma once 
#include "Viewport.h"
#include "SlateViewport.h"
#include "Math.hpp"
namespace Engine {
    // Engine/SceneViewport.h
    class ENGINE_API SceneViewport final : public Viewport,public SlateCore::SlateViewport
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
            const SlateCore::MouseMoveEvent&
            Event) override;

        bool OnMouseButton(
            const SlateCore::MouseButtonEvent&
            Event) override;

        bool OnMouseWheel(
            const SlateCore::MouseWheelEvent&
            Event) override;

        bool OnKeyDown(
            const SlateCore::KeyEvent&
            Event) override;

        bool OnKeyUp(
            const SlateCore::KeyEvent&
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