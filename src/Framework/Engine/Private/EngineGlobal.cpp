#include "EngineGlobal.h"

namespace Engine {
    RenderCore::RenderInterface* GetRenderModuleInstance() {
        static RenderCore::RenderInterface* instance = nullptr;
        if (instance == nullptr) {
            instance = dynamic_cast<RenderCore::RenderInterface*>(Core::ModuleManager::Get().GetModule("Renderer").get());
        }
        return instance;
    }



}