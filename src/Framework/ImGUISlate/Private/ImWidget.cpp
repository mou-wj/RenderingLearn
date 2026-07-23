#include "ImWidget.h"

#include "ImGui/imgui.h"

namespace {

    ImGuiKey TranslateKey(SlateCore::EKey key)
    {
        switch (key)
        {
        case SlateCore::EKey::W:
            return ImGuiKey_W;
        case SlateCore::EKey::A:
            return ImGuiKey_A;
        case SlateCore::EKey::S:
            return ImGuiKey_S;
        case SlateCore::EKey::D:
            return ImGuiKey_D;
        case SlateCore::EKey::Q:
            return ImGuiKey_Q;
        case SlateCore::EKey::E:
            return ImGuiKey_E;
        case SlateCore::EKey::Shift:
            return ImGuiKey_LeftShift;
        case SlateCore::EKey::Ctrl:
            return ImGuiKey_LeftCtrl;
        case SlateCore::EKey::Alt:
            return ImGuiKey_LeftAlt;
        case SlateCore::EKey::Space:
            return ImGuiKey_Space;
        case SlateCore::EKey::Escape:
            return ImGuiKey_Escape;
        case SlateCore::EKey::MouseLeft:
            return ImGuiKey_MouseLeft;
        case SlateCore::EKey::MouseRight:
            return ImGuiKey_MouseRight;
        case SlateCore::EKey::MouseMiddle:
            return ImGuiKey_MouseMiddle;
        default:
            return ImGuiKey_None;
        }
    }

    int TranslateMouseButton(SlateCore::EMouseButton button)
    {
        switch (button)
        {
        case SlateCore::EMouseButton::Left:
            return 0;
        case SlateCore::EMouseButton::Right:
            return 1;
        case SlateCore::EMouseButton::Middle:
            return 2;
        case SlateCore::EMouseButton::Thumb01:
            return 3;
        case SlateCore::EMouseButton::Thumb02:
            return 4;
        default:
            return -1;
        }
    }

    void UpdateModifiers(const SlateCore::ModifierKeys& modifiers)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddKeyEvent(ImGuiMod_Ctrl, modifiers.Ctrl);
        io.AddKeyEvent(ImGuiMod_Shift, modifiers.Shift);
        io.AddKeyEvent(ImGuiMod_Alt, modifiers.Alt);
    }
}

namespace ImGUISlate {

    ImWidget::ImWidget(DrawCallback callback)
        : DrawHandler(std::move(callback))
    {
    }

    void ImWidget::SetDrawCallback(DrawCallback callback)
    {
        DrawHandler = std::move(callback);
    }

    void ImWidget::Draw()
    {
        if (Visibility != SlateCore::EVisibility::Visible)
        {
            return;
        }

        if (DrawHandler)
        {
            DrawHandler(Geometry.X, Geometry.Y, Geometry.Width, Geometry.Height);
        }
    }

    bool ImWidget::OnMouseMove(const SlateCore::MouseMoveEvent& event)
    {
        UpdateModifiers(event.Modifiers);
        ImGui::GetIO().AddMousePosEvent(static_cast<float>(event.X), static_cast<float>(event.Y));
        return true;
    }

    bool ImWidget::OnMouseButton(const SlateCore::MouseButtonEvent& event)
    {
        UpdateModifiers(event.Modifiers);

        const int button = TranslateMouseButton(event.Button);
        if (button < 0)
        {
            return false;
        }

        ImGui::GetIO().AddMouseButtonEvent(button, event.Event != SlateCore::EInputEvent::Released);
        return true;
    }

    bool ImWidget::OnMouseWheel(const SlateCore::MouseWheelEvent& event)
    {
        UpdateModifiers(event.Modifiers);
        ImGui::GetIO().AddMouseWheelEvent(0.0f, event.Delta);
        return true;
    }

    bool ImWidget::OnKeyDown(const SlateCore::KeyEvent& event)
    {
        UpdateModifiers(event.Modifiers);

        const ImGuiKey key = TranslateKey(event.Key);
        if (key == ImGuiKey_None)
        {
            return false;
        }

        ImGui::GetIO().AddKeyEvent(key, true);
        return true;
    }

    bool ImWidget::OnKeyUp(const SlateCore::KeyEvent& event)
    {
        UpdateModifiers(event.Modifiers);

        const ImGuiKey key = TranslateKey(event.Key);
        if (key == ImGuiKey_None)
        {
            return false;
        }

        ImGui::GetIO().AddKeyEvent(key, false);
        return true;
    }

    bool ImWidget::OnFocusReceived()
    {
        ImGui::GetIO().AddFocusEvent(true);
        return true;
    }

    bool ImWidget::OnFocusLost()
    {
        ImGui::GetIO().AddFocusEvent(false);
        return true;
    }

    bool ImWidget::OnResize(uint32_t width, uint32_t height)
    {
        Resize(static_cast<float>(width), static_cast<float>(height));
        return true;
    }
}