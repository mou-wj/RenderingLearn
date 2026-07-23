#include "Win32Surface.h"

#ifdef _WIN32

#include "PlatformSurfaceOwner.h"
#include <Windowsx.h>

namespace SlateCore
{
    Win32Surface::Win32Surface(
        int width,
        int height,
        const std::string& title,
        PlatformSurfaceOwner* owner)
        : PlatformSurface(width, height, title, owner)
    {
    }

    bool Win32Surface::Initialize()
    {
        if (hWnd)
        {
            return true;
        }

        HWND parentHwnd = nullptr;
        if (Owner)
        {
            auto* parentOwner = Owner->GetParent();
            if (parentOwner)
            {
                parentHwnd = static_cast<HWND>(parentOwner->GetNativeHandle());
            }
        }

        WNDCLASS wc = {};
        wc.lpfnWndProc = Win32Surface::WndProcSetup;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = "SlateCoreWin32Surface";

        if (!RegisterClass(&wc) &&
            GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            return false;
        }

        const DWORD windowStyle =
            parentHwnd
            ? (WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS)
            : WS_OVERLAPPEDWINDOW;

        const int posX = parentHwnd ? 0 : CW_USEDEFAULT;
        const int posY = parentHwnd ? 0 : CW_USEDEFAULT;

        hWnd = CreateWindowEx(
            0,
            wc.lpszClassName,
            Title.c_str(),
            windowStyle,
            posX,
            posY,
            Width,
            Height,
            parentHwnd,
            nullptr,
            wc.hInstance,
            this);

        return hWnd != nullptr;
    }

    void Win32Surface::PollEvents()
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    void Win32Surface::Shutdown()
    {
        if (hWnd)
        {
            ::DestroyWindow(hWnd);
            hWnd = nullptr;
        }
    }

    void Win32Surface::Show()
    {
        ::ShowWindow(hWnd, SW_SHOW);
    }

    void Win32Surface::Hide()
    {
        ::ShowWindow(hWnd, SW_HIDE);
    }

    void Win32Surface::Close()
    {
        if (Owner)
        {
            Owner->OnSurfaceCloseRequested();
        }

        if (hWnd)
        {
            ::DestroyWindow(hWnd);
            hWnd = nullptr;
        }
    }

    void Win32Surface::SetPosition(int x, int y)
    {
        if (!hWnd)
        {
            return;
        }

        ::SetWindowPos(
            hWnd,
            nullptr,
            x,
            y,
            0,
            0,
            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void Win32Surface::SetSize(int width, int height)
    {
        Width = width;
        Height = height;

        if (!hWnd)
        {
            return;
        }

        ::SetWindowPos(
            hWnd,
            nullptr,
            0,
            0,
            width,
            height,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void* Win32Surface::GetNativeHandle() const
    {
        return hWnd;
    }

    Core::Int2 Win32Surface::GetFramebufferSize() const
    {
        HWND hwnd = static_cast<HWND>(GetNativeHandle());
        if (!hwnd)
        {
            return { 0, 0 };
        }

        RECT rect;
        GetClientRect(hwnd, &rect);
        int logicalWidth = rect.right - rect.left;
        int logicalHeight = rect.bottom - rect.top;

        UINT dpiX = 96;
        HMODULE user32 = LoadLibraryA("user32.dll");
        if (user32)
        {
            using GetDpiForWindowFunc = UINT(WINAPI*)(HWND);
            auto pGetDpiForWindow = reinterpret_cast<GetDpiForWindowFunc>(
                GetProcAddress(user32, "GetDpiForWindow"));
            if (pGetDpiForWindow)
            {
                dpiX = pGetDpiForWindow(hwnd);
            }
            FreeLibrary(user32);
        }

        int fbWidth = static_cast<int>(logicalWidth * dpiX / 96.0f);
        int fbHeight = static_cast<int>(logicalHeight * dpiX / 96.0f);
        return { fbWidth, fbHeight };
    }

    LRESULT CALLBACK Win32Surface::WndProcSetup(
        HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam)
    {
        Win32Surface* surface = nullptr;

        if (msg == WM_NCCREATE)
        {
            CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
            surface = reinterpret_cast<Win32Surface*>(cs->lpCreateParams);
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(surface));
        }
        else
        {
            surface = reinterpret_cast<Win32Surface*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (surface)
        {
            return surface->WndProc(hwnd, msg, wParam, lParam);
        }

        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

    LRESULT Win32Surface::WndProc(
        HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam)
    {
        switch (msg)
        {
        case WM_CLOSE:
            if (Owner)
            {
                Owner->OnSurfaceCloseRequested();
            }
            return 0;

        case WM_SIZE:
            Width = LOWORD(lParam);
            Height = HIWORD(lParam);
            if (Owner)
            {
                Owner->OnResize(static_cast<uint32_t>(Width), static_cast<uint32_t>(Height));
            }
            return 0;

        case WM_SETFOCUS:
            if (Owner)
            {
                Owner->OnFocusReceived();
            }
            return 0;

        case WM_KILLFOCUS:
            if (Owner)
            {
                Owner->OnFocusLost();
            }
            return 0;
        }

        if (RouteInputMessage(msg, wParam, lParam))
        {
            return 0;
        }

        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

    EKey Win32Surface::TranslateKey(WPARAM key) const
    {
        switch (key)
        {
        case 'W': return EKey::W;
        case 'A': return EKey::A;
        case 'S': return EKey::S;
        case 'D': return EKey::D;
        case 'Q': return EKey::Q;
        case 'E': return EKey::E;

        case VK_SHIFT: return EKey::Shift;
        case VK_CONTROL: return EKey::Ctrl;
        case VK_MENU: return EKey::Alt;

        case VK_SPACE: return EKey::Space;
        case VK_ESCAPE: return EKey::Escape;
        default: return EKey::Unknown;
        }
    }

    bool Win32Surface::RouteInputMessage(
        UINT msg,
        WPARAM wParam,
        LPARAM lParam)
    {
        if (!Owner)
        {
            return false;
        }

        switch (msg)
        {
        case WM_MOUSEMOVE:
        {
            MouseMoveEvent event;
            event.X = GET_X_LPARAM(lParam);
            event.Y = GET_Y_LPARAM(lParam);

            static int32_t lastX = event.X;
            static int32_t lastY = event.Y;

            event.DeltaX = event.X - lastX;
            event.DeltaY = event.Y - lastY;

            lastX = event.X;
            lastY = event.Y;

            event.Modifiers.Ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
            event.Modifiers.Shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
            event.Modifiers.Alt = (::GetKeyState(VK_MENU) & 0x8000) != 0;

            return Owner->OnMouseMove(event);
        }

        case WM_MOUSEWHEEL:
        {
            MouseWheelEvent event;
            event.Delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;

            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ::ScreenToClient(hWnd, &pt);

            event.X = pt.x;
            event.Y = pt.y;

            return Owner->OnMouseWheel(event);
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            MouseButtonEvent event;

            event.X = GET_X_LPARAM(lParam);
            event.Y = GET_Y_LPARAM(lParam);

            event.Event =
                (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN)
                ? EInputEvent::Pressed
                : EInputEvent::Released;

            switch (msg)
            {
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
                event.Button = EMouseButton::Left;
                break;

            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
                event.Button = EMouseButton::Right;
                break;

            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
                event.Button = EMouseButton::Middle;
                break;
            }

            return Owner->OnMouseButton(event);
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            KeyEvent event;
            event.Key = TranslateKey(wParam);
            event.Event = EInputEvent::Pressed;
            return Owner->OnKeyDown(event);
        }

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            KeyEvent event;
            event.Key = TranslateKey(wParam);
            event.Event = EInputEvent::Released;
            return Owner->OnKeyUp(event);
        }
        }

        return false;
    }
}

#endif
