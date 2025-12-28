#pragma once

#include <memory>
#include "Window/WindowViewport.h"

namespace Runtime {

class Application
{
public:
    Application();
    virtual ~Application();

    virtual void Init();
    virtual void Tick(float DeltaTime);
    virtual void Run();

protected:
    virtual float CalculateDeltaTime();
    WindowViewportSP windowViewport;
};

using ApplicationSP = std::shared_ptr<Application>;

} // namespace Runtime