#pragma once 
#include "Window.h"
namespace Slate {

class SLATE_API ApplicationBase
{
public:
    virtual ~ApplicationBase() = default;

    // 生命周期
    virtual bool Initialize() = 0;
    virtual void PumpMessages() = 0;
    virtual bool ShouldQuit() const = 0;
    virtual void Shutdown() = 0;

    // Window
    virtual WindowSP CreateWindow(
        int Width,
        int Height,
        const char* Title
    ) = 0;

    // 帧同步（非常重要）
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
	static ApplicationBase* GetApplication();
    static void SetApplication(ApplicationBase* InApplication);
private:

};
}