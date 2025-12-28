#include "Application.h"

namespace Runtime {

Application::Application() = default;
Application::~Application() = default;

void Application::Init() {
    // 应用初始化逻辑
}

void Application::Tick(float DeltaTime) {
    // 每帧更新逻辑
    windowViewport->Tick(DeltaTime);
}

void Application::Run() {
    Init();
    bool bRunning = true;
    while (bRunning) {
        float DeltaTime = CalculateDeltaTime();
        Tick(DeltaTime);
        // 可扩展退出条件
        // bRunning = false; // 示例：实际应由外部事件控制
    }
}

float Application::CalculateDeltaTime() {
    // 简单返回固定帧间隔
    return 1.0f / 60.0f;
}

} // namespace Runtime