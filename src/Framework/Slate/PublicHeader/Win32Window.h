#include "Window.h"
#ifdef _WIN32
#include <windows.h>
#include <string>

namespace Slate {

    class SLATE_API Win32Window : public Window
    {
    public:
        Win32Window(int width, int height, const std::string& title);
        virtual ~Win32Window() override = default;

        virtual bool Initialize() override;
        virtual void PollEvents() override;
        virtual void Shutdown() override;

        virtual void Show() override;
        virtual void Hide() override;
        virtual void Close() override;

        // Native Window / Surface
        void* GetNativeHandle() const override;
		Core::Int2 GetFramebufferSize() const override;

    private:
        HWND hWnd = nullptr;

        static LRESULT CALLBACK WndProcSetup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
#endif

}