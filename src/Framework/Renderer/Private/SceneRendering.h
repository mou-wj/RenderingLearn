#pragma once
#include "RenderInterface.h"
#include "RenderThread.h"

namespace Renderer{
	using RenderCore::RenderInterface;
    using RenderCore::RenderThread;

class SceneRenderer {
public:
	virtual ~SceneRenderer() = default;
    virtual void Init() {};
    virtual void Render() = 0;  

};


} // namespace Renderer

