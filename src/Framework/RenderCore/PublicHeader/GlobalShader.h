#pragma once

#include "Shader.h"
#include <unordered_map>
#include <mutex>

namespace RenderCore {

/*
 GlobalShader - helper that holds a global registry of ShaderType pointers.
 Shader-derived classes can use the GLOBAL_DECLARE_SHADER_TYPE macro inside their
 class definition to create a static ShaderType instance and register it into
 this global registry during static initialization.
*/
class RENDERCORE_API GlobalShader : public Shader
{
public:
    using ShaderTypeMap = std::unordered_map<std::string, RenderCore::ShaderType*>;

    GlobalShader() = delete;
    ~GlobalShader() = default;

    // Register a ShaderType pointer for a given C++ class type name.
    // Returns true on success.
    static bool RegisterTypeInstance(const std::string& classTypeName, RenderCore::ShaderType* typePtr)
    {
        if (!typePtr) return false;
        std::lock_guard<std::mutex> lg(s_Mutex);
        s_GlobalShaderTypes[classTypeName] = typePtr;
        return true;
    }

    // Get read-only access to the registered shader types
    static const ShaderTypeMap& GetGlobalShaderTypes()
    {
        return s_GlobalShaderTypes;
    }
    static RenderCore::ShaderType* GetGlobalShaderType(const std::string& classTypeName) {
        if (s_GlobalShaderTypes.find(classTypeName) != s_GlobalShaderTypes.end()) {
            return s_GlobalShaderTypes[classTypeName];
        }
        return nullptr;
    }

private:
    static ShaderTypeMap s_GlobalShaderTypes;
    static std::mutex    s_Mutex;
    friend class GlobalShaderMap;
};

class RENDERCORE_API GlobalShaderMap {
public:


    ShaderSP GetShader(ShaderType* shaderType, ShaderPermutationId id);
    bool Initialize();
private:
	bool IsInitialized = false;
    std::unordered_map<ShaderType*, std::unordered_map<ShaderPermutationId, ShaderSP>> ShaderMap;

};

RENDERCORE_API GlobalShaderMap& GetGlobalShaderMap();

#define DECLARE_GLOBAL_SHADER_TYPE(ClassType, ShaderPath, Frequency, ShaderName, EntryPoint) \
public: \
    static RenderCore::ShaderType& StaticShaderTypeInstance() { \
        /* function-local static guarantees single instance without needing out-of-class definition */ \
        static RenderCore::ShaderType s_Instance( (ShaderName), (ShaderPath), (EntryPoint),(Frequency), (&ClassType::ModifyShaderCompilerEnvironment),(&ClassType::ShouldCompilePermutation),(ClassType::GetShaderParameterBindingInfo()) ); \
        return s_Instance; \
    } \
private: \
    /* Inline static bool initialized during static init - performs registration */ \
    inline static bool s_##ClassType##_GlobalShaderType_Registered = (RenderCore::GlobalShader::RegisterTypeInstance(#ClassType, &ClassType::StaticShaderTypeInstance()), true); \
public:

//这个宏有问题，先暂时搁置，能弄清楚UE这套机制后再调整
// convenience shortcuts
#define DECLARE_GLOBAL_VERTEX_SHADER(ClassType, ShaderPath, ShaderName) \
    DECLARE_GLOBAL_SHADER_TYPE(ClassType, ShaderPath, RHI::ERHIShaderFrequency::Vertex, ShaderName, "MainVS")

#define DECLARE_GLOBAL_FRAGMENT_SHADER(ClassType, ShaderPath, ShaderName) \
    DECLARE_GLOBAL_SHADER_TYPE(ClassType, ShaderPath, RHI::ERHIShaderFrequency::Fragment, ShaderName, "MainPS")

#define DECLARE_GLOBAL_COMPUTE_SHADER(ClassType, ShaderPath, ShaderName) \
    DECLARE_GLOBAL_SHADER_TYPE(ClassType, ShaderPath, RHI::ERHIShaderFrequency::Compute, ShaderName, "MainCS")

} // namespace RenderCore