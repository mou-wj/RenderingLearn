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
#include "RHICommandList.h"
namespace RenderCore {
	

// 前向声明
class VertexFactory;
class ShaderType;
class VertexFactoryType;
class ShaderCompilerEnvironment;
using VertexFactoryFeatureFlags = uint64_t;
class ShaderParametersMetadata;
struct VertexFactoryShaderPermutationParameters
{
    const VertexFactoryType* VFType;
    const ShaderType* ShaderType;
    VertexFactoryFeatureFlags VertexFactoryFlags;
    RHI::ERHIShaderPlatform Platform;
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
        uint32_t InPermutationTotalCount,
        VFShouldCompileFunc InShouldCompile,
        VFModifyEnvFunc InModifyEnv,
        const ShaderParametersMetadata* InRootParametersMetadata
    )
        : Name(std::move(InName))
        , ShaderPath(std::move(InShaderPath))
        , PermutationTotalCount (InPermutationTotalCount)
        , ShouldCompileFunc(std::move(InShouldCompile))
        , ModifyEnvFunc(std::move(InModifyEnv))
        , RootParametersMetadata(InRootParametersMetadata)
    {
        GetRegisterMap()[Name] = this;
    }

    // 暴露给编译器的接口
    bool ShouldCompile(const VertexFactoryShaderPermutationParameters& Params) const 
    {
        return ShouldCompileFunc(Params);
    }

    void ModifyCompilationEnvironment(const VertexFactoryShaderPermutationParameters& Params, ShaderCompilerEnvironment& OutEnv) const
    {

		std::string virtualIncludePath = "#include \"" + ShaderPath + "\"";
        OutEnv.VirtualIncludes["/Generated/VertexFactory.sf"] = virtualIncludePath;
        ModifyEnvFunc(Params, OutEnv);
    }

    const std::string& GetName() const { return Name; }
    const std::string& GetShaderPath() const { return ShaderPath; }

    std::string Name;
    std::string ShaderPath;
    VFShouldCompileFunc ShouldCompileFunc;
    VFModifyEnvFunc ModifyEnvFunc;
    uint32_t PermutationTotalCount;
    const ShaderParametersMetadata* RootParametersMetadata;
};


struct VertexStreamComponent
{
    RHI::RHIBuffer* Buffer = nullptr;

    uint32_t ComponentOffset = 0;

    uint32_t Stride = 0;

    RHI::ERHIFormat Format = RHI::ERHIFormat::Unknown;

    RHI::ERHIInputRate InputRate = RHI::ERHIInputRate::PerVertex;
};

// 顶点工厂基类
class RENDERCORE_API VertexFactory
{
public:
    VertexFactory() = default;
    virtual ~VertexFactory() = default;
    virtual RenderCore::VertexFactoryType* GetType() const { return nullptr; }
    RHI::RHIVertexDescState* GetRHIVertexDescState() const;
    VertexFactoryFeatureFlags GetVertexFactoryFlags() const { return VertexFactoryFlags; }
    static bool ShouldCompilePermutation(const VertexFactoryShaderPermutationParameters& Parameters) {
        return false;
    }
    static void ModifyCompilationEnvironment(const VertexFactoryShaderPermutationParameters& Parameters, ShaderCompilerEnvironment& OutEnvironment) {

    }

    void Bind(RHI::RHIGraphicCommandList& RHICmdList) const;


protected:
    struct VertexStream
    {
        RHI::RHIBuffer* Buffer = nullptr;

        uint32_t Offset = 0;
        uint32_t Stride = 0;

        uint32_t Binding = 0;
		RHI::ERHIInputRate InputRate = RHI::ERHIInputRate::PerVertex;

        bool operator==(const VertexStream& rhs) const
        {
            return Buffer == rhs.Buffer &&
                Offset == rhs.Offset &&
                Binding == rhs.Binding;
        }
    };


    struct VertexElement
    {
        uint32_t Binding = 0;

        uint32_t Location = 0;

        uint32_t ElementOffset = 0;

        RHI::ERHIFormat Format = RHI::ERHIFormat::Unknown;
    };
protected:

    //
    // 自动生成 Binding + Attribute
    //
    VertexElement AccessStreamComponent(
        const VertexStreamComponent& Component,
        uint32_t AttributeIndex);

    //
    // 创建最终 VertexInputState
    //
    void InitDeclaration(
        const std::vector<VertexElement>& Elements);


    std::vector<VertexStream> Streams;
    RHI::RHIVertexDescState* RHIVertexDescState;
    VertexFactoryFeatureFlags VertexFactoryFlags;
};


} // namespace RenderCore
#define DECLARE_VERTEX_FACTORY_TYPE(FactoryClass) \
public: \
    static RenderCore::VertexFactoryType StaticType; \
    virtual RenderCore::VertexFactoryType* GetType() const override;\
    static uint32_t PermutationTotalCount;

#define IMPLEMENT_VERTEX_FACTORY_TYPE(FactoryClass, ShaderFile,TotalCount)\
RenderCore::VertexFactoryType FactoryClass::StaticType(\
    #FactoryClass, \
    ShaderFile, \
    FactoryClass::PermutationTotalCount,\
    & FactoryClass::ShouldCompilePermutation, \
    & FactoryClass::ModifyCompilationEnvironment ,\
    FactoryClass::GetShaderParameterMetadata() \
); \
\
RenderCore::VertexFactoryType* FactoryClass::GetType() const \
{ \
return &StaticType; \
}\
uint32_t FactoryClass::PermutationTotalCount = TotalCount;

#define IMPLEMENT_VERTEX_FACTORY_TYPE_DEFAULT(FactoryClass, ShaderFile) \
    IMPLEMENT_VERTEX_FACTORY_TYPE(FactoryClass, ShaderFile, 1)
