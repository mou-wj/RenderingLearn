#include "SlateWidget.h"

#include "PlatformSurface.h"

#ifdef _WIN32
#include "Win32Surface.h"
#endif

namespace SlateCore
{
    SlateWidget::SlateWidget(PlatformSurfaceOwner* parentOwner) : NativeWidget(parentOwner)
    {
        OnVisibilityChanged();
    }

    SlateWidget::~SlateWidget()
    {

    }

    void SlateWidget::OnVisibilityChanged()
    {
        if (!OwnedSurface)
        {
            return;
        }
        if (Visibility == EVisibility::Visible) {
            OwnedSurface->Show();
        }
        else if (Visibility == EVisibility::Hidden) {
            OwnedSurface->Hide();
        }
    }
    void SlateWidget::OnSetSize(float width, float height) {
        SetSize(static_cast<int>(width), static_cast<int>(height));
    }
    void SlateWidget::OnSetPosition(float x, float y) {
        OnSetPosition(static_cast<int>(x), static_cast<int>(y));
    }

    bool SlateWidget::AddChildWidget(SlateViewport* child)
    {
        if (!child)
        {
            ViewportChild = nullptr;
            return true;
        }

        ViewportChild = child;
        return true;
    }

    void SlateWidget::SetViewportChild(SlateViewport* viewport)
    {
        ViewportChild = viewport;
    }

    SlateViewport* SlateWidget::GetViewportChild() const
    {
        return ViewportChild;
    }



    void SlateWidget::Draw()
    {
        if (ViewportChild)
        {
            ViewportChild->Draw();
        }
    }

    bool SlateWidget::OnResize(
        uint32_t width,
        uint32_t height)
    {


        if (!ViewportChild)
        {
            return false;
        }
        
        return ViewportChild->OnResize(width, height);
    }

    bool SlateWidget::OnFocusReceived()
    {
        if (!ViewportChild)
        {
            return false;
        }

        return ViewportChild->OnFocusReceived();
    }

    bool SlateWidget::OnFocusLost()
    {
        if (!ViewportChild)
        {
            return false;
        }

        return ViewportChild->OnFocusLost();
    }

    bool SlateWidget::OnMouseMove(
        const MouseMoveEvent& event)
    {
        if (!ViewportChild)
        {
            return false;
        }

        return ViewportChild->OnMouseMove(event);
    }

    bool SlateWidget::OnMouseButton(
        const MouseButtonEvent& event)
    {
        if (!ViewportChild)
        {
            return false;
        }

        return ViewportChild->OnMouseButton(event);
    }

    bool SlateWidget::OnMouseWheel(
        const MouseWheelEvent& event)
    {
        if (!ViewportChild)
        {
            return false;
        }

        return ViewportChild->OnMouseWheel(event);
    }

    bool SlateWidget::OnKeyDown(
        const KeyEvent& event)
    {
        if (!ViewportChild)
        {
            return false;
        }

        return ViewportChild->OnKeyDown(event);
    }

    bool SlateWidget::OnKeyUp(
        const KeyEvent& event)
    {
        if (!ViewportChild)
        {
            return false;
        }

        return ViewportChild->OnKeyUp(event);
    }
}
