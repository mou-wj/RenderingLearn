#pragma once
#include "RenderResource.h"
#include "EngineExport.h"
namespace Engine {
    class ENGINE_API Viewport : RenderCore::RenderTarget
    {
    public:
        virtual ~Viewport() = default;

        virtual int GetWidth() const = 0;
        virtual int GetHeight() const = 0;

    };
}