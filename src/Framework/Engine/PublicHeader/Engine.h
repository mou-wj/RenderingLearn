#pragma once
#include "EngineExport.h"
namespace Engine {

    class ENGINE_API Engine
    {
    public:
        Engine() = default;
        virtual ~Engine() = default;

        virtual void Init() = 0;
        virtual void Tick(float deltaTime) = 0;
        virtual void Shutdown() = 0;
    protected:
    };

    ENGINE_API Engine* GetEngineInstance();



}