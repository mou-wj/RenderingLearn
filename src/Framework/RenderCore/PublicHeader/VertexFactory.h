#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <unordered_map>
#include "RHIResource.h"
#include "RHIDefine.h"

namespace RenderCore {
    using namespace RHI;
    
// 顶点属性描述
struct VertexElement
{
    std::string SemanticName; // 语义名，如POSITION/NORMAL/TEXCOORD
    uint32_t SemanticIndex = 0;
    uint32_t Offset = 0;
    uint32_t Stride = 0;
    ERHIFormat Format = ERHIFormat::Unknown;
};

// 顶点布局描述
struct VertexDeclaration
{
    std::vector<VertexElement> Elements;
    uint32_t Stride = 0;
};

// 顶点工厂基类
class VertexFactory
{
public:
    VertexFactory() = default;
    virtual ~VertexFactory() = default;

    // 设置顶点声明
    void SetDeclaration(const VertexDeclaration& decl) { Declaration = decl; }
    const VertexDeclaration& GetDeclaration() const { return Declaration; }

    // 设置顶点数据
    void SetVertexData(const std::vector<uint8_t>& data) { VertexData = data; }
    const std::vector<uint8_t>& GetVertexData() const { return VertexData; }

    // 创建RHI VertexBuffer
    virtual RHIBufferSP CreateRHIVertexBuffer() const;

protected:
    VertexDeclaration Declaration;
    std::vector<uint8_t> VertexData;
};

// 顶点工厂管理器（可用于缓存/复用）
class VertexFactoryManager
{
public:
    static VertexFactoryManager& Get();

    std::shared_ptr<VertexFactory> GetOrCreateFactory(const std::string& Key);

private:
    std::unordered_map<std::string, std::shared_ptr<VertexFactory>> FactoryMap;
};

} // namespace RenderCore

