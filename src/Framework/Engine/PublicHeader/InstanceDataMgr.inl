namespace Engine
{
    namespace InstanceDataMgrDetail
    {
        inline RHI::RHIBufferDesc BuildStructuredBufferDesc(uint64_t SizeInBytes, uint32_t Stride, const char* DebugName)
        {
            RHI::RHIBufferDesc Desc;
            Desc.Size = std::max<uint64_t>(static_cast<uint64_t>(Stride), SizeInBytes);
            Desc.Stride = Stride;
            Desc.InitialQueueType = RHI::EQueueType::Graphics;
            Desc.Usage =
                RHI::ERHIBufferUsageFlag::Structured |
                RHI::ERHIBufferUsageFlag::ShaderResource |
                RHI::ERHIBufferUsageFlag::Vertex |
                RHI::ERHIBufferUsageFlag::TransferDst;
            Desc.DebugName = DebugName;
            return Desc;
        }
    }

    template<typename InstanceDataType, typename InstanceIdType>
    InstanceDataBlock<InstanceDataType, InstanceIdType>::InstanceDataBlock(
        ManagerType* InOwner,
        uint32_t InBlockId,
        std::string InDebugName)
        : Owner(InOwner)
        , BlockId(InBlockId)
        , DebugName(std::move(InDebugName))
    {
    }

    template<typename InstanceDataType, typename InstanceIdType>
    uint32_t InstanceDataBlock<InstanceDataType, InstanceIdType>::AddSubParameters(
        const std::vector<InstanceDataType>& InSubParameters)
    {
        if (!Owner || InSubParameters.empty())
        {
            return 0;
        }

        std::lock_guard<std::mutex> Lock(Owner->Mutex);
        const uint32_t LogicalOffset = static_cast<uint32_t>(SubParameters.size());
        SubParameters.insert(SubParameters.end(), InSubParameters.begin(), InSubParameters.end());
        bSubParametersDirty = true;
        return LogicalOffset;
    }

    template<typename InstanceDataType, typename InstanceIdType>
    uint32_t InstanceDataBlock<InstanceDataType, InstanceIdType>::AddInstanceIds(
        const std::vector<InstanceIdType>& InInstanceIds)
    {
        if (!Owner || InInstanceIds.empty())
        {
            return 0;
        }

        std::lock_guard<std::mutex> Lock(Owner->Mutex);
        const uint32_t LogicalOffset = static_cast<uint32_t>(InstanceIds.size());
        InstanceIds.insert(InstanceIds.end(), InInstanceIds.begin(), InInstanceIds.end());
        bInstanceIdsDirty = true;
        return LogicalOffset;
    }

    template<typename InstanceDataType, typename InstanceIdType>
    bool InstanceDataBlock<InstanceDataType, InstanceIdType>::UpdateSubParameters(
        uint32_t LogicalOffset,
        const std::vector<InstanceDataType>& InSubParameters)
    {
        if (!Owner || InSubParameters.empty())
        {
            return false;
        }

        std::lock_guard<std::mutex> Lock(Owner->Mutex);
        const uint32_t Count = static_cast<uint32_t>(InSubParameters.size());
        const uint32_t RequiredSize = LogicalOffset + Count;
        if (RequiredSize > static_cast<uint32_t>(SubParameters.size()))
        {
            SubParameters.resize(RequiredSize);
        }

        std::copy(InSubParameters.begin(), InSubParameters.end(), SubParameters.begin() + LogicalOffset);
        bSubParametersDirty = true;
        return true;
    }

    template<typename InstanceDataType, typename InstanceIdType>
    void InstanceDataBlock<InstanceDataType, InstanceIdType>::UpdateGPUResources()
    {
        if (bSubParametersDirty)
        {
            if (SubParameters.empty())
            {
                InstanceDataBuffer.reset();
            }
            else
            {
                const uint64_t SizeInBytes = static_cast<uint64_t>(SubParameters.size()) * sizeof(InstanceDataType);
                if (!InstanceDataBuffer || InstanceDataBuffer->GetRHI()->GetDesc().Size < SizeInBytes)
                {
                    InstanceDataBuffer = std::make_shared<RenderCore::RenderBuffer>(
                        InstanceDataMgrDetail::BuildStructuredBufferDesc(
                            SizeInBytes,
                            sizeof(InstanceDataType),
                            "InstanceDataBuffer"));
                    InstanceDataBuffer->InitRHIResource();
                }

                InstanceDataBuffer->UploadData(SubParameters.data(), static_cast<uint32_t>(SizeInBytes));
                RenderCore::TransitionBufferImmediate(
                    RHI::GRHIApi,
                    InstanceDataBuffer.get(),
                    RHI::ERHIResourceAccess::ShadingRateSource,
                    RHI::EQueueType::Graphics);
            }

            bSubParametersDirty = false;
        }

        if (bInstanceIdsDirty)
        {
            if (InstanceIds.empty())
            {
                InstanceIdBuffer.reset();
            }
            else
            {
                const uint64_t SizeInBytes = static_cast<uint64_t>(InstanceIds.size()) * sizeof(InstanceIdType);
                if (!InstanceIdBuffer || InstanceIdBuffer->GetRHI()->GetDesc().Size < SizeInBytes)
                {
                    InstanceIdBuffer = std::make_shared<RenderCore::RenderBuffer>(
                        InstanceDataMgrDetail::BuildStructuredBufferDesc(
                            SizeInBytes,
                            sizeof(InstanceIdType),
                            "InstanceIdBuffer"));
                    InstanceIdBuffer->InitRHIResource();
                }

                InstanceIdBuffer->UploadData(InstanceIds.data(), static_cast<uint32_t>(SizeInBytes));
                RenderCore::TransitionBufferImmediate(
                    RHI::GRHIApi,
                    InstanceIdBuffer.get(),
                    RHI::ERHIResourceAccess::VertexOrIndexBuffer,
                    RHI::EQueueType::Graphics);
            }

            bInstanceIdsDirty = false;
        }
    }

    template<typename InstanceDataType, typename InstanceIdType>
    void InstanceDataBlock<InstanceDataType, InstanceIdType>::ClearGPUResources()
    {
        SubParameters.clear();
        InstanceIds.clear();
        InstanceDataBuffer.reset();
        InstanceIdBuffer.reset();
        bSubParametersDirty = false;
        bInstanceIdsDirty = false;
    }

    template<typename InstanceDataType, typename InstanceIdType>
    RenderCore::RenderBuffer* InstanceDataBlock<InstanceDataType, InstanceIdType>::GetInstanceIdBuffer() const
    {
        return InstanceIdBuffer.get();
    }

    template<typename InstanceDataType, typename InstanceIdType>
    RHI::RHIShaderResourceView* InstanceDataBlock<InstanceDataType, InstanceIdType>::GetInstanceDataSRV() const
    {
        RHI::RHIBufferSRVCreateInfo CreateInfo;
        CreateInfo.Format = RHI::ERHIFormat::Unknown;
        CreateInfo.NumElements = static_cast<uint32_t>(SubParameters.size());
        CreateInfo.Stride = sizeof(InstanceDataType);
        return InstanceDataBuffer
            ? InstanceDataBuffer->GetViewCache().GetOrCreateSRV(InstanceDataBuffer->GetRHI(), CreateInfo)
            : nullptr;
    }

    template<typename InstanceDataType, typename InstanceIdType>
    InstanceDataManager<InstanceDataType, InstanceIdType>&
        InstanceDataManager<InstanceDataType, InstanceIdType>::Get()
    {
        static InstanceDataManager Instance;
        return Instance;
    }

    template<typename InstanceDataType, typename InstanceIdType>
    typename InstanceDataManager<InstanceDataType, InstanceIdType>::BlockRef
        InstanceDataManager<InstanceDataType, InstanceIdType>::CreateInstanceBlock(const std::string& DebugName)
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        const uint32_t BlockId = NextBlockId++;
        BlockRef Block(new BlockType(this, BlockId, DebugName));
        Blocks.emplace(BlockId, Block);
        return Block;
    }

    template<typename InstanceDataType, typename InstanceIdType>
    void InstanceDataManager<InstanceDataType, InstanceIdType>::RemoveInstanceBlock(uint32_t BlockId)
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        auto It = Blocks.find(BlockId);
        if (It == Blocks.end())
        {
            return;
        }

        It->second->ClearGPUResources();
        Blocks.erase(It);
    }

    template<typename InstanceDataType, typename InstanceIdType>
    void InstanceDataManager<InstanceDataType, InstanceIdType>::RemoveInstanceBlock(const BlockRef& Block)
    {
        if (!Block)
        {
            return;
        }

        RemoveInstanceBlock(Block->GetBlockId());
    }

    template<typename InstanceDataType, typename InstanceIdType>
    void InstanceDataManager<InstanceDataType, InstanceIdType>::BeginUpdateGPUResources()
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        for (auto& Pair : Blocks)
        {
            if (!Pair.second)
            {
                continue;
            }

            Pair.second->InstanceIds.clear();
            Pair.second->bInstanceIdsDirty = true;
        }
    }

    template<typename InstanceDataType, typename InstanceIdType>
    void InstanceDataManager<InstanceDataType, InstanceIdType>::UpdateGPUResources()
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        for (auto& Pair : Blocks)
        {
            if (Pair.second)
            {
                Pair.second->UpdateGPUResources();
            }
        }
    }

    template<typename InstanceDataType, typename InstanceIdType>
    RenderCore::RenderBuffer* InstanceDataManager<InstanceDataType, InstanceIdType>::GetInstanceIdBuffer(const BlockRef& Block) const
    {
        std::lock_guard<std::mutex> Lock(Mutex);
        return Block ? Block->GetInstanceIdBuffer() : nullptr;
    }
}