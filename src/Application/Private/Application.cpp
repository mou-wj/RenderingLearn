#include "Application.h"
#include "SlateRHIModule.h"
namespace App {

Application::Application()
{
    SetApplication(this);
}

Application::~Application()
{
    Shutdown();
}

// 初始化
bool Application::Initialize()
{
    // 可以在这里初始化 Slate / Input / RHI
    ViewportClient = std::make_unique<AppViewportClient>();
	Renderer = SlateRHIRenderer::GSlateRHIRendererModule->CreateSlateRenderer();
    Window =  CreateWindowSP(800, 600, "My Application");
    return true;
}

bool Application::RequestExit()
{
    return QuitFlag;
}

// 关闭
void Application::Shutdown()
{
    MainViewport.reset();
    ViewportClient.reset();
    Window = nullptr;
}

// 创建窗口
Slate::WindowSP Application::CreateWindowSP(int Width, int Height, const char* Title)
{
    // 这里假设你有具体 Window 派生类，例如 Win32Window / GLFWWindow
    Slate::WindowSP NewWindow = Slate::WindowFactory::CreateWindowSP(Width, Height, Title);
    NewWindow->Initialize(); // 初始化 OS 窗口
    NewWindow->Show();
    NewWindow->SetResizeCallback([this](int W, int H) {
        if (MainViewport)
            MainViewport->Resize(W, H);
        });
    Renderer->CreateViewport(NewWindow.get());
    auto framebufferSize = NewWindow->GetFramebufferSize();

    // 同步创建 SceneViewport
    if (!MainViewport)
    {
        MainViewport = std::make_unique<Engine::SceneViewport>(ViewportClient.get(), framebufferSize);
    }
	NewWindow->AddWidget({ 0,0,framebufferSize.x,framebufferSize.y, MainViewport.get() });

    return NewWindow;
}



// TickFrame
void Application::TickFrame()
{
    // Poll Window / Input

    Window->PollEvents();

    // Draw 主 SceneViewport
    if (MainViewport)
        MainViewport->Draw();
    Renderer->Render(Window.get());

}


} // namespace Runtime