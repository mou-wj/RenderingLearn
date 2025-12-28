#pragma once 
#include "Viewport.h"
namespace Engine {
	class ENGINE_API SceneViewport : public Viewport {
	public:
		SceneViewport();
		virtual ~SceneViewport() = default;
		virtual int GetWidth() const override;
		virtual int GetHeight() const override;
	};




}