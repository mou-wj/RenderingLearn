#include "EngineModule.h"
#include "IEngine.h"
namespace Engine {

    std::unique_ptr<Engine> EngineObj;
    EngineModule::EngineModule() {

    }
    EngineModule::~EngineModule() {

    }

    void EngineModule::StartupModule()
    {
        EngineObj = std::make_unique<IEngine>();
        EngineObj->Init();

        isLoaded = true;
    }

    void EngineModule::ShutdownModule()
    {
        if (EngineObj)
        {
            EngineObj->Shutdown();
            EngineObj.reset();
        }
        isLoaded = false;
    }

    bool EngineModule::IsLoaded() const {
        return isLoaded;
    }


    IMPLEMENT_SIMPLE_MODULE(EngineModule, "Engine")

    Engine* GetEngineInstance() {
        return EngineObj.get();
    }
}