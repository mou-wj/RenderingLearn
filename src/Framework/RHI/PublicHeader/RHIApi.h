#pragma once
#include <memory>
#include "RHIResource.h"
#include "RHICommandList.h"
#include "RHICommandContex.h"
#include "RHIShaderLibrary.h"
#include <map>
#include <string>

namespace RHI{
class RHIApi;
class RHI_API RHIApiCreator{
    public:
    RHIApiCreator() = default;
    virtual ~RHIApiCreator() = default;
    virtual RHIApi* CreateRHIApi() = 0;
};


class RHI_API RHIApiInitHelper{
public:
    static RHIApiInitHelper& Instance();
    bool InitGRHIApi(const ::std::string& apiName);
    void RegisterRHIApiCreator(const ::std::string& apiName, RHIApiCreator* creator);
    ~RHIApiInitHelper() ;
private:
    RHIApiInitHelper() = default;
    ::std::map<::std::string, RHIApiCreator*> m_RHIApiMap;


};

extern ERHIShaderPlatform GRHIShaderPlatform;

RHI_API bool InitGRHIApi(const ::std::string& apiName);
RHI_API RHIApi* GetGlobalRHIApi();


template<class RHIApiCreatorDerived>
class RHIApiCreatorRegister
{
public:
    RHIApiCreatorRegister(const ::std::string& apiName)
    {
        RHIApiInitHelper::Instance().RegisterRHIApiCreator(apiName, new RHIApiCreatorDerived());
    }

};

#define REGISTER_RHI_API_CREATOR(apiName, apiCreator) \
    static RHIApiCreatorRegister<apiCreator> g_RHIApiCreatorRegister##apiCreator(apiName);
class RHIShaderLibrary;
using RHIShaderLibrarySP = std::shared_ptr<RHIShaderLibrary>;

class RHI_API RHIApi
{
public:

    virtual ~RHIApi() = default;
	virtual bool Init() = 0;
	virtual void Shutdown() = 0;


    virtual RHIShaderLibrarySP CreateShaderLibrary(const std::string& name, ERHIShaderPlatform platform) = 0;

    virtual RHITextureSP CreateTexture(const RHITextureDesc& desc) = 0;
    virtual RHIBufferSP CreateBuffer(const RHIBufferDesc& desc) = 0;
    virtual void UpdateTexture(RHITextureSP texture, const void* data,const RHITextureRegion& size) = 0;
    virtual void UpdateBuffer(RHIBufferSP buffer, const void* data, uint64_t size) = 0;

    

    virtual RHIGraphicsPipelineStateSP CreateGraphicsPipelineState(const RHIGraphicsPipelineStateDesc& desc) = 0;
    virtual RHIComputePipelineStateSP CreateComputePipelineState(const RHIComputePipelineStateDesc& desc) = 0;
    virtual RHIRayTracingPipelineStateSP CreateRayTracingsPipelineState(const RHIRayTracingPipelineStateDesc& desc) = 0;

    virtual RHIVertexDescStateSP CreateVertexDescState(const RHIVertexDescStateDesc& desc) = 0;
    virtual RHIRasterizerStateSP CreateRasterizerState(const RHIRasterizerStateDesc& desc) = 0;
    virtual RHIColorBlendStateSP CreateColorBlendState(const RHIColorBlendStateDesc& desc) = 0;
    virtual RHIDepthStencilStateSP CreateDepthStencilState(const RHIDepthStencilStateDesc& desc) = 0;

    virtual RHIShaderSP CreateShader(const std::vector<char>& shaderSourceCode,const ERHIResourceType& shaderType) = 0;
    virtual RHIVertexShaderSP CreateVertexShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHIFragmentShaderSP CreateFragmentShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHIComputeShaderSP CreateComputeShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHIGeometryShaderSP CreateGeometryShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHITessControlShaderSP CreateTessControlShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHITessEvalShaderSP CreateTessEvalShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHIMeshShaderSP CreateMeshShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHITaskShaderSP CreateTaskShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHIRayGenShaderSP CreateRayGenShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHICloseHitShaderSP CreateCloseHitShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHIMissShaderSP CreateMissShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHIAnyHitShaderSP CreateAnyHitShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHIIntersectionShaderSP CreateIntersectionShader(const std::vector<char>& shaderSourceCode) = 0;
    virtual RHICallableShaderSP CreateCallableShader(const std::vector<char>& shaderSourceCode) = 0;

    virtual RHIFenceSP CreateFence() = 0;
    virtual RHIVIewportSP CreateViewport(void* inWindowHandle,uint32_t w,uint32_t h) = 0;
    virtual RHISamplerSP CreateSampler(const RHISamplerDesc& desc) = 0;

    virtual RHICommandContexSP GetGlobalCommandContex() = 0;

    virtual RHICommandContexSP CreateCommandContex() = 0;

 
};






}