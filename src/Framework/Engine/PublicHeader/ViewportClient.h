#pragma once
#include "Viewport.h"
namespace Engine {
    // Engine/ViewportClient.h
    class ENGINE_API ViewportClient
    {
    public:
        virtual ~ViewportClient() = default;

        // 核心：让 Client 填充 ViewFamily
        virtual void Draw(Viewport* InViewport) = 0;

        // Resize 通知
        virtual void OnViewportResized(int Width, int Height) {}

        // 输入（以后可扩展）
        virtual void Tick(float DeltaTime) {}
    };



}