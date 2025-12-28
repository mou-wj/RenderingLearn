#pragma once

#include <string>
#include <memory>
#include <functional>
#include <cstdint>

namespace Runtime {

class WindowViewport
{
public:
    WindowViewport();
    virtual ~WindowViewport();

    // 初始化视口
    virtual void Init(uint32_t width, uint32_t height);

    // 设置视口尺寸
    virtual void Resize(uint32_t width, uint32_t height);

    // 获取视口宽高
    uint32_t GetWidth() const { return Width; }
    uint32_t GetHeight() const { return Height; }

    void Tick(float deltaTime);

    // 执行渲染
    void Render();

    // 获取窗口句柄（如HWND，平台相关，可选）
    virtual void* GetNativeHandle() const { return NativeHandle; }

protected:
    uint32_t Width = 0;
    uint32_t Height = 0;
    void* NativeHandle = nullptr;
};

using WindowViewportSP = std::shared_ptr<WindowViewport>;

} // namespace Runtime