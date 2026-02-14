#pragma once
#include <string>
#include <cstdint>
#include "RHIResource.h"

namespace RHI
{
    // 基础类型参数描述
    struct RHIShaderUniformParameter
    {
        EShaderUniformBaseType Type;
        uint32_t Offset; // 在统一 buffer 内偏移
        uint32_t Size;   // 字节大小
    };

    // 资源类型参数描述
    struct RHIShaderResourceParameter
    {
        RHIResource* Resource;
        EShaderUniformBaseType Type; // Texture / Buffer / UAV / Sampler
        uint16_t BindSlot;           // GPU绑定槽
        uint16_t ArraySize = 1;      // 支持数组
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