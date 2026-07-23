#include "EventHandler.h"

namespace SlateCore
{
    EventHandler::~EventHandler() = default;

    bool EventHandler::OnMouseMove(
        const MouseMoveEvent&)
    {
        return false;
    }

    bool EventHandler::OnMouseButton(
        const MouseButtonEvent&)
    {
        return false;
    }

    bool EventHandler::OnMouseWheel(
        const MouseWheelEvent&)
    {
        return false;
    }

    bool EventHandler::OnKeyDown(
        const KeyEvent&)
    {
        return false;
    }

    bool EventHandler::OnKeyUp(
        const KeyEvent&)
    {
        return false;
    }

    bool EventHandler::OnFocusReceived()
    {
        return false;
    }

    bool EventHandler::OnFocusLost()
    {
        return false;
    }

    bool EventHandler::OnResize(
        uint32_t,
        uint32_t)
    {
        return false;
    }
}
