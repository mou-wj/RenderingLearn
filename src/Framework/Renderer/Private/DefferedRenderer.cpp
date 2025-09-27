#include "DefferedRenderer.h"
#include <iostream>
namespace Renderer {
    void DefferedRenderer::Init() {
        // 初始化渲染资源、场景等
        std::cout << "DefferedRenderer::Init()" << std::endl;
    }
    void DefferedRenderer::Render() {
        // 执行延迟渲染流程
        std::cout << "DefferedRenderer::Render()" << std::endl;
    }
}