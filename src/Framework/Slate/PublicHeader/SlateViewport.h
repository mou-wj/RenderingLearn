#pragma once
namespace Slate {

    class SLATE_API SlateViewport
    {
    public:
        virtual ~SlateViewport() = default;

        // ³ß´ç
        virtual int GetWidth() const = 0;
        virtual int GetHeight() const = 0;
        virtual void Resize(int Width, int Height) = 0;
        virtual void* GetViewportRenderTargetTexture() const = 0;

    };

    using SlateViewportSP = std::shared_ptr<SlateViewport>;

}