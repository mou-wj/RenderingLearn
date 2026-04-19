#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <unordered_map>
#include <functional>
#include "RHIResource.h"
#include "ShaderCore.h"
#include "RHIDefine.h"
namespace RenderCore {
	

// 顶点属性描述
struct RENDERCORE_API VertexElement
{
    std::string SemanticName; // 语义名，如POSITION/NORMAL/TEXCOORD
    uint32_t SemanticIndex = 0;
    uint32_t Offset = 0;
    uint32_t Stride = 0;
    RHI::ERHIFormat Format = RHI::ERHIFormat::Unknown;
};

// 顶点布局描述
struct RENDERCORE_API VertexDeclaration
{
    std::vector<VertexElement> Elements;
    uint32_t Stride = 0;
};


// 前向声明
class VertexFactory;
class ShaderType;
class VertexFactoryType;
class ShaderCompilerEnvironment;
struct VertexFactoryShaderPermutationParameters
{
    const VertexFactoryType* VFType;
    const ShaderType* ShaderType;
};

// 顶点工厂类型// 声明函数原型
using VFShouldCompileFunc = std::function<bool(const VertexFactoryShaderPermutationParameters&)>;
using VFModifyEnvFunc = std::function<void(const VertexFactoryShaderPermutationParameters&, ShaderCompilerEnvironment&)>;

class RENDERCORE_API VertexFactoryType
{
public:
    static std::unordered_map<std::string, VertexFactoryType*>& GetRegisterMap() {
        static std::unordered_map<std::string, VertexFactoryType*> Map;
        return Map;
    }

    VertexFactoryType(
        std::string InName,
        std::string InShaderPath,
        VFShouldCompileFunc InShouldCompile,
        VFModifyEnvFunc InModifyEnv
    )
        : Name(std::move(InName))
        , ShaderPath(std::move(InShaderPath))
        , ShouldCompileFunc(std::move(InShouldCompile))
        , ModifyEnvFunc(std::move(InModifyEnv))
    {}

    // 暴露给编译器的接口
    bool ShouldCompile(const VertexFactoryShaderPermutationParameters& Params) const 
    {
        return ShouldCompileFunc ? ShouldCompileFunc(Params) : true;
    }

    void ModifyCompilationEnvironment(const VertexFactoryShaderPermutationParameters& Params, ShaderCompilerEnvironment& OutEnv) const
    {
        if (ModifyEnvFunc) ModifyEnvFunc(Params, OutEnv);
    }

    const std::string& GetName() const { return Name; }
    const std::string& GetShaderPath() const { return ShaderPath; }

private:
    std::string Name;
    std::string ShaderPath;
    VFShouldCompileFunc ShouldCompileFunc;
    VFModifyEnvFunc ModifyEnvFunc;
};




// 顶点工厂基类
class RENDERCORE_API VertexFactory
{
public:
    VertexFactory() = default;
    virtual ~VertexFactory() = default;

    // 设置顶点声明
    void SetDeclaration(const VertexDeclaration& decl) { Declaration = decl; }
    const VertexDeclaration& GetDeclaration() const { return Declaration; }
    
    RHI::RHIVertexDescStateSP GetRHIVertexDescState() const;

    static bool ShouldCompilePermutation(const VertexFactoryShaderPermutationParameters& Parameters) {
        return false;
    }
    static void ModifyCompilationEnvironment(const VertexFactoryShaderPermutationParameters& Parameters, ShaderCompilerEnvironment& OutEnvironment) {

    }

    void SetType(VertexFactoryType* type) { VFType = type; }
    VertexFactoryType* GetType() const { return VFType; }

protected:
    VertexDeclaration Declaration;
    VertexFactoryType* VFType = nullptr; // 指向类型信息
    RHI::RHIVertexDescStateSP RHIVertexDescState;
};

/** * 注册宏：用于在 .cpp 文件中声明并注册一个新的顶点工厂类型
 * @param T             具体的顶点工厂类名 (如 LocalVertexFactory)
 * @param ShaderPath    对应的 .ush 文件路径
 */
#define IMPLEMENT_VERTEX_FACTORY_TYPE(T, ShaderPath) \
    RenderCore::TVertexFactoryType<T> G##T##Type( \
        #T, \
        ShaderPath \
    ); \
    struct F##T##Register { \
        F##T##Register() { \
            RenderCore::VertexFactoryType::GetRegisterMap()[#T] = &G##T##Type; \
        } \
    } G##T##RegisterInstance;


} // namespace RenderCore

