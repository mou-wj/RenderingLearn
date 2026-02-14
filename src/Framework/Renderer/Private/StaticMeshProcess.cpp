#include "StaticMeshProcess.h"
#include "Scene.h"
#include "StaticMeshComponent.h"
namespace Renderer {

	void StaticMeshDrawBuild(Engine::SceneViewCollection* Views, RenderCore::RenderGraphBuilder& builder) 
	{
		auto scene = dynamic_cast<Engine::Scene*>(Views->Scene);
		scene->ForEachPrimitiveComponent([](Engine::PrimitiveComponent* Component) {
			if (Component->IsA<Engine::StaticMeshComponent>()) {
				Engine::StaticMeshComponent* StaticMeshComponent = dynamic_cast<Engine::StaticMeshComponent*>(Component);
				

			}
		});

	}


}