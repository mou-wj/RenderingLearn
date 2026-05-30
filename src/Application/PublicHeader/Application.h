#pragma once

#include <memory>
#include "ApplicationBase.h"
#include "SceneViewport.h"
#include "AppViewportClient.h"
namespace App {

    class APPLICATION_API Application : public Slate::ApplicationBase
    {
    public:
        Application();
        virtual ~Application();

        // 生命周期
        bool Initialize() override;
        bool RequestExit() override;
        void Shutdown() override;

        // Window 管理
        Slate::WindowSP CreateWindowSP(int Width, int Height, const char* Title);

        // 帧同步
        void TickFrame() override;

    protected:
        Slate::WindowSP Window;
        std::unique_ptr<Engine::SceneViewport> MainViewport;
        std::unique_ptr<AppViewportClient> ViewportClient;
		bool QuitFlag = false;
    };

using ApplicationSP = std::shared_ptr<Application>;

} // namespace Runtime