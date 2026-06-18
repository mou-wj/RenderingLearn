#include "VulkanPendingPipelineState.h"
#include "VulkanCommandContex.h"
#include <cstring>
namespace RHIVulkan {

    void VulkanCommonPipelineDescriptorState::FlushAndBind(VulkanCommandBuffer* cmd)
    {
        const auto& sets = pipeline->GetLayout()->GetInfo().GetAllLayouts();

        // 首先处理PackedUniformBuffer的更新
        bool bWrapAroundDetected = false;
        uint64_t lastAllocationOffset = 0;
        
        VulkanLooseUniformDataUploader* uploader = nullptr;
        if (auto* computeContext = dynamic_cast<VulkanComputeContext*>(Context))
        {
            uploader = computeContext->GetLooseUniformDataUploader();
        }
        else if (auto* graphicContext = dynamic_cast<VulkanGraphicContext*>(Context))
        {
            uploader = graphicContext->GetLooseUniformDataUploader();
        }

        if (!uploader)
        {
            return;
        }
        
        // 遍历每个shader frequency的PackedUniformBuffer
        for (auto& pair : PackedUniformBuffersByFrequency)
        {
            ERHIShaderFrequency freq = pair.first;
            PackedUniformBuffer& buffer = pair.second;
            
            // 检查是否需要分配内存
            if (buffer.IsDirty() || bWrapAroundDetected)
            {
                // 为该频率的PackedUniformBuffer分配GPU内存
                uint64_t newOffset = uploader->AllocateMemory(
                    buffer.Num(), 
                    256  // 256字节对齐
                    );
                
                // 检测是否发生了环绕
                if (newOffset < lastAllocationOffset && lastAllocationOffset > 0)
                {
                    bWrapAroundDetected = true;
                    
                    // 如果发生了环绕，标记所有其他buffer为dirty，需要重新上载
                    for (auto& otherPair : PackedUniformBuffersByFrequency)
                    {
                        otherPair.second.MarkDirty();
                    }
                }
                
                lastAllocationOffset = newOffset;
                buffer.GPUBufferOffset = newOffset;
                //更新GPUoffset
                for (auto& bufferBinding : buffer.Bindings) {
                    bufferBinding.GPUOffset = bufferBinding.CPUOffset + newOffset;
                }
                
                // 复制数据到GPU缓冲区
                uint8_t* gpuPtr = uploader->GetCPUMappedPointer() + newOffset;
                std::memcpy(gpuPtr, buffer.GetData(), buffer.Num());
                
                // 标记为clean
                buffer.MarkClean();
            }
            // 从GlobalUniformBufferInfo中查找该frequency对应的set和binding
            for (auto& BindingInfos : buffer.Bindings) {
                if (BindingInfos.bUseExternalBuffer)
                {
                    BindingInfos.bUseExternalBuffer=false;
                    continue;
                }

                uint32_t setIndex = BindingInfos.SetIndex;
                uint32_t binding = BindingInfos.Binding;

                SetUniformBuffer(
                    setIndex,
                    binding,
                    uploader->GetCPUBufferHandle(),
                    BindingInfos.GPUOffset,
                    BindingInfos.Size);
            }
        }

  
        // 然后处理Descriptor的分配和更新
        bool isUpdated = UpdateDescriptorSetsIfDirty();
        if (!isUpdated) {
            //如果没有更新就不需要重新绑定，所以直接return
            return ;
        }

        std::vector<VkDescriptorSet> vkSets;
        vkSets.reserve(sets.size());

        for (uint32_t i = 0; i < sets.size(); i++)
            vkSets.push_back(GetDescriptorSet(i));

        Device->GetDescriptorSetManager()->BindDescriptorSets(cmd, pipeline->GetLayout()->GetHandle(), pipelineBindingPoint, vkSets);

    }

}

