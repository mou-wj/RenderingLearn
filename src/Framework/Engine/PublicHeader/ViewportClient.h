#pragma once
#include "Viewport.h"
namespace Engine {
	class ENGINE_API ViewportClient {
	public:
		ViewportClient(ViewportClient* InViewport);
		virtual ~ViewportClient();
		virtual void Draw(Viewport* InViewport) = 0;

	};



}