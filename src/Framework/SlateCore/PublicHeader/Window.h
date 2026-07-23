#pragma once
#include <memory>
#include <string>
#include <functional>
#include <Math.hpp>
#include <SlateViewport.h>
#include "PlatformSurfaceOwner.h"
#include "PlatformSurface.h"

namespace SlateCore {

    class SLATECORE_API Window : public PlatformSurfaceOwner
    {
    public:
        Window(int width, int height, const std::string& title);
        virtual ~Window() override = default;

        // ��������
        virtual bool Initialize();
        void PollEvents();
        void Shutdown();

        // ���ڲ���
        void Show();
        void Hide();
        void Close();

        // ��������
        int GetWidth() const { return Width; }
        int GetHeight() const { return Height; }
        Core::Int2 GetFramebufferSize() const;
        const std::string& GetTitle() const { return Title; }
        void* GetNativeHandle() const override;

        void SetRootWidget(Widget* widget)
        {
            RootWidget = widget;
        }

        Widget* GetRootWidgets() const
        {
            return RootWidget;
        }

    public:
        void OnSurfaceCloseRequested() override;
        bool OnResize(uint32_t width, uint32_t height) override;

        bool OnFocusReceived() override;
        bool OnFocusLost() override;

        bool OnMouseMove(const MouseMoveEvent& event) override;
        bool OnMouseButton(const MouseButtonEvent& event) override;
        bool OnMouseWheel(const MouseWheelEvent& event) override;
        bool OnKeyDown(const KeyEvent& event) override;
        bool OnKeyUp(const KeyEvent& event) override;

    protected:
        PlatformSurface* GetOwnedSurface() const override;

    protected:
        int Width = 0;
        int Height = 0;
        std::string Title;
        Widget* RootWidget = nullptr;
        std::unique_ptr<PlatformSurface> Surface;
    };
	using WindowSP = std::shared_ptr<Window>;


    class SLATECORE_API WindowFactory {
    public:
        WindowFactory() = delete;
        static WindowSP CreateWindowSP(int w, int h, const std::string& title);

        using CreatorFunc = std::function<WindowSP(int, int, const std::string&)>;

    private:
        static CreatorFunc Creator;
    };

#define REGISTER_WINDOW_PLATFORM(PlatformClass) \
    CreatorFunc WindowFactory::Creator = [](int w, int h, const std::string& t) -> WindowSP{ \
        auto win = std::make_shared<PlatformClass>(w, h, t); \
        win->Initialize(); \
        return win; \
    };


}