#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <mutex>
#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "PathInfo.h"

namespace Core {

// 简单的模块接口，仿 UE 的 Module
class CORE_API Module
{
public:
    virtual ~Module() = default;
    // Called when the module is loaded / should start up.
    virtual void StartupModule() = 0;

    // Called when the module is unloaded / should shut down.
    virtual void ShutdownModule() = 0;

    // Called before the module is unloaded when hot-reloading / reload ordering is needed.
    virtual void PreUnloadCallback() { }

    // Called after the module has been loaded and initialized.
    virtual void PostLoadCallback() { }

    // Query functions
    virtual bool IsLoaded() const = 0;
    virtual const std::string& GetModuleName() const = 0;
};



using ModulePtr = std::shared_ptr<Module>;

// 简单的模块管理器，负责注册/查找模块实例
class CORE_API ModuleManager
{
public:
    static ModuleManager& Get()
    {
        static ModuleManager Instance;
        return Instance;
    }

    //加载模块
    void LoadModule(const std::string& name) {
        #if defined(_WIN32)
            std::string path = GetExecutableDir() + "/" + name + ".dll";
            ::LoadLibraryA(path.c_str());
        #else
            std::string path = GetExecutableDir() + "/" + name + ".so";
            return dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
        #endif
    }

    // 注册模块实例（通常在模块创建时调用）
    void RegisterModule(const std::string& name, ModulePtr module)
    {
        if (!module) return;
        std::lock_guard<std::mutex> lg(Mutex);
        Modules[name] = module;
    }

    // 注销模块
    void UnregisterModule(const std::string& name)
    {
        std::lock_guard<std::mutex> lg(Mutex);
        Modules.erase(name);
    }

    // 获取已注册模块（未找到返回 nullptr）
    ModulePtr GetModule(const std::string& name)
    {
        std::lock_guard<std::mutex> lg(Mutex);
        auto it = Modules.find(name);
        return it != Modules.end() ? it->second : nullptr;
    }

    // 启动所有已注册模块
    void StartupAll()
    {
        std::lock_guard<std::mutex> lg(Mutex);
        for (auto& kv : Modules)
        {
            if (kv.second && !kv.second->IsLoaded())
                kv.second->StartupModule();
        }
    }

    // 关闭所有已注册模块
    void ShutdownAll()
    {
        std::lock_guard<std::mutex> lg(Mutex);
        for (auto& kv : Modules)
        {
            if (kv.second && kv.second->IsLoaded())
                kv.second->ShutdownModule();
        }
    }

private:
    ModuleManager() = default;
    ~ModuleManager() = default;

    std::unordered_map<std::string, ModulePtr> Modules;
    std::mutex Mutex;
};

// 方便宏：在模块实现文件中快速注册一个静态模块实例
#define IMPLEMENT_SIMPLE_MODULE(ModuleClass, ModuleName)                         \
    namespace {                                                                  \
        struct ModuleClass##_Registrar                                           \
        {                                                                        \
            ModuleClass##_Registrar()                                            \
            {                                                                    \
                auto inst = std::make_shared<ModuleClass>(ModuleName);           \
                Core::ModuleManager::Get().RegisterModule(ModuleName, inst);     \
                /* 不自动 Startup，由使用方调用 ModuleManager::StartupAll */  \
            }                                                                    \
            ~ModuleClass##_Registrar()                                           \
            {                                                                    \
                Core::ModuleManager::Get().UnregisterModule(ModuleName);         \
            }                                                                    \
        } ModuleClass##_Registrar_Instance;                                      \
    }

} // namespace Core