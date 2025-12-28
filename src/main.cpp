
#include "Engine.h"
int main() {

	Engine::EngineLoop::Init();
    Engine::EngineLoop::Run();
	Engine::EngineLoop::Shutdown();
	return 0;
}