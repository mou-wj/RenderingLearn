#include "Engine.h"
#include "ApplicationBase.h"
#include "Module.h"
namespace Engine {

	void EngineLoop::Init() {
		Core::ModuleManager::Get().LoadModule("Renderer");



	}
	void EngineLoop::Run() {
		while (!Slate::ApplicationBase::GetApplication()->ShouldQuit()) {
			EngineLoop::Tick();
		}
	}
	void EngineLoop::Tick() {
		

	}
	void EngineLoop::Shutdown() {
	}
} // namespace EngineEngin