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
    class VulkanCommandContext;
    struct DescriptorBindingState
    {
        VkDescriptorType Type;
        uint64_t ResourceHash = 0; // SRV/UAV/Sampler Ψһ��ʶ
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
        uint64_t GPUBufferOffset = 0;  // GPU缓冲区中的偏移
        bool bDirty = true;  // 是否有待更新的数据
        struct PackedUniformBinding
        {
            uint32_t SetIndex = 0;
            uint32_t Binding = 0;

            uint32_t CPUOffset = 0;
            uint32_t GPUOffset = 0;

            uint32_t Size = 0;
        };
        std::vector<PackedUniformBinding> Bindings;
		int GetBindingInfoIndex(uint32_t BufferIndex) const
		{
			for (size_t i = 0; i < Bindings.size(); i++)
			{
				if (Bindings[i].Binding == BufferIndex)
				{
					return static_cast<int>(i);
				}
			}
			return -1; // 没有找到
		}
		PackedUniformBuffer() = default;
        PackedUniformBuffer(size_t InSize)
            : Data(InSize, 0), GPUBufferOffset(0), bDirty(true)
        {
        }
        PackedUniformBuffer(const std::vector<PackedUniformBinding>& BindingInfo)
            : GPUBufferOffset(0), bDirty(true)
        {
			Bindings = BindingInfo;
			size_t totalSize = 0;
			for (auto& binding : Bindings)
			{
                binding.CPUOffset = totalSize;
                binding.GPUOffset = totalSize;
                int result = binding.Size % 64;
				totalSize += result == 0 ? binding.Size : ((binding.Size / 64 )+ 1)* 64;
			}
			Data.resize(totalSize, 0);
        }
        uint8_t* GetData() { return Data.data(); }
        size_t Num() const { return Data.size(); }
        
        void MarkDirty() { bDirty = true; }
        void MarkClean() { bDirty = false; }
        bool IsDirty() const { return bDirty; }
    };


    class VulkanCommonPipelineDescriptorState
    {
    public:

        VulkanCommonPipelineDescriptorState(VulkanDevice* device, VulkanPipelineBase* pipeline,VulkanCommandContext* context)
            : Device(device)
            , pipeline(pipeline)
            , Context(context)
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
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
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
				VK_DESCRIPTOR_TYPE_SAMPLER,
				VK_NULL_HANDLE,
				VK_IMAGE_LAYOUT_UNDEFINED,
                sampler->GetSampler());
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
                    srv->GetTextureView().View,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            else
            {
                auto hasBufferView = srv->GetBufferView().View != VK_NULL_HANDLE;
                if (!hasBufferView) {
                    set.Writer.WriteBuffer(
						binding,
                        srv->GetDescriptorType(),
                        srv->GetBufferView().Buffer,
                        srv->GetBufferView().Offset,
                        srv->GetBufferView().Size);
				}
                else
                {
                    set.Writer.WriteTexelBuffer(
                        binding,
                        srv->GetDescriptorType(),
                        srv->GetBufferView().View);
                }
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
                    uav->GetTextureView().View,
                    VK_IMAGE_LAYOUT_GENERAL);
            }
            else
            {
                auto hasBufferView = uav->GetBufferView().View  != VK_NULL_HANDLE;
                if (!hasBufferView) {
					set.Writer.WriteBuffer(
						binding,
                        uav->GetDescriptorType(),
                        uav->GetBufferView().Buffer,
                        uav->GetBufferView().Offset,
                        uav->GetBufferView().Size);
                }
                else {
                    set.Writer.WriteTexelBuffer(
                        binding,
                        uav->GetDescriptorType(),
                        uav->GetBufferView().View);
                }
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
            for (const auto& pair : layout.GetInfo().setLayoutsByFrequency)
            {
                ERHIShaderFrequency freq = static_cast<ERHIShaderFrequency>(pair.first);
                const PipelineLayoutInfo::ShaderFrequencyLayoutInfo& freqInfo = pair.second;

                // 如果该shader阶段有全局uniform buffer，则创建对应的PackedUniformBuffer
                if (!freqInfo.UniformBufferLayouts.empty()) {
					// 计算该频率的PackedUniformBuffer大小
					std::vector<PackedUniformBuffer::PackedUniformBinding> bindings;
					for (const auto& ubLayout : freqInfo.UniformBufferLayouts)
					{
						PackedUniformBuffer::PackedUniformBinding binding;
                        binding.SetIndex = ubLayout.SetIndex;
						binding.Binding = ubLayout.BindingIndex;
						binding.Size = ubLayout.Size;
						bindings.push_back(binding);
					}
					PackedUniformBuffersByFrequency[freq] = PackedUniformBuffer(bindings);
                }
                // 构建shader参数的binding映射
                for (size_t i = 0; i < freqInfo.ResourceParameterLayouts.size(); ++i)
                {
                    for (const auto& binding : freqInfo.ResourceParameterLayouts)
                    {
                        ShaderParameterKey key{ freq, binding.BindingIndex };
                        ShaderParameterBinding value{ binding.SetIndex, binding.BindingIndex };
                        ParameterBindingMap[key] = value;
                    }
                }
            }

        }
        
        struct GlobalUniformBufferDesc
        {
            uint32_t SetIndex;
            uint32_t BindingIndex;
        };
        
        //std::unordered_map<ERHIShaderFrequency, GlobalUniformBufferDesc> GlobalUniformBufferInfo;
        std::unordered_map<
            ShaderParameterKey,
            ShaderParameterBinding,
            ShaderParameterKeyHash> ParameterBindingMap;
    public:
        // �ⲿ��ȡ set + binding
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

        void FlushAndBind(VulkanCommandBuffer* cmd);
    protected:
        std::unordered_map<ERHIShaderFrequency, PackedUniformBuffer> PackedUniformBuffersByFrequency;
        public:
        void SetPackedGlobalParameter(
            ERHIShaderFrequency frequency,
            uint32_t BufferIndex,
            uint32_t ByteOffset,
            uint32_t NumBytes,
            const void* NewValue)
        {
            auto it = PackedUniformBuffersByFrequency.find(frequency);
            if (it == PackedUniformBuffersByFrequency.end())
                return;

            PackedUniformBuffer& buffer = it->second;
            //获取bufferIndex的信息
            int index = buffer.GetBindingInfoIndex(BufferIndex);
            if (index == -1) 
            {
                return;
            }
            const PackedUniformBuffer::PackedUniformBinding& binding = buffer.Bindings[index];
            assert(ByteOffset + NumBytes <= binding.Size);
            assert((NumBytes & 3) == 0 && (ByteOffset & 3) == 0);

            uint32_t* dst = reinterpret_cast<uint32_t*>(buffer.GetData() + ByteOffset + binding.CPUOffset);
            const uint32_t* src = reinterpret_cast<const uint32_t*>(NewValue);
            size_t count = NumBytes / 4;

            for (size_t i = 0; i < count; ++i)
            {
                dst[i] = src[i];
            }
            
            // 标记缓冲区为dirty，需要更新到GPU
            buffer.MarkDirty();
        }
    protected:
        VkPipelineBindPoint pipelineBindingPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
		VulkanCommandContext* Context = nullptr;
    };




    class VulkanComputePipelineDescriptorState
        : public VulkanCommonPipelineDescriptorState
    {
    public:

        VulkanComputePipelineDescriptorState(
            VulkanDevice* device,
            VulkanComputePipelineState* pipeline,
            VulkanCommandContext* context)
            : VulkanCommonPipelineDescriptorState(device, pipeline,context)
            , Pipeline(pipeline)
        {
            pipelineBindingPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        }
    private:
        VulkanComputePipelineState* Pipeline = nullptr;
    };

    class VulkanGraphicsPipelineDescriptorState
        : public VulkanCommonPipelineDescriptorState
    {
    public:
        VulkanGraphicsPipelineDescriptorState(
            VulkanDevice* device,
            VulkanGraphicsPipelineState* pipeline,
            VulkanCommandContext* context)
            : VulkanCommonPipelineDescriptorState(device, pipeline,context)
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
        VulkanPendingComputeState(VulkanDevice* device, VulkanCommandContext* context)
            : Device(device)
			, Context(context)
        {
        }

        void Reset()
        {
            CurrentPipeline = nullptr;
            CurrentState = nullptr;
            bDirtyPipelineState = false;
            CurrentPipeline = nullptr;
            CurrentState = nullptr;
            for (auto& state : States) {
                state.second->Reset();
            }
        }

        void SetPipeline(VulkanComputePipelineState* pipeline)
        {
            bDirtyPipelineState = true;
            if (CurrentPipeline == pipeline)
                return;
           
            CurrentPipeline = pipeline;

            auto it = States.find(pipeline);
            if (it == States.end())
            {
                auto state =
                    std::make_unique<
                    VulkanComputePipelineDescriptorState>(
                        Device, pipeline,Context);

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
        // ͨ�� ShaderFrequency + parameterId ���� SRV
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
        // ͨ�� ShaderFrequency + parameterId ���� UAV
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
        // ͨ�� ShaderFrequency + parameterId ���� UniformBuffer
        //--------------------------------------------------------
        void SetUniformBuffer(ERHIShaderFrequency frequency, uint32_t parameterId, VulkanBuffer* uniformBuffer)
        {
            if (!CurrentState)
                return;

            uint32_t setIndex = 0;
            uint32_t binding = 0;
            if (CurrentState->GetBinding(frequency, parameterId, setIndex, binding))
            {
                CurrentState->SetUniformBuffer(setIndex, binding, uniformBuffer->GetHandle(), 0, uniformBuffer->GetSize());
            }
        }

        void SetShaderParameter(ERHIShaderFrequency frequency,uint32_t BufferIndex, uint32_t BaseIndex, uint32_t NumBytes, const void* NewValue) {
			if (!CurrentState)
				return;
            CurrentState->SetPackedGlobalParameter(frequency, BufferIndex, BaseIndex, NumBytes, NewValue);
        }

        bool HasPipeline() const
        {
            return CurrentPipeline != nullptr;
        }

        void PrepareForDispatch(VulkanCommandBuffer* cmd)
        {
            if (bDirtyPipelineState) {
                CurrentPipeline->Bind(cmd);
                CurrentState->FlushAndBind(cmd);
                //暂时每次都重置一下
                for (auto& state : States) {
                    state.second->Reset();
                }
                bDirtyPipelineState = false;
            }

        }

    private:
        VulkanDevice* Device;
        friend class VulkanCommandContext;
        bool bDirtyPipelineState = false;
        VulkanComputePipelineState* CurrentPipeline = nullptr;
        VulkanComputePipelineDescriptorState* CurrentState = nullptr;
		VulkanCommandContext* Context = nullptr;

        std::unordered_map<
            VulkanComputePipelineState*,
            std::unique_ptr<VulkanComputePipelineDescriptorState>> States;
    };

    class VulkanPendingGfxState
    {
    public:
        VulkanPendingGfxState(VulkanDevice* device,VulkanCommandContext* context)
            : Device(device)
			, Context(context)
        {
            Reset();
        }

        void Reset()
        {
            Viewports.clear();
            Scissors.clear();
            
            StencilRef = 0;
            PrimitiveType = PT_Num;
            bDirtyVertexStreams = true;

            CurrentPipeline = nullptr;
            CurrentState = nullptr;
            bDirtyPipelineState = false;
            CurrentPipeline = nullptr;
            CurrentState = nullptr;
            for (auto& state : States) {
                state.second->Reset();
            }
        }

        // Pipeline
        void SetPipeline(VulkanGraphicsPipelineState* pipeline)
        {
            bDirtyPipelineState = true;
            if (CurrentPipeline == pipeline)
                return;
            
            CurrentPipeline = pipeline;
            auto it = States.find(pipeline);
            if (it == States.end())
            {
                auto state = std::make_unique<VulkanGraphicsPipelineDescriptorState>(
                    Device, pipeline,Context);
                CurrentState = state.get();
                States[pipeline] = std::move(state);
            }
            else
            {
                CurrentState = it->second.get();
            }
        }

        // �� VulkanPendingGfxState ��
        //--------------------------------------------------------
        // ʹ�� ShaderFrequency + parameterId ���� Texture
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
        // ʹ�� ShaderFrequency + parameterId ���� sampler
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
        // ʹ�� ShaderFrequency + parameterId ���� SRV
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
        // ʹ�� ShaderFrequency + parameterId ���� UAV
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
        // ʹ�� ShaderFrequency + parameterId ���� UniformBuffer
        //--------------------------------------------------------
        void SetUniformBuffer(ERHIShaderFrequency frequency, uint32_t parameterId, VulkanBuffer* uniformBuffer)
        {
            if (!CurrentState)
                return;

            uint32_t setIndex = 0;
            uint32_t binding = 0;
            if (CurrentState->GetBinding(frequency, parameterId, setIndex, binding))
            {
                CurrentState->SetUniformBuffer(setIndex, binding, uniformBuffer->GetHandle(), 0, uniformBuffer->GetSize());
            }
        }
        void SetShaderParameter(ERHIShaderFrequency frequency,uint32_t BufferIndex, uint32_t BaseIndex, uint32_t NumBytes, const void* NewValue) {
            if (!CurrentState)
                return;
            CurrentState->SetPackedGlobalParameter(frequency,BufferIndex, BaseIndex, NumBytes, NewValue);
        }

        bool HasPipeline() const
        {
            return CurrentPipeline != nullptr;
        }

        // Dynamic states
        void SetViewport(const VkViewport& viewport)
        {
            Viewports.clear();
            auto revertViewport = viewport;
            revertViewport.y =
                revertViewport.y + revertViewport.height;

            revertViewport.height =
                -revertViewport.height;
            Viewports.push_back(revertViewport);
            ViewportDiry = true;
        }

        void SetScissor(const VkRect2D& scissor)
        {
            Scissors.clear();
            Scissors.push_back(scissor);
            ScissorDirty = true;
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

        // Drawǰ��
        void PrepareForDraw(VulkanCommandBuffer* cmd)
        {
            if (bDirtyPipelineState) {
                // 1. Bind pipeline
                CurrentPipeline->Bind(cmd);

                // 2. Bind descriptor sets
                CurrentState->FlushAndBind(cmd);
                bDirtyPipelineState = false;
            }
            // 3. Bind vertex buffers
            if (bDirtyVertexStreams)
            {
                for (uint32_t i = 0; i < MaxVertexElementCount; ++i)
                {
                    auto& stream = PendingStreams[i];
                    if (stream.Stream != VK_NULL_HANDLE)
                    {
                        VkDeviceSize offsets[1] = { stream.BufferOffset };
                        VKFunc::CmdBindVertexBuffers(cmd->GetHandle(), i, 1, &stream.Stream, offsets);
                    }
                }
                bDirtyVertexStreams = false;
            }

            // 4. Set viewport / scissor
            if (!Viewports.empty() && ViewportDiry)
            {
                VKFunc::CmdSetViewport(cmd->GetHandle(), 0, (uint32_t)Viewports.size(), Viewports.data());
				ViewportDiry = false;
            }
                

            if (!Scissors.empty() && ScissorDirty)
            {
                VKFunc::CmdSetScissor(cmd->GetHandle(), 0, (uint32_t)Scissors.size(), Scissors.data());
				ScissorDirty = false;
            }
                

            // 5. Set stencil ref
            //vkCmdSetStencilReference(cmd->GetHandle(), VK_STENCIL_FRONT_AND_BACK, StencilRef);
        }

    private:
        friend class VulkanCommandContext;
        VulkanDevice* Device;
        bool bDirtyPipelineState = false;
        VulkanGraphicsPipelineState* CurrentPipeline = nullptr;
        VulkanGraphicsPipelineDescriptorState* CurrentState = nullptr;

        std::unordered_map<VulkanGraphicsPipelineState*, std::unique_ptr<VulkanGraphicsPipelineDescriptorState>> States;

        // Dynamic states
        std::vector<VkViewport> Viewports;
        std::vector<VkRect2D> Scissors;
        bool ViewportDiry = false;
        bool ScissorDirty = false;
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
		VulkanCommandContext* Context = nullptr;
    };



}