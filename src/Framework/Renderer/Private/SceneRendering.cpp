#include "SceneRendering.h"
#include <iostream>

namespace Renderer {

void SceneRenderInterface::Init() {
    // 初始化渲染资源、场景等
    std::cout << "SceneRenderInterface::Init()" << std::endl;
	renderThread.Start();
}

void SceneRenderInterface::Resize(uint32_t width, uint32_t height) {
    Width = width;
    Height = height;
    // 处理视口、缓冲区等资源的重建
    std::cout << "SceneRenderInterface::Resize() to " << Width << "x" << Height << std::endl;
}

void SceneRenderInterface::Render() {
    // 执行渲染流程
    std::cout << "SceneRenderInterface::Render()" << std::endl;
	renderThread.EnqueueCommand(RenderCore::RenderCommand("RenderScene", [this]() {
		if (sceneRenderer) {
			sceneRenderer->Render();
		}
		}));
}

void SceneRenderInterface::Update() {
    // 更新场景、动画等
    std::cout << "SceneRenderInterface::Update()" << std::endl;
}

void SceneRenderInterface::Destroy() {
    // 释放资源
    std::cout << "SceneRenderInterface::Destroy()" << std::endl;
}

} // namespace Renderer