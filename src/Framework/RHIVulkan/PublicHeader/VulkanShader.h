#pragma once
#include "RHIResource.h"
#include "VulkanDevice.h"
#include "ShaderCompiledDataPacker.h"
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

    

    bool Initialize(const std::vector<char>& packedCode);
    void Cleanup();
	RenderCore::SPIRVCompiledBinaryResultPacker::Header GetShaderReflection() const { return Reflection; }
	VkShaderModule  GetShaderModule() const { return shaderModule; }
	const std::string& GetEntryPoint() const { return EntryPoint; }
    VkShaderStageFlagBits GetShaderStage() const { return ShaderStage; }
protected:
	RenderCore::SPIRVCompiledBinaryResultPacker::Header Reflection;
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VulkanDevice* Device = nullptr;
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


// hash函数，直接对 std::vector<char> 做 FNV 或 std::hash
inline size_t HashShaderCode(const std::vector<char>& code)
{
    // 简单 FNV-1a 64位
    const uint64_t fnvOffsetBasis = 14695981039346656037ULL;
    const uint64_t fnvPrime = 1099511628211ULL;

    uint64_t hash = fnvOffsetBasis;
    for (auto c : code)
    {
        hash ^= static_cast<uint8_t>(c);
        hash *= fnvPrime;
    }
    return static_cast<size_t>(hash);
}

class VulkanShaderManager
{
public:
    VulkanShaderManager(VulkanDevice* device) : Device(device) {}
    ~VulkanShaderManager() { Cleanup(); }

    // -----------------------
    // 通用模板创建接口
    // ShaderType = VulkanRHIVertexShader / VulkanRHIFragmentShader ...
    // -----------------------
    template<typename ShaderType>
    std::shared_ptr<ShaderType> GetOrCreateShader(
        const std::vector<char>& shaderCode)
    {
        const size_t hash = HashShaderCode(shaderCode);

        // 先查缓存
        {
            std::lock_guard<std::mutex> lg(ShaderMutex);
            auto it = ShaderCache.find(hash);
            if (it != ShaderCache.end())
            {
                return std::static_pointer_cast<ShaderType>(it->second);
            }
        }

        // 创建新的 shader
        auto shader = std::make_shared<ShaderType>(Device);



        if (!shader->Initialize(shaderCode))
        {
            return nullptr;
        }

        // 加入缓存
        {
            std::lock_guard<std::mutex> lg(ShaderMutex);
            ShaderCache[hash] = shader;
        }

        return shader;
    }

    void Cleanup()
    {
        std::lock_guard<std::mutex> lg(ShaderMutex);
        for (auto& kv : ShaderCache)
        {
            if (kv.second)
                kv.second->Cleanup();
        }
        ShaderCache.clear();
    }

private:
    VulkanDevice* Device = nullptr;

    std::unordered_map<size_t, std::shared_ptr<VulkanRHIShader>> ShaderCache;
    std::mutex ShaderMutex;
};



} // namespace WR::RHIVulkan
