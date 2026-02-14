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
        auto tmp = RenderTarget;
        RenderTarget = ViewportTexture;
        ViewportTexture = tmp;
    }

    void SceneViewport::InitRHIResource()
    {
        //初始化RenderTarget
        RHITextureDesc desc;
        desc.Width = Width;
        desc.Height = Height;
        desc.Format = ERHIFormat::R8G8B8A8_UNorm;
        desc.Usage = ERHITextureCreateFlags::RenderTarget;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.SampleCount = 1;
        RenderTarget  = GRHIApi->CreateTexture(desc);

		ViewportTexture = GRHIApi->CreateTexture(desc);

    }
    void SceneViewport::ReleaseRHIResource()
    {

        RenderTarget = nullptr;
    }


}
