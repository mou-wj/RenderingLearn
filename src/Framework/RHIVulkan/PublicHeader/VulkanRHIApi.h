#pragma once
#include "vulkan/vulkan.h"
#include "RHIApi.h"




namespace RHIVulkan{
    using namespace RHI;
class VulkanDevice;


class VulkanRHIApi : public RHIApi
{
public:
    ~VulkanRHIApi() override;

    bool Init() override;
    void Shutdown() override;
    VkInstance GetInstance() const { return Instance; }

    RHIShaderLibrarySP CreateShaderLibrary(const std::string& name, ERHIShaderPlatform platform) override;

    RHITextureSP CreateTexture(const RHITextureDesc& desc) override;
    RHIBufferSP CreateBuffer(const RHIBufferDesc& desc) override;
    void UpdateTexture(RHITextureSP texture, const void* data,const RHITextureRegion& size) override;
    void UpdateBuffer(RHIBufferSP buffer, const void* data, uint64_t size) override;

    RHIGraphicsPipelineStateSP CreateGraphicsPipelineState(const RHIGraphicsPipelineStateDesc& desc) override;
    RHIComputePipelineStateSP CreateComputePipelineState(const RHIComputePipelineStateDesc& desc) override;
    RHIRayTracingPipelineStateSP CreateRayTracingsPipelineState(const RHIRayTracingPipelineStateDesc& desc) override;
    
    RHIVertexDescStateSP CreateVertexDescState(const RHIVertexDescStateDesc& desc) override;
    RHIRasterizerStateSP CreateRasterizerState(const RHIRasterizerStateDesc& desc) override;
    RHIColorBlendStateSP CreateColorBlendState(const RHIColorBlendStateDesc& desc) override;
    RHIDepthStencilStateSP CreateDepthStencilState(const RHIDepthStencilStateDesc& desc) override;

    RHIShaderSP CreateShader(const std::vector<char>& shaderSourceCode, const ERHIResourceType& shaderType) override;
    RHIVertexShaderSP CreateVertexShader(const std::vector<char>& shaderSourceCode) override;
    RHIFragmentShaderSP CreateFragmentShader(const std::vector<char>& shaderSourceCode) override;
    RHIComputeShaderSP CreateComputeShader(const std::vector<char>& shaderSourceCode) override;
    RHIGeometryShaderSP CreateGeometryShader(const std::vector<char>& shaderSourceCode) override;
    RHITessControlShaderSP CreateTessControlShader(const std::vector<char>& shaderSourceCode) override;
    RHITessEvalShaderSP CreateTessEvalShader(const std::vector<char>& shaderSourceCode) override;
    RHIMeshShaderSP CreateMeshShader(const std::vector<char>& shaderSourceCode) override;
    RHITaskShaderSP CreateTaskShader(const std::vector<char>& shaderSourceCode) override;
    RHIRayGenShaderSP CreateRayGenShader(const std::vector<char>& shaderSourceCode) override;
    RHICloseHitShaderSP CreateCloseHitShader(const std::vector<char>& shaderSourceCode) override;
    RHIMissShaderSP CreateMissShader(const std::vector<char>& shaderSourceCode) override;
    RHIAnyHitShaderSP CreateAnyHitShader(const std::vector<char>& shaderSourceCode) override;
    RHIIntersectionShaderSP CreateIntersectionShader(const std::vector<char>& shaderSourceCode) override;
    RHICallableShaderSP CreateCallableShader(const std::vector<char>& shaderSourceCode) override;
    RHIFenceSP CreateFence() override;
    RHIVIewportSP CreateViewport(void* inWindowHandle, uint32_t w, uint32_t h) override;
    RHISamplerSP CreateSampler(const RHISamplerDesc& desc) override;

    virtual RHICommandContexSP GetGlobalCommandContex() override;

    virtual RHICommandContexSP CreateCommandContex() override;


private:
	VkPhysicalDevice PickPhysicalDevice();

    std::vector<const char*> GetWantedInstanceLayers();

    std::vector<const char*> GetWantedInstanceExtensions();

	std::vector<const char*> GetWantedDeviceLayers();

    std::vector<const char*> GetWantedDeviceExtensions();



    // Vulkan 设备和上下文资源
    VkInstance Instance = VK_NULL_HANDLE;

    // 其他 Vulkan 资源管理结构
	VulkanDevice* Device = nullptr;
	VkPhysicalDevice PhysicalDevice = nullptr;
};


class VulkanRHIApiCreator: public RHIApiCreator
{
public:
    VulkanRHIApiCreator() = default;
    ~VulkanRHIApiCreator() override = default;

    RHIApi* CreateRHIApi() override
    {
        return new VulkanRHIApi();
    }

};


}