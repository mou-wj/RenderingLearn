#pragma once
#include "vulkan/vulkan.h"
#include "RHIApi.h"




namespace RHIVulkan{
    using namespace RHI;
class VulkanDevice;


class RHIVULKAN_API VulkanRHIApi : public RHIApi
{
public:
    ~VulkanRHIApi() override;

    bool Init() override;
    void Shutdown() override;
    VkInstance GetInstance() const { return Instance; }

    RHIShaderLibrarySP CreateShaderLibrary(const std::string& name, ERHIShaderPlatform platform) override;

    RHITextureSP CreateTexture(const RHITextureDesc& desc) override;
    RHIBufferSP CreateBuffer(const RHIBufferDesc& desc) override;
    void UpdateTexture(RHICommandList& cmdList, RHITexture* texture, const void* data,const RHITextureRegion& size) override;
    void UpdateBuffer(RHICommandList& cmdList, RHIBuffer* buffer, const void* data, const RHIBufferRegion& region) override;

    RHIShaderResourceViewSP CreateTextureShaderResourceView(
        RHITexture* Texture, const RHITexSRVCreateInfo& Desc) override;

    RHIUnorderedAccessViewSP CreateTextureUnorderedAccessView(
        RHITexture* Texture, const RHITexUAVCreateInfo& Desc) override;

    RHIShaderResourceViewSP CreateBufferShaderResourceView(
        RHIBuffer* Buffer, const RHIBufferSRVCreateInfo& Desc) override;

    RHIUnorderedAccessViewSP CreateBufferUnorderedAccessView(
        RHIBuffer* Buffer, const RHIBufferUAVCreateInfo& Desc) override;

    // 创建 StagingBuffer
    RHIStagingBufferSP CreateStagingBuffer(uint32_t size) override;

    RHIGraphicsPipelineStateSP CreateGraphicsPipelineState(const RHIGraphicsPipelineStateDesc& desc) override;
    RHIComputePipelineStateSP CreateComputePipelineState(const RHIComputePipelineStateDesc& desc) override;
    RHIRayTracingPipelineStateSP CreateRayTracingsPipelineState(const RHIRayTracingPipelineStateDesc& desc) override;
    
    RHIVertexDescStateSP CreateVertexDescState(const RHIVertexDescStateDesc& desc) override;
    RHIRasterizerStateSP CreateRasterizerState(const RHIRasterizerStateDesc& desc) override;
    RHIColorBlendStateSP CreateColorBlendState(const RHIColorBlendStateDesc& desc) override;
    RHIDepthStencilStateSP CreateDepthStencilState(const RHIDepthStencilStateDesc& desc) override;

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
    RHIViewportSP CreateViewport(void* inWindowHandle, uint32_t w, uint32_t h, ERHIFormat format) override;
    RHITextureSP GetViewportBackBuffer(RHIViewport* viewport) override;
    RHISamplerSP CreateSampler(const RHISamplerDesc& desc) override;

    RHICommandContex* GetDefualtCommandContex() override;

    RHITransientResourceManagerSP CreateTransientResourceManager() override;

    virtual RHIPlatformCommandList* FinalizeCommandContex(RHICommandContex* contex) override;
    virtual void SubmitPlatformCommandLists(std::vector<RHIPlatformCommandList*> cmdLists) override;
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


class RHIVULKAN_API VulkanRHIModule final : public RHI::RHIModule
{
public:
    VulkanRHIModule();
    ~VulkanRHIModule() override;

    // ===== Module 接口 =====
    void StartupModule() override;
    void ShutdownModule() override;
    bool IsLoaded() const override;

    // ===== RHIInterface =====
    RHIApi* CreateRHIApi() override;

private:
    bool bLoaded = false;
    std::string ModuleName = "VulkanRHI";
};

}