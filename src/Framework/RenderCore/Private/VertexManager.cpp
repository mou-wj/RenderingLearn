#include "VertexFactory.h"

namespace RenderCore {

// VertexFactoryManager 单例实现
VertexFactoryManager& VertexFactoryManager::Get() {
    static VertexFactoryManager Instance;
    return Instance;
}

std::shared_ptr<VertexFactory> VertexFactoryManager::GetOrCreateFactory(const std::string& Key) {
    auto it = FactoryMap.find(Key);
    if (it != FactoryMap.end()) {
        return it->second;
    }
    // 默认创建一个基础 VertexFactory，可根据Key扩展不同类型
    auto factory = std::make_shared<VertexFactory>();
    FactoryMap[Key] = factory;
    return factory;
}

// VertexFactory 创建RHI VertexBuffer的默认实现
RHIBufferSP VertexFactory::CreateRHIVertexBuffer() const {
    // 这里只是示例，实际应调用RHI接口创建缓冲区
    // 例如: return GetGlobalRHIApi()->CreateVertexBuffer(VertexData.data(), VertexData.size(), ...);
    return nullptr;
}

} // namespace RenderCore