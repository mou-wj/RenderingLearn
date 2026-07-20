#pragma once

#include "TypeIDCast.h"
#include "InputHandler.h"

namespace SlateCore
{
    struct WidgetGeometry
    {
        float X = 0;
        float Y = 0;

        float Width = 0;
        float Height = 0;
    };

    enum class EVisibility
    {
        Visible,
        Hidden,
        Collapsed
    };

    class Widget : public InputHandler
    {
        DECLARE_TYPE_ID_BASE_TYPE(Widget)

    public:
        virtual ~Widget() = default;

    public:
        virtual void Tick(float dt)
        {
        }

        virtual void Draw() = 0;

        virtual void Resize(
            float width,
            float height)
        {
            SetSize(width, height);
        }

        virtual bool HitTest(
            float x,
            float y) const
        {
            if (Visibility !=
                EVisibility::Visible)
            {
                return false;
            }

            const auto& g =
                Geometry;

            return
                x >= g.X &&
                x <= g.X + g.Width &&
                y >= g.Y &&
                y <= g.Y + g.Height;
        }

    public:
        void SetPosition(
            float x,
            float y)
        {
            Geometry.X = x;
            Geometry.Y = y;
        }

        void SetSize(
            float width,
            float height)
        {
            Geometry.Width = width;
            Geometry.Height = height;
        }

        void SetGeometry(
            float x,
            float y,
            float width,
            float height)
        {
            Geometry.X = x;
            Geometry.Y = y;

            Geometry.Width = width;
            Geometry.Height = height;
        }

        const WidgetGeometry&
            GetGeometry() const
        {
            return Geometry;
        }

        void SetVisibility(
            EVisibility visibility)
        {
            Visibility =
                visibility;
        }

        EVisibility
            GetVisibility() const
        {
            return Visibility;
        }

    protected:
        WidgetGeometry Geometry;

        EVisibility Visibility =
            EVisibility::Visible;
    };
}