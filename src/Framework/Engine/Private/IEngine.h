#pragma once
#include "Engine.h"
namespace Engine {
    class ENGINE_API IEngine : public Engine
    {
    public:
        IEngine();
        virtual ~IEngine();

        void Init() override;
        void Tick(float deltaTime) override;
        void Shutdown() override;
    };



}