#include "Widget.h"
#include "PlatformSurface.h"
#include "Win32Surface.h"
namespace SlateCore {

    NativeWidget::NativeWidget(PlatformSurfaceOwner* parentOwner)
    {
        SetParent(parentOwner);
        InitializeOwnedSurface();
    }

    NativeWidget::~NativeWidget()
    {
        if (OwnedSurface)
        {
            OwnedSurface->Shutdown();
            OwnedSurface.reset();
        }
    }

    PlatformSurface* NativeWidget::GetOwnedSurface() const
    {
        return OwnedSurface.get();
    }

    void NativeWidget::SetParentOwnerSource(PlatformSurfaceOwner* parentOwner)
    {
        SetParent(parentOwner);
    }

    PlatformSurfaceOwner* NativeWidget::GetParentOwnerSource() const
    {
        return GetParent();
    }

    void NativeWidget::SetNativeHandle(void* nativeHandle)
    {
        NativeHandle = nativeHandle;
    }

    bool NativeWidget::HasParentNativeRelationship() const
    {
        return GetParent() != nullptr;
    }

    void* NativeWidget::GetParentNativeHandle() const
    {
        auto* parentOwner = GetParent();
        if (!parentOwner)
        {
            return nullptr;
        }

        return parentOwner->GetNativeHandle();
    }
    void* NativeWidget::GetNativeHandle() const
    {
        if (OwnedSurface)
        {
            return OwnedSurface->GetNativeHandle();
        }

        return NativeHandle;
    }


    bool NativeWidget::InitializeOwnedSurface()
    {
        if (OwnedSurface)
        {
            return true;
        }

#ifdef _WIN32
        OwnedSurface = std::make_unique<Win32Surface>(
            Geometry.Width,
            Geometry.Height,
            "SlateWidgetSurface",
            this);
#else
        return false;
#endif

        if (!OwnedSurface->Initialize())
        {
            OwnedSurface.reset();
            return false;
        }

        NativeHandle = OwnedSurface->GetNativeHandle();
        OnVisibilityChanged();
        return true;
    }

}