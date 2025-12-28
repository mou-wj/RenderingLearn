#include "Window/WindowViewport.h"

namespace Runtime {

WindowViewport::WindowViewport() = default;

WindowViewport::~WindowViewport() = default;

void WindowViewport::Init(uint32_t width, uint32_t height) {
    Width = width;
    Height = height;
    // 这里可以初始化平台相关窗口或视口资源
	// 例如，使用GLFW创建一个窗口
}

void WindowViewport::Resize(uint32_t width, uint32_t height) {
    Width = width;
    Height = height;
    // 这里可以处理平台相关的窗口或视口尺寸调整
}


void WindowViewport::Tick(float deltaTime)
{
    Render();
}

void WindowViewport::Render() {

}

} // namespace Runtime