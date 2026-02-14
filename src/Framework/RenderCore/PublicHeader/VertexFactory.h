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

// 顶点工厂类型
class RENDERCORE_API VertexFactoryType
{
public:
    using ModifyEnvironmentFunc = std::function<void(ShaderCompilerEnvironment&)>;

    VertexFactoryType(
        std::string name,
        ModifyEnvironmentFunc modifyEnv = nullptr)
        : Name(std::move(name)), ModifyCompilationEnvironment(modifyEnv)
    {
    }

    // 名称标识
    const std::string& GetName() const { return Name; }

    // 调用 VF 提供的接口修改 Shader 编译环境
    void ApplyToEnvironment(ShaderCompilerEnvironment& Env) const
    {
        if (ModifyCompilationEnvironment)
            ModifyCompilationEnvironment(Env);
    }

    size_t GetHash() const { return std::hash<std::string>{}(Name); }

private:
    std::string Name;
    ModifyEnvironmentFunc ModifyCompilationEnvironment;
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

    void SetType(VertexFactoryType* type) { VFType = type; }
    VertexFactoryType* GetType() const { return VFType; }

protected:
    VertexDeclaration Declaration;
    VertexFactoryType* VFType = nullptr; // 指向类型信息
    RHI::RHIVertexDescStateSP RHIVertexDescState;
};

} // namespace RenderCore

