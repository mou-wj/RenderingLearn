#pragma once

#include "Widget.h"
#include <functional>

namespace ImGUISlate {

    class IMGUISLATE_API ImWidget : public SlateCore::Widget
    {
        DECLARE_TYPE_ID_DERIVED_TYPE(ImWidget, SlateCore::Widget)

    public:
        using DrawCallback = std::function<void(int x, int y, int w,int h)>;

        ImWidget() = default;
        explicit ImWidget(DrawCallback callback);

        void SetDrawCallback(DrawCallback callback);

        virtual void Draw() override;
        virtual bool OnMouseMove(const SlateCore::MouseMoveEvent& event) override;
        virtual bool OnMouseButton(const SlateCore::MouseButtonEvent& event) override;
        virtual bool OnMouseWheel(const SlateCore::MouseWheelEvent& event) override;
        virtual bool OnKeyDown(const SlateCore::KeyEvent& event) override;
        virtual bool OnKeyUp(const SlateCore::KeyEvent& event) override;
        virtual bool OnFocusReceived() override;
        virtual bool OnFocusLost() override;
        virtual bool OnResize(uint32_t width, uint32_t height) override;

    private:
        DrawCallback DrawHandler;
    };
}