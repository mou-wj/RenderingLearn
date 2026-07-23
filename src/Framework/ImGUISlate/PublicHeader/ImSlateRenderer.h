#pragma once

#include "Window.h"
#include "EventHandler.h"
#include <memory>
#include <unordered_map>
#include <list>

struct ImGuiContext;

namespace ImGUISlate {

    class ImWidget;

    class ImSlateRenderer : public SlateCore::EventHandler
    {
    public:
        ImSlateRenderer(SlateCore::Window* window);
        ~ImSlateRenderer();

        void Render();
        void RegisterWidget(ImWidget* widget);

        virtual bool OnMouseMove(
            const SlateCore::MouseMoveEvent& Event)override;

        virtual bool OnMouseButton(
            const SlateCore::MouseButtonEvent& Event)override;

        virtual bool OnMouseWheel(
            const SlateCore::MouseWheelEvent& Event)override;

        virtual bool OnKeyDown(
            const SlateCore::KeyEvent& Event)override;

        virtual bool OnKeyUp(
            const SlateCore::KeyEvent& Event)override;

        virtual bool OnFocusReceived() override;

        virtual bool OnFocusLost()override;

        virtual bool OnResize(
            uint32_t Width,
            uint32_t Height) override;
    private:

        bool EnsureOpenGLContext();
        void DestroyOpenGLContexts();
        void UpdateDisplaySize();

        ImGuiContext* Context = nullptr;
        bool bOpenGLBackendInitialized = false;
        SlateCore::Window* Window = nullptr;
        void* DeviceContext = nullptr;
        void* GLContext = nullptr;
        std::vector<ImWidget*> Widgets;
    };

    IMGUISLATE_API ImSlateRenderer* CreateSlateRenderer(SlateCore::Window* window);
}