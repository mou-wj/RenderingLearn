#pragma once
#include "VulkanDevice.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "VulkanDescriptorSets.h"
#include "VulkanRenderPass.h"
#include "VulkanResource.h"
#include "VulkanPipelineState.h"
#include "VulkanCommandBuffer.h"
namespace RHIVulkan {

    struct DescriptorBindingState
    {
        VkDescriptorType Type;
        uint64_t ResourceHash = 0; // SRV/UAV/Sampler 唯一标识
        bool Dirty = true;
    };

    struct VulkanPerSetDescriptorState
    {
        VulkanDescriptorWriter Writer;

        VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;

        bool bDirty = true;

        void Reset()
        {
            Writer.Reset();
            DescriptorSet = VK_NULL_HANDLE;
            bDirty = true;
        }
    };

    struct ShaderParameterKey
    {
        ERHIShaderFrequency Frequency;
        uint32_t ParameterId;

        bool operator==(const ShaderParameterKey& other) const
        {
            return Frequency == other.Frequency &&
                ParameterId == other.ParameterId;
        }
    };

    struct ShaderParameterKeyHash
    {
        size_t operator()(const ShaderParameterKey& k) const
        {
            return (size_t)k.Frequency ^ ((size_t)k.ParameterId << 8);
        }
    };

    struct ShaderParameterBinding
    {
        uint32_t SetIndex = 0;
        uint32_t Binding = 0;
    };

    struct PackedUniformBuffer
    {
        std::vector<uint8_t> Data;

        PackedUniformBuffer(size_t InSize)
            : Data(InSize, 0)
        {
        }

        uint8_t* GetData() { return Data.data(); }
        size_t Num() const { return Data.size(); }
    };


    class VulkanCommonPipelineDescriptorState
    {
    public:

        VulkanCommonPipelineDescriptorState(VulkanDevice* device, VulkanPipelineBase* pipeline)
            : Device(device)
            , pipeline(pipeline)
        {
            Sets.resize(MaxDescriptorSets);
			ParsePipelineLayout(*(pipeline->GetLayout()));
        }

        virtual ~VulkanCommonPipelineDescriptorState() = default;

    public:

        void Reset()
        {
            for (auto& set : Sets)
            {
                set.Reset();
            }
        }

    public:

        //---------------------------------------------------------
        // Texture
        //---------------------------------------------------------

        void SetTexture(
            uint32_t setIndex,
            uint32_t binding,
            VulkanTexture* texture)
        {
            auto& set = Sets[setIndex];

            set.Writer.WriteImage(
                binding,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                texture->GetImageView(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            set.bDirty = true;
        }
        
		void SetSampler(
			uint32_t setIndex,
			uint32_t binding,
			VulkanSampler* sampler)
		{
			auto& set = Sets[setIndex];
			set.Writer.WriteImage(
				binding,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_NULL_HANDLE,
				VK_IMAGE_LAYOUT_UNDEFINED);
			set.bDirty = true;
		}

    public:

        //---------------------------------------------------------
        // SRV
        //---------------------------------------------------------

        void SetSRV(
            uint32_t setIndex,
            uint32_t binding,
            VulkanShaderResourceView* srv)
        {
            auto& set = Sets[setIndex];

            if (srv->IsTexture())
            {
                set.Writer.WriteImage(
                    binding,
                    srv->GetDescriptorType(),
                    srv->GetImageView(),
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            else
            {
                set.Writer.WriteUniformTexelBuffer(
                    binding,
                    srv->GetBufferView());
            }

            set.bDirty = true;
        }

    public:

        //---------------------------------------------------------
        // UAV
        //---------------------------------------------------------

        void SetUAV(
            uint32_t setIndex,
            uint32_t binding,
            VulkanUnorderedAccessView* uav)
        {
            auto& set = Sets[setIndex];

            if (uav->IsTexture())
            {
                set.Writer.WriteImage(
                    binding,
                    uav->GetDescriptorType(),
                    uav->GetImageView(),
                    VK_IMAGE_LAYOUT_GENERAL);
            }
            else
            {
                set.Writer.WriteStorageTexelBuffer(
                    binding,
                    uav->GetBufferView());
            }

            set.bDirty = true;
        }

    public:

        //---------------------------------------------------------
        // UniformBuffer
        //---------------------------------------------------------

        void SetUniformBuffer(
            uint32_t setIndex,
            uint32_t binding,
            VkBuffer buffer,
            VkDeviceSize offset,
            VkDeviceSize size)
        {
            auto& set = Sets[setIndex];

            set.Writer.WriteBuffer(
                binding,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                buffer,
                offset,
                size);

            set.bDirty = true;
        }

    public:

        //---------------------------------------------------------
        // Allocate
        //---------------------------------------------------------

        void AllocateIfNeeded(
            uint32_t setIndex,
            const DescriptorSetLayoutInfo& layoutInfo)
        {
            auto& set = Sets[setIndex];

            if (set.DescriptorSet != VK_NULL_HANDLE)
                return;

            set.DescriptorSet = Device->GetDescriptorSetManager()
                ->GetDescriptorSet(layoutInfo);
        }

    public:

        //---------------------------------------------------------
        // Update
        //---------------------------------------------------------

        void UpdateIfDirty()
        {
            for (auto& set : Sets)
            {
                if (set.DescriptorSet == VK_NULL_HANDLE)
                    continue;

                if (set.bDirty)
                {
                    set.Writer.Update(
                        Device->GetHandle(),
                        set.DescriptorSet);

                    set.bDirty = false;
                }
            }
        }

    public:

        VkDescriptorSet GetDescriptorSet(uint32_t setIndex) const
        {
            return Sets[setIndex].DescriptorSet;
        }

    private:

        static constexpr uint32_t MaxDescriptorSets = 8;

    protected:

        struct VulkanPerSetDescriptorState
        {
            VulkanDescriptorWriter Writer;

            VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;

            bool bDirty = true;

            void Reset()
            {
                Writer.Reset();
                DescriptorSet = VK_NULL_HANDLE;
                bDirty = true;
            }
        };

        VulkanDevice* Device = nullptr;

        std::vector<VulkanPerSetDescriptorState> Sets;
        VulkanPipelineBase* pipeline;

    private:
        void ParsePipelineLayout(const VulkanPipelineLayout& layout)
        {
            const auto& sets = layout.GetInfo().setLayouts;

            for (uint32_t setIndex = 0; setIndex < sets.size(); ++setIndex)
            {
                const DescriptorSetLayoutInfo& setLayout = sets[setIndex];
                for (const auto& binding : setLayout.bindings)
                {
                    ShaderParameterKey key;
                    key.Frequency = ERHIShaderFrequency::Compute;
                    key.ParameterId = binding.Binding; // parameterId这里简化为binding id

                    ShaderParameterBinding value;
                    value.SetIndex = setIndex;
                    value.Binding = binding.Binding;

                    ParameterBindingMap[key] = value;
                }
            }
        }
        std::unordered_map<
            ShaderParameterKey,
            ShaderParameterBinding,
            ShaderParameterKeyHash> ParameterBindingMap;
    public:
        // 外部获取 set + binding
        bool GetBinding(
            ERHIShaderFrequency frequency,
            uint32_t parameterId,
            uint32_t& outSetIndex,
            uint32_t& outBinding) const
        {
            ShaderParameterKey key{ frequency, parameterId };
            auto it = ParameterBindingMap.find(key);
            if (it == ParameterBindingMap.end())
                return false;

            outSetIndex = it->second.SetIndex;
            outBinding = it->second.Binding;
            return true;
        }

        void FlushAndBind(VulkanCommandBuffer* cmd)
        {
            const auto& sets = pipeline->GetLayout()->GetInfo().setLayouts;

            for (uint32_t i = 0; i < sets.size(); i++)
            {
                AllocateIfNeeded(i, sets[i]);
            }

            UpdateIfDirty();

            std::vector<VkDescriptorSet> vkSets;
            vkSets.reserve(sets.size());

            for (uint32_t i = 0; i < sets.size(); i++)
                vkSets.push_back(GetDescriptorSet(i));

            Device->GetDescriptorSetManager()->BindDescriptorSets(cmd, pipeline->GetLayout()->GetHandle(),pipelineBindingPoint,vkSets);
        }
    protected:
        std::vector<PackedUniformBuffer> PackedUniformBuffers;
        uint64_t PackedUniformBufferDirtyMask = 0; // 每一位表示一个 buffer 是否被修改

        // 初始化，比如 4 个 uniform buffer，每个 4KB
        void InitPackedUniformBuffers()
        {
            PackedUniformBuffers.clear();
            for (int i = 0; i < 4; ++i)
            {
                PackedUniformBuffers.emplace_back(4096);
            }
        }
        public:
        void SetPackedGlobalParameter(
            uint32_t BufferIndex,
            uint32_t ByteOffset,
            uint32_t NumBytes,
            const void* NewValue)
        {
            // 检查范围
            auto& StagingBuffer = PackedUniformBuffers[BufferIndex];
            assert(ByteOffset + NumBytes <= StagingBuffer.Num());
            assert((NumBytes & 3) == 0 && (ByteOffset & 3) == 0);

            uint32_t* dst = reinterpret_cast<uint32_t*>(StagingBuffer.GetData() + ByteOffset);
            const uint32_t* src = reinterpret_cast<const uint32_t*>(NewValue);
            size_t count = NumBytes / 4;

            bool bChanged = false;
            for (size_t i = 0; i < count; ++i)
            {
                if (dst[i] != src[i])
                {
                    dst[i] = src[i];
                    bChanged = true;
                }
            }

            if (bChanged)
                PackedUniformBufferDirtyMask |= (1ull << BufferIndex);
        }
    protected:
        VkPipelineBindPoint pipelineBindingPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    };




    class VulkanComputePipelineDescriptorState
        : public VulkanCommonPipelineDescriptorState
    {
    public:

        VulkanComputePipelineDescriptorState(
            VulkanDevice* device,
            VulkanComputePipeline* pipeline)
            : VulkanCommonPipelineDescriptorState(device, pipeline)
            , Pipeline(pipeline)
        {
            pipelineBindingPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        }
    private:
        VulkanComputePipeline* Pipeline = nullptr;
    };

    class VulkanGraphicsPipelineDescriptorState
        : public VulkanCommonPipelineDescriptorState
    {
    public:
        VulkanGraphicsPipelineDescriptorState(
            VulkanDevice* device,
            VulkanGraphicsPipelineState* pipeline)
            : VulkanCommonPipelineDescriptorState(device, pipeline)
            , Pipeline(pipeline)
        {
            pipelineBindingPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        }
    private:
        VulkanGraphicsPipelineState* Pipeline;
    };

    class VulkanPendingComputeState
    {
    public:
        VulkanPendingComputeState(VulkanDevice* device)
            : Device(device)
        {
        }

        void SetPipeline(VulkanComputePipeline* pipeline)
        {
            if (CurrentPipeline == pipeline)
                return;

            CurrentPipeline = pipeline;

            auto it = States.find(pipeline);
            if (it == States.end())
            {
                auto state =
                    std::make_unique<
                    VulkanComputePipelineDescriptorState>(
                        Device, pipeline);

                CurrentState = state.get();
                States[pipeline] = std::move(state);
            }
            else
            {
                CurrentState = it->second.get();
            }
        }

        void SetTexture(ERHIShaderFrequency frequency, uint32_t parameterId, VulkanTexture* texture)
        {
            if (!CurrentState)
                return;

            uint32_t setIndex = 0;
            uint32_t binding = 0;
            if (CurrentState->GetBinding(frequency, parameterId, setIndex, binding))
            {
                CurrentState->SetTexture(setIndex, binding, texture);
            }
        }

		void SetSampler(ERHIShaderFrequency frequency, uint32_t parameterId, VulkanSampler* sampler)
		{
			if (!CurrentState)
				return;
			uint32_t setIndex = 0;
			uint32_t binding = 0;
			if (CurrentState->GetBinding(frequency, parameterId, setIndex, binding))
			{
				CurrentState->SetSampler(setIndex, binding, sampler);
			}
		}


        //--------------------------------------------------------
        // 通过 ShaderFrequency + parameterId 设置 SRV
        //--------------------------------------------------------
        void SetSRV(ERHIShaderFrequency frequency, uint32_t parameterId, VulkanShaderResourceView* srv)
        {
            if (!CurrentState)
                return;

            uint32_t setIndex = 0;
            uint32_t binding = 0;
            if (CurrentState->GetBinding(frequency, parameterId, setIndex, binding))
            {
                CurrentState->SetSRV(setIndex, binding, srv);
            }
        }

        //--------------------------------------------------------
        // 通过 ShaderFrequency + parameterId 设置 UAV
        //--------------------------------------------------------
        void SetUAV(ERHIShaderFrequency frequency, uint32_t parameterId, VulkanUnorderedAccessView* uav)
        {
            if (!CurrentState)
                return;

            uint32_t setIndex = 0;
            uint32_t binding = 0;
            if (CurrentState->GetBinding(frequency, parameterId, setIndex, binding))
            {
                CurrentState->SetUAV(setIndex, binding, uav);
            }
        }

        //--------------------------------------------------------
        // 通过 ShaderFrequency + parameterId 设置 UniformBuffer
        //--------------------------------------------------------
        void SetUniformBuffer(ERHIShaderFrequency frequency, uint32_t parameterId, VulkanUniformBuffer* uniformBuffer)
        {
            if (!CurrentState)
                return;

            uint32_t setIndex = 0;
            uint32_t binding = 0;
            if (CurrentState->GetBinding(frequency, parameterId, setIndex, binding))
            {
                ///CurrentState->SetUniformBuffer(setIndex, binding, buffer, offset, size);
            }
        }

        void SetShaderParameter(uint32_t BufferIndex, uint32_t BaseIndex, uint32_t NumBytes, const void* NewValue) {
			if (!CurrentState)
				return;
            CurrentState->SetPackedGlobalParameter(BufferIndex, BaseIndex, NumBytes, NewValue);
        }

        void PrepareForDispatch(VulkanCommandBuffer* cmd)
        {
            CurrentPipeline->Bind(cmd);
            CurrentState->FlushAndBind(cmd);
        }

    private:
        VulkanDevice* Device;
        friend class VulkanCommandContext;
        VulkanComputePipeline* CurrentPipeline = nullptr;
        VulkanComputePipelineDescriptorState* CurrentState = nullptr;

        std::unordered_map<
            VulkanComputePipeline*,
            std::unique_ptr<VulkanComputePipelineDescriptorState>> States;
    };

    class VulkanPendingGfxState
    {
    public:
        VulkanPendingGfxState(VulkanDevice* device)
            : Device(device)
        {
            Reset();
        }

        void Reset()
        {
            Viewports.clear();
            Scissors.clear();
            bScissorEnable = false;
            StencilRef = 0;
            PrimitiveType = PT_Num;
            bDirtyVertexStreams = true;

            CurrentPipeline = nullptr;
            CurrentState = nullptr;
        }

        // Pipeline
        void SetPipeline(VulkanGraphicsPipelineState* pipeline)
        {
            if (CurrentPipeline == pipeline)
                return;

            CurrentPipeline = pipeline;
            auto it = States.find(pipeline);
            if (it == States.end())
            {
                auto state = std::make_unique<VulkanGraphicsPipelineDescriptorState>(
                    Device, pipeline);
                CurrentState = state.get();
                States[pipeline] = std::move(state);
            }
            else
            {
                CurrentState = it->second.get();
            }
        }

        // 在 VulkanPendingGfxState 中
        //--------------------------------------------------------
        // 使用 ShaderFrequency + parameterId 设置 Texture
        //--------------------------------------------------------
        void SetTexture(ERHIShaderFrequency frequency, uint32_t parameterId, VulkanTexture* texture)
        {
            if (!CurrentState)
                return;

            uint32_t setIndex = 0;
            uint32_t binding = 0;
            if (CurrentState->GetBinding(frequency, parameterId, setIndex, binding))
            {
                CurrentState->SetTexture(setIndex, binding, texture);
            }
        }

        //--------------------------------------------------------
        // 使用 ShaderFrequency + parameterId 设置 sampler
        //--------------------------------------------------------
		void SetSampler(ERHIShaderFrequency frequency, uint32_t parameterId, VulkanSampler* sampler)
		{
			if (!CurrentState)
				return;
			uint32_t setIndex = 0;
			uint32_t binding = 0;
			if (CurrentState->GetBinding(frequency, parameterId, setIndex, binding))
			{
				CurrentState->SetSampler(setIndex, binding, sampler);
			}
		}

        //--------------------------------------------------------
        // 使用 ShaderFrequency + parameterId 设置 SRV
        //--------------------------------------------------------
        void SetSRV(ERHIShaderFrequency frequency, uint32_t parameterId, VulkanShaderResourceView* srv)
        {
            if (!CurrentState)
                return;

            uint32_t setIndex = 0;
            uint32_t binding = 0;
            if (CurrentState->GetBinding(frequency, parameterId, setIndex, binding))
            {
                CurrentState->SetSRV(setIndex, binding, srv);
            }
        }

        //--------------------------------------------------------
        // 使用 ShaderFrequency + parameterId 设置 UAV
        //--------------------------------------------------------
        void SetUAV(ERHIShaderFrequency frequency, uint32_t parameterId, VulkanUnorderedAccessView* uav)
        {
            if (!CurrentState)
                return;

            uint32_t setIndex = 0;
            uint32_t binding = 0;
            if (CurrentState->GetBinding(frequency, parameterId, setIndex, binding))
            {
                CurrentState->SetUAV(setIndex, binding, uav);
            }
        }

        //--------------------------------------------------------
        // 使用 ShaderFrequency + parameterId 设置 UniformBuffer
        //--------------------------------------------------------
        void SetUniformBuffer(ERHIShaderFrequency frequency, uint32_t parameterId, VulkanUniformBuffer* uniformBuffer)
        {
            if (!CurrentState)
                return;

            uint32_t setIndex = 0;
            uint32_t binding = 0;
            if (CurrentState->GetBinding(frequency, parameterId, setIndex, binding))
            {
                //CurrentState->SetUniformBuffer(setIndex, binding, buffer, offset, size);
            }
        }
        void SetShaderParameter(uint32_t BufferIndex, uint32_t BaseIndex, uint32_t NumBytes, const void* NewValue) {
            if (!CurrentState)
                return;
            CurrentState->SetPackedGlobalParameter(BufferIndex, BaseIndex, NumBytes, NewValue);
        }

        // Dynamic states
        void SetViewport(const VkViewport& viewport)
        {
            Viewports.clear();
            Viewports.push_back(viewport);
            bScissorEnable = false;
        }

        void SetScissor(const VkRect2D& scissor)
        {
            Scissors.clear();
            Scissors.push_back(scissor);
            bScissorEnable = true;
        }

        void SetStencilRef(uint32_t ref)
        {
            StencilRef = ref;
        }

        void SetVertexStream(uint32_t index, VkBuffer buffer, uint32_t offset)
        {
            PendingStreams[index].Stream = buffer;
            PendingStreams[index].BufferOffset = offset;
            bDirtyVertexStreams = true;
        }

        // Draw前绑定
        void PrepareForDraw(VulkanCommandBuffer* cmd)
        {
            // 1. Bind pipeline
            CurrentPipeline->Bind(cmd);

            // 2. Bind descriptor sets
            CurrentState->FlushAndBind(cmd);

            // 3. Bind vertex buffers
            if (bDirtyVertexStreams)
            {
                for (uint32_t i = 0; i < MaxVertexElementCount; ++i)
                {
                    auto& stream = PendingStreams[i];
                    if (stream.Stream != VK_NULL_HANDLE)
                    {
                        VkDeviceSize offsets[1] = { stream.BufferOffset };
                        vkCmdBindVertexBuffers(cmd->GetHandle(), i, 1, &stream.Stream, offsets);
                    }
                }
                bDirtyVertexStreams = false;
            }

            // 4. Set viewport / scissor
            if (!Viewports.empty())
                vkCmdSetViewport(cmd->GetHandle(), 0, (uint32_t)Viewports.size(), Viewports.data());

            if (!Scissors.empty())
                vkCmdSetScissor(cmd->GetHandle(), 0, (uint32_t)Scissors.size(), Scissors.data());

            // 5. Set stencil ref
            //vkCmdSetStencilReference(cmd->GetHandle(), VK_STENCIL_FRONT_AND_BACK, StencilRef);
        }

    private:
        friend class VulkanCommandContext;
        VulkanDevice* Device;

        VulkanGraphicsPipelineState* CurrentPipeline = nullptr;
        VulkanGraphicsPipelineDescriptorState* CurrentState = nullptr;

        std::unordered_map<VulkanGraphicsPipelineState*, std::unique_ptr<VulkanGraphicsPipelineDescriptorState>> States;

        // Dynamic states
        std::vector<VkViewport> Viewports;
        std::vector<VkRect2D> Scissors;
        bool bScissorEnable = false;
        uint32_t StencilRef = 0;

        enum EPrimitiveType { PT_Num } PrimitiveType;

        struct FVertexStream
        {
            VkBuffer Stream = VK_NULL_HANDLE;
            uint32_t BufferOffset = 0;
        };
        constexpr static uint32_t MaxVertexElementCount = 8;
        FVertexStream PendingStreams[MaxVertexElementCount];
        bool bDirtyVertexStreams = true;
    };



}