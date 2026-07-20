#include "InputHandler.h"

namespace SlateCore
{
    InputHandler::~InputHandler() = default;

    bool InputHandler::OnMouseMove(
        const MouseMoveEvent&)
    {
        return false;
    }

    bool InputHandler::OnMouseButton(
        const MouseButtonEvent&)
    {
        return false;
    }

    bool InputHandler::OnMouseWheel(
        const MouseWheelEvent&)
    {
        return false;
    }

    bool InputHandler::OnKeyDown(
        const KeyEvent&)
    {
        return false;
    }

    bool InputHandler::OnKeyUp(
        const KeyEvent&)
    {
        return false;
    }

    bool InputHandler::OnFocusReceived()
    {
        return false;
    }

    bool InputHandler::OnFocusLost()
    {
        return false;
    }

    bool InputHandler::OnResize(
        uint32_t,
        uint32_t)
    {
        return false;
    }
}