#include "SceneViewport.h"
#include "ViewportClient.h"
#include "RHIApi.h"
#include "RenderThread.h"
#include "RHIDefine.h"
using namespace RHI;
using namespace RenderCore;
namespace Engine{
    // Engine/SceneViewport.cpp
    SceneViewport::SceneViewport(
        ViewportClient* InClient,
        Core::Int2 InSize
    )
        : Viewport(InClient)
        , Width(InSize.x)
        , Height(InSize.y)
    {
        Client = InClient;

        InitRHIResource();

    }

    SceneViewport::~SceneViewport()
    {

    }

    int SceneViewport::GetWidth() const
    {
        return Width;
    }

    int SceneViewport::GetHeight() const
    {
        return Height;
    }

    void SceneViewport::Resize(int InWidth, int InHeight)
    {
        if (Width == InWidth && Height == InHeight)
            return;

        Width = InWidth;
        Height = InHeight;
        ReleaseRHIResource();
        InitRHIResource();

        if (Client)
        {
            Client->OnViewportResized(Width, Height);
        }
    }
    RenderCore::RenderTexture* SceneViewport::GetRenderTarget() {
        return ViewportTexture.get();
    }


    void* SceneViewport::GetViewportRenderTargetTexture() const {
		return ViewportTexture.get();
    }

    void SceneViewport::Draw()
    {
        if (!Client)
            return;
        // 2. 让 Client 构建 View / ViewFamily 并绘制
        Client->Draw(this);
        // 交换
        //auto tmp = RenderTarget;
        //RenderTarget = ViewportTexture;
        //ViewportTexture = tmp;
    }

    void SceneViewport::InitRHIResource()
    {
        RHITextureDesc desc;
        desc.Width = static_cast<uint32_t>(Width);
        desc.Height = static_cast<uint32_t>(Height);
        desc.Depth = 1;
        desc.MipLevels = 1; // 0 通常在 RHI 中表示自动计算全链层级
        desc.ArraySize = 1;

        // 映射格式：因为强制了 STBI_rgb_alpha，这里使用 RGBA8
        // 注意：如果是 UI 图片建议用 RGBA8_UNORM，如果是照片建议用 sRGB
        desc.Format = ERHIFormat::R8G8B8A8_UNorm;

        desc.Type = ERHITextureType::Texture2D;
        desc.SampleCount = 1;
        desc.Usage = ERHITextureCreateFlag::ShaderResource | ERHITextureCreateFlag::RenderTarget | ERHITextureCreateFlag::TransferSrc | ERHITextureCreateFlag::TransferDest;

        // 如果需要生成 Mips，确保 Usage 包含相应的 Flag (如 RenderTarget 或 UAV，取决于 RHI 实现)
        desc.bGenerateMips = false;

        // 3. 调用 RHI 创建纹理
        ViewportTexture = std::make_shared<RenderCore::RenderTexture>(desc);
        ViewportTexture->InitRHIResource();

    }
    void SceneViewport::ReleaseRHIResource()
    {

        //RenderTarget = nullptr;
    }
    bool SceneViewport::OnMouseMove(
        const Slate::MouseMoveEvent&
        Event)
    {
        return Client
            ? Client->OnMouseMove(
                Event)
            : false;
    }

    bool SceneViewport::OnMouseButton(
        const Slate::MouseButtonEvent&
        Event)
    {
        return Client
            ? Client->OnMouseButton(
                Event)
            : false;
    }

    bool SceneViewport::OnMouseWheel(
        const Slate::MouseWheelEvent&
        Event)
    {
        return Client
            ? Client->OnMouseWheel(
                Event)
            : false;
    }

    bool SceneViewport::OnKeyDown(
        const Slate::KeyEvent&
        Event)
    {
        return Client
            ? Client->OnKeyDown(
                Event)
            : false;
    }

    bool SceneViewport::OnKeyUp(
        const Slate::KeyEvent&
        Event)
    {
        return Client
            ? Client->OnKeyUp(
                Event)
            : false;
    }

    bool SceneViewport::
        OnFocusReceived()
    {
        return Client
            ? Client
            ->OnFocusReceived()
            : false;
    }

    bool SceneViewport::
        OnFocusLost()
    {
        return Client
            ? Client
            ->OnFocusLost()
            : false;
    }

    bool SceneViewport::OnResize(
        uint32_t Width,
        uint32_t Height)
    {
        Resize(
            static_cast<int>(
                Width),
            static_cast<int>(
                Height));

        return Client
            ? Client->OnResize(
                Width,
                Height)
            : false;
    }

}
