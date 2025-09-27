#include "SceneRendering.h"
#include "RenderGraphBuilder.h"

namespace Renderer {
	class DefferedRenderer : public SceneRenderer {
	public:
		DefferedRenderer() = default;
		virtual ~DefferedRenderer() override = default;
		void Init() override;
		void Render() override;
	private:
		RenderCore::RenderGraphBuilder graphBuilder;
	};
}