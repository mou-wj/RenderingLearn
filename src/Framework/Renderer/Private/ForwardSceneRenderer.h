#include "SceneRenderer.h"

namespace Renderer {
	class ForwardSceneRenderer : public SceneRenderer {
	public:
		// Ò»Ö¡äÖÈ¾Èë¿Ú
		void Build(RenderCore::RenderGraphBuilder& graphBuilder) override;
	};
}