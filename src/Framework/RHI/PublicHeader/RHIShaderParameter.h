#pragma once
#include <string>
#include <cstdint>
#include "RHIResource.h"

namespace RHI
{

// 着色器参数类型
enum class ERHIShaderParameterType
{
    Unknown = 0,
    ConstantBuffer,
    Texture,
    Sampler,
    Buffer,
    UAV,
    // 可扩展
};

// 着色器参数描述
struct RHIShaderParameterDesc
{
    std::string Name;                    // 参数名
    ERHIShaderParameterType Type = ERHIShaderParameterType::Unknown;
    uint32_t BindSlot = 0;               // 绑定槽
    uint32_t Size = 0;                   // 对于CB/Buffer等
    uint32_t Offset = 0;                 // 偏移（如结构体成员）
    // 可扩展：数组大小、结构体类型等
};

// 着色器参数基类
class RHIShaderParameter
{
public:
    RHIShaderParameter(const RHIShaderParameterDesc& desc)
        : Desc(desc) {}
    virtual ~RHIShaderParameter() = default;

    const RHIShaderParameterDesc& GetDesc() const { return Desc; }

    // 资源指针（可根据Type动态指向不同资源类型）
    std::shared_ptr<RHIResource> ResourceRef;

protected:
    RHIShaderParameterDesc Desc;
};

using RHIShaderParameterSP = std::shared_ptr<RHIShaderParameter>;

struct RHIBatchedShaderParameter{
    std::vector<RHIShaderParameterSP> Parameters; // 批量参数列表
};


}