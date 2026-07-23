#pragma once

#include <memory>
#include "ApplicationBase.h"
#include "SceneViewport.h"
#include "AppViewportClient.h"
#include "SlateWidget.h"
#include "ImMainWindow.h"
namespace ImGUISlate {
    class ImSlateRenderer;
    class ImWidget;
}

namespace App {

    class APPLICATION_API Application : public SlateCore::ApplicationBase
    {
    public:
        Application();
        virtual ~Application();

        // ��������
        bool Initialize() override;
        bool RequestExit() override;
        void Shutdown() override;

        // Window ����
        std::shared_ptr<ImGUISlate::ImMainWindow> CreateWindowSP(int Width, int Height, const char* Title);

        // ֡ͬ��
        void TickFrame() override;

    protected:
        std::shared_ptr<ImGUISlate::ImMainWindow> Window;
        std::unique_ptr<SlateCore::SlateWidget> SceneSlateWidget;
        std::unique_ptr<Engine::SceneViewport> SceneMainViewport;
        std::unique_ptr<AppViewportClient> SceneViewportClient;
		std::unique_ptr<ImGUISlate::ImWidget> ImGuiWidget;
		bool QuitFlag = false;
    };

using ApplicationSP = std::shared_ptr<Application>;

} // namespace Runtime