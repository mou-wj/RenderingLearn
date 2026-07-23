#pragma once
#include "Viewport.h"
#include "EventHandler.h"
namespace Engine {
    // Engine/ViewportClient.h
    class ENGINE_API ViewportClient : public SlateCore::EventHandler
    {
    public:
        virtual ~ViewportClient() = default;

        // ���ģ��� Client ��� ViewFamily
        virtual void Draw(Viewport* InViewport) = 0;

        // Resize ֪ͨ
        virtual void OnViewportResized(int Width, int Height) {}

        // ���루�Ժ����չ��
        virtual void Tick(float DeltaTime) {}
    };



}