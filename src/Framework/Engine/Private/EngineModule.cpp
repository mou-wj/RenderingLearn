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
    }

    void EngineModule::ShutdownModule()
    {
        if (EngineObj)
        {
            EngineObj->Shutdown();
            EngineObj.reset();
        }
    }

    bool EngineModule::IsLoaded() const {
        return false;
    }


    IMPLEMENT_SIMPLE_MODULE(EngineModule, "Engine")

    Engine* GetEngineInstance() {
        return EngineObj.get();
    }
}