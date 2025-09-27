#pragma once
#include "RenderInterface.h"
#include "RenderThread.h"

namespace Renderer{
	using RenderCore::RenderInterface;
    using RenderCore::RenderThread;

class SceneRenderer {
public:
	virtual ~SceneRenderer() = default;
    virtual void Init() {};
    virtual void Render() = 0;  

};


class SceneRenderInterface : public RenderInterface
{
public:
    SceneRenderInterface() = default;
    virtual ~SceneRenderInterface() override = default;

    void Init() override;
    void Resize(uint32_t width, uint32_t height) override;
    void Render() override;
    void Update() override;
    void Destroy() override;

private:
    uint32_t Width = 0;
    uint32_t Height = 0;
    // 你可以在这里添加更多成员变量，如场景数据、渲染资源等
    RenderThread renderThread;
    std::shared_ptr<SceneRenderer> sceneRenderer;
};

} // namespace Renderer

