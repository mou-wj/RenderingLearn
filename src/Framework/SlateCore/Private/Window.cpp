#include "Window.h"

#ifdef _WIN32
#include "Win32Surface.h"
#endif

namespace SlateCore {

    Window::Window(
        int width,
        int height,
        const std::string& title)
        : Width(width)
        , Height(height)
        , Title(title)
    {
    }

    bool Window::Initialize()
    {
        if (Surface)
        {
            return true;
        }

#ifdef _WIN32
        Surface = std::make_unique<Win32Surface>(
            Width,
            Height,
            Title,
            this);
        return Surface->Initialize();
#else
        return false;
#endif
    }

    void Window::PollEvents()
    {
        if (Surface)
        {
            Surface->PollEvents();
        }
    }

    void Window::Shutdown()
    {
        if (Surface)
        {
            Surface->Shutdown();
        }
    }

    void Window::Show()
    {
        if (Surface)
        {
            Surface->Show();
        }
    }

    void Window::Hide()
    {
        if (Surface)
        {
            Surface->Hide();
        }
    }

    void Window::Close()
    {
        if (Surface)
        {
            Surface->Close();
        }
    }

    Core::Int2 Window::GetFramebufferSize() const
    {
        if (!Surface)
        {
            return { 0, 0 };
        }

        return Surface->GetFramebufferSize();
    }

    void* Window::GetNativeHandle() const
    {
        if (!Surface)
        {
            return nullptr;
        }

        return Surface->GetNativeHandle();
    }

    PlatformSurface* Window::GetOwnedSurface() const
    {
        return Surface.get();
    }

    void Window::OnSurfaceCloseRequested()
    {
        if (CloseCallback)
        {
            CloseCallback();
        }
    }

    bool Window::OnResize(
        uint32_t width,
        uint32_t height)
    {
        Width = static_cast<int>(width);
        Height = static_cast<int>(height);

        if (ResizeCallback)
        {
            ResizeCallback(Width, Height);
        }

        if (RootWidget)
        {
            RootWidget->OnResize(width, height);
        }

        return RootWidget != nullptr;
    }

    bool Window::OnFocusReceived()
    {
        if (!RootWidget)
        {
            return false;
        }

        return RootWidget->OnFocusReceived();
    }

    bool Window::OnFocusLost()
    {
        if (!RootWidget)
        {
            return false;
        }

        return RootWidget->OnFocusLost();
    }

    bool Window::OnMouseMove(
        const MouseMoveEvent& event)
    {
        if (!RootWidget)
        {
            return false;
        }

        return RootWidget->OnMouseMove(event);
    }

    bool Window::OnMouseButton(
        const MouseButtonEvent& event)
    {
        if (!RootWidget)
        {
            return false;
        }

        return RootWidget->OnMouseButton(event);
    }

    bool Window::OnMouseWheel(
        const MouseWheelEvent& event)
    {
        if (!RootWidget)
        {
            return false;
        }

        return RootWidget->OnMouseWheel(event);
    }

    bool Window::OnKeyDown(
        const KeyEvent& event)
    {
        if (!RootWidget)
        {
            return false;
        }

        return RootWidget->OnKeyDown(event);
    }

    bool Window::OnKeyUp(
        const KeyEvent& event)
    {
        if (!RootWidget)
        {
            return false;
        }

        return RootWidget->OnKeyUp(event);
    }


    WindowSP WindowFactory::CreateWindowSP(int w, int h, const std::string& title) {
        return Creator(w, h, title);
    }


    WindowFactory::CreatorFunc WindowFactory::Creator = [](int w, int h, const std::string& t) -> WindowSP {
        auto win = std::make_shared<Window>(w, h, t);
        win->Initialize();
        return win;
    };


}