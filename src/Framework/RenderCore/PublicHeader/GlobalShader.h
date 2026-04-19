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
    // 继承父类的构造函数
    GlobalShader(const ShaderCompiledInitializer& Initializer)
        : Shader(Initializer)
    {
    }

    // Global Shader 通常不需要 VertexFactory
    static bool ShouldCompilePermutation(const ShaderPermutationParameters&Parameters)
    {
        return true;
    }
};
class RENDERCORE_API GlobalShaderMap {
public:
    // 获取一个已编译的 Shader 实例
    ShaderSP GetShader(ShaderType* shaderType, ShaderPermutationId id);

    /**
     * 初始化：遍历所有注册为 Global 的 ShaderType，并触发编译/加载
     * 建议在引擎初始化 RHI 后调用
     */
    bool Initialize();

private:
    bool IsInitialized = false;
    // 存储结构：ShaderType -> PermutationID -> ShaderInstance
    std::unordered_map<ShaderType*, std::unordered_map<ShaderPermutationId, ShaderSP>> ShaderMap;
};

RENDERCORE_API GlobalShaderMap& GetGlobalShaderMap();
#define DECLARE_GLOBAL_SHADER_TYPE(ClassType) \
public: \
    using ShaderMetaType = ClassType; \
    static RenderCore::ShaderType StaticType; \
    /* 每一个 Global Shader 类必须实现的构造函数 */ \
    ClassType(const RenderCore::ShaderCompiledInitializer& Initializer) : RenderCore::GlobalShader(Initializer) {}

#define IMPLEMENT_GLOBAL_SHADER_TYPE(ClassType, ShaderPath, ShaderName, EntryPoint, Frequency) \
    RenderCore::ShaderType ClassType::StaticType( \
        ShaderName, ShaderPath, EntryPoint, Frequency, \
        &ClassType::ModifyShaderCompilerEnvironment, \
        &ClassType::ShouldCompilePermutation, \
        [](const RenderCore::ShaderCompiledInitializer& Initializer) { return new ClassType(Initializer); } \
    ); \
    /* 利用静态变量初始化实现自动注册 */ \
    static RenderCore::ShaderTypeRegister GRegister_##ClassType(&ClassType::StaticType);

} // namespace RenderCore