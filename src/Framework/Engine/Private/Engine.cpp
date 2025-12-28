#include "Engine.h"
#include "ApplicationBase.h"
namespace Engine {

	void EngineLoop::Init() {
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