#pragma once

#include "Shader.h"
#include <unordered_map>
#include <mutex>

namespace RenderCore {
    class RENDERCORE_API GlobalShaderType : public ShaderType
    {
    public:
        using ShaderCompiledInitializer = ShaderCompiledInitializer;
        GlobalShaderType(
            const std::string& InName,
            const std::string& InSourceFile,
            const std::string& InEntryPoint,
            RHI::ERHIShaderFrequency InFrequency,
            ModifyCompilationEnvironmentFuncType InModifyCompilationEnvironment,
            ShouldCompilePermutationFuncType InShouldCompilePermutation,
            ConstructCompiledFuncType InConstructCompiled,
            int32_t InTotalPermutationCount = 1,
            const ShaderParametersMetadata* InRootParametersMetadata = nullptr
        )
            : ShaderType(
                InName,
                InSourceFile,
                InEntryPoint,
                InFrequency,
                InModifyCompilationEnvironment,
                InShouldCompilePermutation,
                InConstructCompiled,
                InTotalPermutationCount,
                InRootParametersMetadata,
                EShaderTypeFlag::Global
            )
        {
        }

        virtual ~GlobalShaderType() = default;

    public:

    };

#define DECLARE_GLOBAL_SHADER_TYPE(ClassType) \
    DECLARE_SHADER_TYPE(ClassType)\
    ClassType(const ShaderMetaType::ShaderCompiledInitializer& Initializer) : GlobalShader(Initializer) {}\
    
#define IMPLEMENT_GLOBAL_SHADER_TYPE(ClassType,ShaderPath,ShaderName,EntryPoint,Frequency ) \
    IMPLEMENT_SHADER_TYPE(ClassType,ShaderPath,ShaderName,EntryPoint,Frequency)
/*
 GlobalShader - helper that holds a global registry of ShaderType pointers.
 Shader-derived classes can use the GLOBAL_DECLARE_SHADER_TYPE macro inside their
 class definition to create a static ShaderType instance and register it into
 this global registry during static initialization.
*/
class RENDERCORE_API GlobalShader : public Shader
{
public:
    using ShaderMetaType = GlobalShaderType;
public:
    // 继承父类的构造函数
    GlobalShader(const GlobalShaderType::ShaderCompiledInitializer& Initializer)
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
    Shader* GetShader(ShaderType* shaderType, ShaderPermutationId id);

    /**
     * 初始化：遍历所有注册为 Global 的 ShaderType，并触发编译/加载
     * 建议在引擎初始化 RHI 后调用
     */
    bool Initialize();

	void Clear();

private:
    bool IsInitialized = false;
    // 存储结构：ShaderType -> PermutationID -> ShaderInstance
    std::unordered_map<ShaderType*, std::unordered_map<ShaderPermutationId, ShaderSP>> ShaderMap;
};
extern RENDERCORE_API GlobalShaderMap GShaderMap;


} // namespace RenderCore