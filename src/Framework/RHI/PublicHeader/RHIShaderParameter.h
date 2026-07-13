#pragma once
#include <string>
#include <cstdint>
#include "RHIResource.h"

namespace RHI
{
    // 基础类型参数描述
    struct RHIShaderUniformParameter
    {
        uint32_t BufferIndex;
        uint32_t BaseIndex;
        uint32_t Offset; // 在统一 buffer 内偏移
        uint32_t Size;   // 字节大小
    };

    // 资源类型参数描述
    struct RHIShaderResourceParameter
    {
        enum class EType : uint8_t
        {
            Texture,
            SRV,
            UAV,
            Sampler,
            UniformBuffer,
        };

        RHIResource* Resource;
        EType Type; // Texture / Buffer / UAV / Sampler
        uint32_t Index = 0;
        uint32_t ArrayIndex = 0;
		template<typename T>
		T* GetResourceAs() const
		{
			return static_cast<T*>(Resource);
		}
    };

   

    // Batch 存储
    struct RHIBatchedShaderParameters
    {
        std::vector<uint8_t> Data;
        std::vector<RHIShaderUniformParameter> UniformParameters;
        std::vector<RHIShaderResourceParameter> ResourceParameters;

        void Reset()
        {
            Data.clear();
            UniformParameters.clear();
            ResourceParameters.clear();
        }

        bool HasParameters() const
        {
            return !UniformParameters.empty() || !ResourceParameters.empty() || !Data.empty();
        }
    };




}