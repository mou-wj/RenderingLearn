#pragma once
#include "RenderInterface.h"

namespace Renderer{
	using RenderCore::RenderInterface;

class SceneRenderer : public RenderInterface
{
public:
    SceneRenderer() = default;
    virtual ~SceneRenderer() override = default;

    void Init() override;
    void Resize(uint32_t width, uint32_t height) override;
    void Render() override;
    void Update() override;
    void Destroy() override;

private:
    uint32_t Width = 0;
    uint32_t Height = 0;
    // 你可以在这里添加更多成员变量，如场景数据、渲染资源等
};

} // namespace Renderer

