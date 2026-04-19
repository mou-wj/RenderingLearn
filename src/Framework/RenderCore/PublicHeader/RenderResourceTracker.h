#pragma once
#include <unordered_map>
#include <memory>
#include "RHIDefine.h"

namespace std {
    template<>
    struct hash<RHI::RHISubresourceRange> {
        size_t operator()(const RHI::RHISubresourceRange& range) const noexcept {
            size_t h = 0;
            h ^= std::hash<uint32_t>{}(range.MipIndex) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint32_t>{}(range.ArraySlice) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<uint32_t>{}(range.PlaneSlice) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
}


namespace RenderCore {

// 资源追踪基类
class RenderResourceTrackerBase {
public:
    virtual ~RenderResourceTrackerBase() = default;
    // 追踪和更新最后的访问Fence
    void UpdateLastAccessFence(const RHI::RHIFence& fence) { LastAccessFence = fence; }
    const RHI::RHIFence& GetLastAccessFence() const { return LastAccessFence; }
protected:
    RHI::RHIFence LastAccessFence;
};

// 纹理资源追踪
class RenderTextureTracker : public RenderResourceTrackerBase {
public:
    // 更新子资源访问控制
    void UpdateSubresourceAccess(const RHI::RHISubresourceRange& range, RHI::ERHIResourceAccess access) {
        SubresourceAccess[range] = access;
    }
    // 获取子资源访问控制
    RHI::ERHIResourceAccess GetSubresourceAccess(const RHI::RHISubresourceRange& range) const {
        auto it = SubresourceAccess.find(range);
        if (it != SubresourceAccess.end()) return it->second;
        return RHI::ERHIResourceAccess::Unknown;
    }
private:
    std::unordered_map<RHI::RHISubresourceRange, RHI::ERHIResourceAccess> SubresourceAccess;
};

// 缓冲区资源追踪
class RenderBufferTracker : public RenderResourceTrackerBase {
public:
    void UpdateAccess(RHI::ERHIResourceAccess access) { BufferAccess = access; }
    RHI::ERHIResourceAccess GetAccess() const { return BufferAccess; }
private:
    RHI::ERHIResourceAccess BufferAccess = RHI::ERHIResourceAccess::Unknown;
};


} // namespace RenderCore