#pragma once
#include "RHIResource.h"
#include "VulkanDevice.h"
#include <vector>
#include <vulkan/vulkan.h>
#include <map>

namespace RHIVulkan {

// ----------------------------
// Vulkan 描述符布局结构体
// ----------------------------
struct VulkanDescriptorSetLayoutInfo
{
    std::vector<VkDescriptorSetLayoutBinding> Bindings;
    bool operator==(const VulkanDescriptorSetLayoutInfo& rhs) const
    {
        if (Bindings.size() != rhs.Bindings.size()) return false;
        for (size_t i = 0; i < Bindings.size(); ++i)
        {
            const auto& a = Bindings[i];
            const auto& b = rhs.Bindings[i];
            if (a.binding != b.binding ||
                a.descriptorType != b.descriptorType ||
                a.descriptorCount != b.descriptorCount ||
                a.stageFlags != b.stageFlags)
                return false;
        }
        return true;
    }
};

// ----------------------------
// VulkanRHIShader Base
// ----------------------------
class VulkanRHIShader
{
public:
    VulkanRHIShader(VulkanDevice* device);
    virtual ~VulkanRHIShader();

    

    bool Initialize(const std::vector<char>& spirvCode);
    void Cleanup();

    // 新增：描述符布局信息
    const std::map<uint32_t, VulkanDescriptorSetLayoutInfo>& GetDescriptorSetLayouts() const { return DescriptorSetLayouts; }
	VkShaderModule  GetShaderModule() const { return shaderModule; }
	const std::string GetEntryPoint() const { return EntryPoint; }
    VkShaderStageFlagBits GetShaderStage() const { return ShaderStage; }
protected:
    void ParseShaderDescriptorLayout(const std::vector<char>& spirvCode);
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VulkanDevice* Device = nullptr;
    std::map<uint32_t,VulkanDescriptorSetLayoutInfo> DescriptorSetLayouts;
    std::string EntryPoint;
    VkShaderStageFlagBits ShaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
};

// 各具体Shader类型，继承对应RHIShader和VulkanRHIShader
class VulkanRHIVertexShader : public RHIVertexShader, public VulkanRHIShader
{
public:
    VulkanRHIVertexShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHIVertexShader() = default;
};

class VulkanRHIFragmentShader : public RHIFragmentShader, public VulkanRHIShader
{
public:
    VulkanRHIFragmentShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHIFragmentShader() = default;
};

class VulkanRHIGeometryShader : public RHIGeometryShader, public VulkanRHIShader
{
public:
    VulkanRHIGeometryShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHIGeometryShader() = default;
};

class VulkanRHIComputeShader : public RHIComputeShader, public VulkanRHIShader
{
public:
    VulkanRHIComputeShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHIComputeShader() = default;
};

class VulkanRHITessControlShader : public RHITessControlShader, public VulkanRHIShader
{
public:
    VulkanRHITessControlShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHITessControlShader() = default;
};

class VulkanRHITessEvalShader : public RHITessEvalShader, public VulkanRHIShader
{
public:
    VulkanRHITessEvalShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHITessEvalShader() = default;
};

class VulkanRHIMeshShader : public RHIMeshShader, public VulkanRHIShader
{
public:
    VulkanRHIMeshShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHIMeshShader() = default;
};

class VulkanRHITaskShader : public RHITaskShader, public VulkanRHIShader
{
public:
    VulkanRHITaskShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHITaskShader() = default;
};

// 光线追踪相关着色器
class VulkanRHIRayGenShader : public RHIRayGenShader, public VulkanRHIShader
{
public:
    VulkanRHIRayGenShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHIRayGenShader() = default;
};

class VulkanRHICloseHitShader : public RHICloseHitShader, public VulkanRHIShader
{
public:
    VulkanRHICloseHitShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHICloseHitShader() = default;
};

class VulkanRHIMissShader : public RHIMissShader, public VulkanRHIShader
{
public:
    VulkanRHIMissShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHIMissShader() = default;
};

class VulkanRHIAnyHitShader : public RHIAnyHitShader, public VulkanRHIShader
{
public:
    VulkanRHIAnyHitShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHIAnyHitShader() = default;
};

class VulkanRHIIntersectionShader : public RHIIntersectionShader, public VulkanRHIShader
{
public:
    VulkanRHIIntersectionShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHIIntersectionShader() = default;
};

class VulkanRHICallableShader : public RHICallableShader, public VulkanRHIShader
{
public:
    VulkanRHICallableShader(VulkanDevice* device) : VulkanRHIShader(device) {}
    virtual ~VulkanRHICallableShader() = default;
};

} // namespace WR::RHIVulkan
