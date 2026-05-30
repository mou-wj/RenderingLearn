#include "Win32Window.h"
#ifdef _WIN32
#include <Windowsx.h>
namespace Slate {



    Win32Window::Win32Window(int width, int height, const std::string& title)
    {
        Width = width;
        Height = height;
        Title = title;
    }

    bool Win32Window::Initialize()
    {
        WNDCLASS wc = {};
        wc.lpfnWndProc = Win32Window::WndProcSetup;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = "MyUEWindowClass";

        if (!RegisterClass(&wc))
        {
            return false;
        }

        hWnd = CreateWindowEx(
            0,
            wc.lpszClassName,
            Title.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            Width, Height,
            nullptr, nullptr,
            wc.hInstance,
            this
        );

        if (!hWnd)
        {
            return false;
        }

        return true;
    }

    void Win32Window::Show()
    {
        ::ShowWindow(hWnd, SW_SHOW);
    }

    void Win32Window::Hide()
    {
        ::ShowWindow(hWnd, SW_HIDE);
    }

    void Win32Window::Close()
    {
        if (CloseCallback) CloseCallback();
        if (hWnd)
        {
            ::DestroyWindow(hWnd);
            hWnd = nullptr;
        }
    }

    // Native Window / Surface
    void* Win32Window::GetNativeHandle() const
    {
        return hWnd;
    }

    Core::Int2 Win32Window::GetFramebufferSize() const
    {
        HWND hwnd = static_cast<HWND>(GetNativeHandle());
        if (!hwnd) return { 0,0 };


        // 获取客户区逻辑尺寸
        RECT rect;
        GetClientRect(hwnd, &rect);
        int logicalWidth = rect.right - rect.left;
        int logicalHeight = rect.bottom - rect.top;


        // 获取 DPI
        UINT dpiX = 96, dpiY = 96;
        // Win10+ 推荐用 GetDpiForWindow
        HMODULE user32 = LoadLibraryA("user32.dll");
        if (user32)
        {
            typedef UINT(WINAPI* GetDpiForWindowFunc)(HWND);
            GetDpiForWindowFunc pGetDpiForWindow =
                (GetDpiForWindowFunc)GetProcAddress(user32, "GetDpiForWindow");
            if (pGetDpiForWindow)
            {
                dpiX = dpiY = pGetDpiForWindow(hwnd);
            }
            FreeLibrary(user32);
        }


        int fbWidth = static_cast<int>(logicalWidth * dpiX / 96.0f);
        int fbHeight = static_cast<int>(logicalHeight * dpiY / 96.0f);


        return { fbWidth, fbHeight };
    }

    void Win32Window::PollEvents()
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    void Win32Window::Shutdown()
    {
        if (hWnd)
        {
            ::DestroyWindow(hWnd);
            hWnd = nullptr;
        }
    }

    LRESULT CALLBACK
        Win32Window::WndProcSetup(
            HWND hwnd,
            UINT msg,
            WPARAM wParam,
            LPARAM lParam)
    {
        Win32Window* window = nullptr;

        if (msg == WM_NCCREATE)
        {
            CREATESTRUCT* cs =
                reinterpret_cast<CREATESTRUCT*>(lParam);

            window =
                reinterpret_cast<Win32Window*>(
                    cs->lpCreateParams);

            ::SetWindowLongPtr(
                hwnd,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(
                    window));
        }
        else
        {
            window =
                reinterpret_cast<Win32Window*>(
                    ::GetWindowLongPtr(
                        hwnd,
                        GWLP_USERDATA));
        }

        if (window)
        {
            return window->WndProc(
                hwnd,
                msg,
                wParam,
                lParam);
        }

        return ::DefWindowProc(
            hwnd,
            msg,
            wParam,
            lParam);
    }


    LRESULT Win32Window::WndProc(
        HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam)
    {
        switch (msg)
        {
        case WM_CLOSE:
        {
            if (CloseCallback)
            {
                CloseCallback();
            }
            return 0;
        }

        case WM_SIZE:
        {
            Width = LOWORD(lParam);
            Height = HIWORD(lParam);

            if (ResizeCallback)
            {
                ResizeCallback(
                    Width,
                    Height);
            }

            if (RootWidget)
            {
                RootWidget->OnResize(
                    Width,
                    Height);
            }

            return 0;
        }

        case WM_SETFOCUS:
        {
            if (RootWidget)
            {
                RootWidget
                    ->OnFocusReceived();
            }

            return 0;
        }

        case WM_KILLFOCUS:
        {
            if (RootWidget)
            {
                RootWidget
                    ->OnFocusLost();
            }

            return 0;
        }
        }

        if (RouteInputMessage(
            msg,
            wParam,
            lParam))
        {
            return 0;
        }

        return ::DefWindowProc(
            hwnd,
            msg,
            wParam,
            lParam);
    }
    Slate::EKey
        Win32Window::TranslateKey(
            WPARAM key) const
    {
        switch (key)
        {
        case 'W':
            return Slate::EKey::W;

        case 'A':
            return Slate::EKey::A;

        case 'S':
            return Slate::EKey::S;

        case 'D':
            return Slate::EKey::D;

        case 'Q':
            return Slate::EKey::Q;

        case 'E':
            return Slate::EKey::E;

        case VK_SHIFT:
            return Slate::EKey::Shift;

        case VK_CONTROL:
            return Slate::EKey::Ctrl;

        case VK_MENU:
            return Slate::EKey::Alt;

        case VK_SPACE:
            return Slate::EKey::Space;

        case VK_ESCAPE:
            return Slate::EKey::Escape;
        }

        return Slate::EKey::Unknown;
    }

    bool Win32Window::RouteInputMessage(
        UINT msg,
        WPARAM wParam,
        LPARAM lParam)
    {
        if (!RootWidget)
        {
            return false;
        }

        switch (msg)
        {
        case WM_MOUSEMOVE:
        {
            Slate::MouseMoveEvent event;

            event.X =
                GET_X_LPARAM(lParam);

            event.Y =
                GET_Y_LPARAM(lParam);

            static int32_t LastX =
                event.X;

            static int32_t LastY =
                event.Y;

            event.DeltaX =
                event.X - LastX;

            event.DeltaY =
                event.Y - LastY;

            LastX = event.X;
            LastY = event.Y;

            event.Modifiers.Ctrl =
                (::GetKeyState(
                    VK_CONTROL) &
                    0x8000) != 0;

            event.Modifiers.Shift =
                (::GetKeyState(
                    VK_SHIFT) &
                    0x8000) != 0;

            event.Modifiers.Alt =
                (::GetKeyState(
                    VK_MENU) &
                    0x8000) != 0;

            return RootWidget
                ->OnMouseMove(
                    event);
        }

        case WM_MOUSEWHEEL:
        {
            Slate::MouseWheelEvent event;

            event.Delta =
                static_cast<float>(
                    GET_WHEEL_DELTA_WPARAM(
                        wParam))
                / WHEEL_DELTA;

            POINT pt;

            pt.x =
                GET_X_LPARAM(lParam);

            pt.y =
                GET_Y_LPARAM(lParam);

            ::ScreenToClient(
                hWnd,
                &pt);

            event.X = pt.x;
            event.Y = pt.y;

            return RootWidget
                ->OnMouseWheel(
                    event);
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            Slate::MouseButtonEvent event;

            event.X =
                GET_X_LPARAM(lParam);

            event.Y =
                GET_Y_LPARAM(lParam);

            event.Event =
                (msg == WM_LBUTTONDOWN ||
                    msg == WM_RBUTTONDOWN ||
                    msg == WM_MBUTTONDOWN)
                ? Slate::EInputEvent
                ::Pressed
                : Slate::EInputEvent
                ::Released;

            switch (msg)
            {
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
                event.Button =
                    Slate::EMouseButton
                    ::Left;
                break;

            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
                event.Button =
                    Slate::EMouseButton
                    ::Right;
                break;

            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
                event.Button =
                    Slate::EMouseButton
                    ::Middle;
                break;
            }

            return RootWidget
                ->OnMouseButton(
                    event);
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            Slate::KeyEvent event;

            event.Key =
                TranslateKey(
                    wParam);

            event.Event =
                Slate::EInputEvent
                ::Pressed;

            return RootWidget
                ->OnKeyDown(
                    event);
        }

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            Slate::KeyEvent event;

            event.Key =
                TranslateKey(
                    wParam);

            event.Event =
                Slate::EInputEvent
                ::Released;

            return RootWidget
                ->OnKeyUp(
                    event);
        }
        }

        return false;
    }


    WindowFactory::CreatorFunc WindowFactory::Creator = [](int w, int h, const std::string& t) -> WindowSP { 
        auto win = std::make_shared<Win32Window>(w,h,t);
        win->Initialize(); 
        return win; 
        };




}
#endif