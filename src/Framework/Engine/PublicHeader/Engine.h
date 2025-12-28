#pragma once
#include "EngineExport.h"
namespace Engine {
    class ENGINE_API EngineLoop
    {
    public:
        static void Init();
        static void Run();
        static void Tick();
        static void Shutdown();
    };




}