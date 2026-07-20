#pragma once

#include <cstdint>

namespace SlateCore
{
    enum class EInputEvent
    {
        Pressed,
        Released,
        Repeat
    };

    enum class EMouseButton
    {
        Left,
        Right,
        Middle,
        Thumb01,
        Thumb02
    };

    enum class EKey
    {
        Unknown,

        W,
        A,
        S,
        D,
        Q,
        E,

        Shift,
        Ctrl,
        Alt,

        Space,
        Escape,

        MouseLeft,
        MouseRight,
        MouseMiddle
    };

    struct ModifierKeys
    {
        bool Ctrl = false;
        bool Shift = false;
        bool Alt = false;
    };

    struct MouseMoveEvent
    {
        int32_t X = 0;
        int32_t Y = 0;

        int32_t DeltaX = 0;
        int32_t DeltaY = 0;

        ModifierKeys Modifiers;
    };

    struct MouseButtonEvent
    {
        EMouseButton Button = EMouseButton::Left;
        EInputEvent Event = EInputEvent::Pressed;

        int32_t X = 0;
        int32_t Y = 0;

        ModifierKeys Modifiers;
    };

    struct MouseWheelEvent
    {
        float Delta = 0.0f;

        int32_t X = 0;
        int32_t Y = 0;

        ModifierKeys Modifiers;
    };

    struct KeyEvent
    {
        EKey Key = EKey::Unknown;
        EInputEvent Event = EInputEvent::Pressed;

        ModifierKeys Modifiers;
    };

    class SLATECORE_API InputHandler
    {
    public:
        virtual ~InputHandler();

    public:
        virtual bool OnMouseMove(
            const MouseMoveEvent& Event);

        virtual bool OnMouseButton(
            const MouseButtonEvent& Event);

        virtual bool OnMouseWheel(
            const MouseWheelEvent& Event);

        virtual bool OnKeyDown(
            const KeyEvent& Event);

        virtual bool OnKeyUp(
            const KeyEvent& Event);

        virtual bool OnFocusReceived();

        virtual bool OnFocusLost();

        virtual bool OnResize(
            uint32_t Width,
            uint32_t Height);
    };
}