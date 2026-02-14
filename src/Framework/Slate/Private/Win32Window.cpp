#include "Win32Window.h"
namespace Slate {

#ifdef _WIN32

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

    LRESULT CALLBACK Win32Window::WndProcSetup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_NCCREATE)
        {
            CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
            Win32Window* window = reinterpret_cast<Win32Window*>(cs->lpCreateParams);
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
            return window->WndProc(hwnd, msg, wParam, lParam);
        }
        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

    LRESULT Win32Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_CLOSE:
            if (CloseCallback) CloseCallback();
            ::DestroyWindow(hwnd);
            hWnd = nullptr;
            return 0;

        case WM_SIZE:
            Width = LOWORD(lParam);
            Height = HIWORD(lParam);
            if (ResizeCallback) ResizeCallback(Width, Height);
            return 0;
        }

        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

    WindowFactory::CreatorFunc WindowFactory::Creator = [](int w, int h, const std::string& t) -> WindowSP { 
        auto win = std::make_shared<Win32Window>(w,h,t);
        win->Initialize(); 
        return win; 
        };

#endif


}