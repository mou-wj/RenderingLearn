#pragma once
#include <memory>
#include <string>
#include <functional>
#include <Math.hpp>
#include <SlateViewport.h>

namespace SlateCore {

    // 事件回调类型
    using WindowCloseCallback = std::function<void()>;
    using WindowResizeCallback = std::function<void(int width, int height)>;

    class SLATECORE_API Window
    {
    public:
        virtual ~Window() = default;

        // 生命周期
        virtual bool Initialize() = 0;
        virtual void PollEvents() = 0;
        virtual void Shutdown() = 0;

        // 窗口操作
        virtual void Show() = 0;
        virtual void Hide() = 0;
        virtual void Close() = 0;

        // 设置回调
        void SetCloseCallback(WindowCloseCallback cb) { CloseCallback = cb; }
        void SetResizeCallback(WindowResizeCallback cb) { ResizeCallback = cb; }

        // 访问属性
        int GetWidth() const { return Width; }
        int GetHeight() const { return Height; }
        virtual Core::Int2 GetFramebufferSize() const = 0;
        const std::string& GetTitle() const { return Title; }
        virtual void* GetNativeHandle() const = 0;
        void SetRootWidget(Widget* viewport) { RootWidget = viewport; }
		Widget* GetRootWidgets() const { return RootWidget; }
    protected:
        int Width = 0;
        int Height = 0;
        std::string Title;
        Widget* RootWidget = nullptr;
        WindowCloseCallback CloseCallback;
        WindowResizeCallback ResizeCallback;
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
        auto win = std::make_shared<PlatformClass>(); \
        win->Initialize(); \
        return win; \
    };


}