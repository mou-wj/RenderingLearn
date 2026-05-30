#include "SceneRenderer.h"

namespace Renderer {
	class DefferedSceneRenderer : public SceneRenderer {
	public:
		// Ò»Ö¡äÖÈ¾Èë¿Ú
		void Build(RenderCore::RenderGraphBuilder& graphBuilder) override;
	};
}