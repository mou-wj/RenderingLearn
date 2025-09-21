#include "SceneRenderer.h"
#include <iostream>

namespace Renderer {

void SceneRenderer::Init() {
    // 初始化渲染资源、场景等
    std::cout << "SceneRenderer::Init()" << std::endl;
}

void SceneRenderer::Resize(uint32_t width, uint32_t height) {
    Width = width;
    Height = height;
    // 处理视口、缓冲区等资源的重建
    std::cout << "SceneRenderer::Resize() to " << Width << "x" << Height << std::endl;
}

void SceneRenderer::Render() {
    // 执行渲染流程
    std::cout << "SceneRenderer::Render()" << std::endl;
}

void SceneRenderer::Update() {
    // 更新场景、动画等
    std::cout << "SceneRenderer::Update()" << std::endl;
}

void SceneRenderer::Destroy() {
    // 释放资源
    std::cout << "SceneRenderer::Destroy()" << std::endl;
}

} // namespace Renderer