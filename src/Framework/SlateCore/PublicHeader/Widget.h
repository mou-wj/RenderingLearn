#pragma once

#include "TypeIDCast.h"
#include "EventHandler.h"
#include "PlatformSurfaceOwner.h"
#include <memory>

namespace SlateCore
{
    struct WidgetGeometry
    {
        float X = 0;
        float Y = 0;

        float Width = 100;
        float Height = 100;
    };

    enum class EVisibility
    {
        Visible,
        Hidden,
        Collapsed
    };

    class Widget : public EventHandler
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
            SetGeometry(
                Geometry.X,
                Geometry.Y,
                width,
                height);
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
        void SetGeometry(
            float x,
            float y,
            float width,
            float height)
        {
            bool posChanged = false;
            if (x != Geometry.X || y != Geometry.Y) {
                posChanged = true;
            }
            Geometry.X = x;
            Geometry.Y = y;
            if (posChanged) {
                OnSetPosition(x, y);
            }


			bool sizeChanged = false;
            if (width != Geometry.Width || height != Geometry.Height) {
                sizeChanged = true;
            }
            Geometry.Width = width;
            Geometry.Height = height;
            if (sizeChanged) {
                OnSetSize(width, height);
            }

        }

        const WidgetGeometry&
            GetGeometry() const
        {
            return Geometry;
        }

        void SetVisibility(
            EVisibility visibility)
        {
            if (Visibility == visibility)
                return;
            Visibility =
                visibility;
            OnVisibilityChanged();
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
        virtual void OnVisibilityChanged()
        {
        }
        virtual void OnSetSize(float width, float height)
        {
        }
        virtual void OnSetPosition(float x, float y)
        {
        }
    };
    class PlatformSurface;
    class SLATECORE_API NativeWidget : public Widget, public PlatformSurfaceOwner
    {
    public:
        explicit NativeWidget(PlatformSurfaceOwner* parentOwner = nullptr);
        virtual ~NativeWidget() override;

        void SetParentOwnerSource(PlatformSurfaceOwner* parentOwner);
        PlatformSurfaceOwner* GetParentOwnerSource() const;

        void SetNativeHandle(void* nativeHandle);

        bool HasParentNativeRelationship() const;
        void* GetParentNativeHandle() const;
        void* GetNativeHandle() const override;
    protected:
        bool InitializeOwnedSurface();
        PlatformSurface* GetOwnedSurface() const override;
        void* NativeHandle = nullptr;
        std::unique_ptr<PlatformSurface> OwnedSurface;
    };
}