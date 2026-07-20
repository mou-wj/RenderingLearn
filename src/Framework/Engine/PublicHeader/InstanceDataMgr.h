
#pragma once

#include "RenderResource.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Engine
{
    class InstanceIdBufferDefferedAccessor
    {
    public:
        virtual ~InstanceIdBufferDefferedAccessor() = default;
        virtual RenderCore::RenderBuffer* GetInstanceIdBuffer() const = 0;
    };

    template<typename InstanceDataType, typename InstanceIdType = uint32_t>
    class InstanceDataManager;

    template<typename InstanceDataType, typename InstanceIdType = uint32_t>
    class InstanceDataBlock : public InstanceIdBufferDefferedAccessor
    {
    public:
        using ManagerType = InstanceDataManager<InstanceDataType, InstanceIdType>;

        uint32_t GetBlockId() const { return BlockId; }
        const std::string& GetDebugName() const { return DebugName; }

        uint32_t AddSubParameters(const std::vector<InstanceDataType>& InSubParameters);
        uint32_t AddInstanceIds(const std::vector<InstanceIdType>& InInstanceIds);
        bool UpdateSubParameters(uint32_t LogicalOffset, const std::vector<InstanceDataType>& InSubParameters);

        uint32_t GetSubParameterCount() const { return static_cast<uint32_t>(SubParameters.size()); }
        uint32_t GetInstanceIdCount() const { return static_cast<uint32_t>(InstanceIds.size()); }
        RenderCore::RenderBuffer* GetInstanceIdBuffer() const override;
        RHI::RHIShaderResourceView* GetInstanceDataSRV() const;

    private:
        friend class InstanceDataManager<InstanceDataType, InstanceIdType>;

        InstanceDataBlock(ManagerType* InOwner, uint32_t InBlockId, std::string InDebugName);
        void UpdateGPUResources();
        void ClearGPUResources();

        ManagerType* Owner = nullptr;
        uint32_t BlockId = 0;
        std::string DebugName;
        std::vector<InstanceDataType> SubParameters;
        std::vector<InstanceIdType> InstanceIds;
        std::shared_ptr<RenderCore::RenderBuffer> InstanceDataBuffer;
        std::shared_ptr<RenderCore::RenderBuffer> InstanceIdBuffer;
        bool bSubParametersDirty = false;
        bool bInstanceIdsDirty = false;
    };

    template<typename InstanceDataType, typename InstanceIdType>
    class InstanceDataManager
    {
    public:
        using BlockType = InstanceDataBlock<InstanceDataType, InstanceIdType>;
        using BlockRef = std::shared_ptr<BlockType>;

        static InstanceDataManager& Get();

        BlockRef CreateInstanceBlock(const std::string& DebugName = std::string());
        void RemoveInstanceBlock(uint32_t BlockId);
        void RemoveInstanceBlock(const BlockRef& Block);

        void BeginUpdateGPUResources();
        void UpdateGPUResources();

        RenderCore::RenderBuffer* GetInstanceIdBuffer(const BlockRef& Block) const;

    private:
        friend class InstanceDataBlock<InstanceDataType, InstanceIdType>;

        mutable std::mutex Mutex;
        uint32_t NextBlockId = 1;
        std::unordered_map<uint32_t, BlockRef> Blocks;
    };
}

#include "InstanceDataMgr.ini"