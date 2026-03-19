#include "TestBase.h"
#include "RHIApi.h"
#include "VulkanRHIApi.h"
#include "ShaderCompiler.h"
#include "Window.h"
#include "RHIPipelineStateCache.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace RenderCore;

namespace Test {

// 常量缓冲结构体
struct ComputeShaderConstants
{
    uint32_t bFlipTextureVertical;   // 是否纹理上下颠倒
    uint32_t bFlipBufferHorizontal;  // 是否缓冲左右颠倒
    uint32_t padding0;
    uint32_t padding1;
};

class RHIShaderParameterTest : public TestBase
{
public:
    RHIShaderParameterTest() = default;
    virtual ~RHIShaderParameterTest() = default;

    void Setup() override
    {
        RHI::GRHIApi = new RHIVulkan::VulkanRHIApi();
        RHI::GRHIApi->Init();
        RenderCore::GShaderCompilationCache = new RenderCore::ShaderCompilationCache();
        auto* api = RHI::GRHIApi;
        if (!api)
            return;

        CreateShaders(api);
        CreateTestResources(api);
        CreateComputeResources(api);
    }

    void Run() override
    {
        auto* api = RHI::GRHIApi;
        if (!api || !ComputePipelineState)
            return;

        auto* cmdContext = api->GetDefualtCommandContex();
        if (!cmdContext)
            return;

        auto& cmdList = cmdContext->GetCommandList();

        // ========================
        // 执行计算着色器 (4 次迭代)
        // ========================
        for (int iteration = 0; iteration < 4; ++iteration)
        {
            cmdList.SetImmediate(true);

            // 设置计算管线状态
            auto* context = cmdList.GetCommandContex();
            if (context)
            {
                context->SetComputePipelineState(ComputePipelineState.get());
            }

            // 准备计算着色器参数
            RHI::RHIBatchedShaderParameters computeParams;

            // 常量缓冲数据：根据迭代次数设置翻转标志
            ComputeShaderConstants cbData = { 0, 0, 0, 0 };

            if (iteration == 1)
                cbData.bFlipTextureVertical = 1;
            else if (iteration == 2)
                cbData.bFlipBufferHorizontal = 1;
            else if (iteration == 3)
            {
                cbData.bFlipTextureVertical = 1;
                cbData.bFlipBufferHorizontal = 1;
            }

            // 更新常量缓冲
            computeParams.Data.resize(sizeof(ComputeShaderConstants));
            memcpy(computeParams.Data.data(), &cbData, sizeof(ComputeShaderConstants));

            RHI::RHIShaderUniformParameter cbParam;
            cbParam.BufferIndex = 0;
            cbParam.BaseIndex = 0;
            cbParam.Offset = 0;
            cbParam.Size = sizeof(ComputeShaderConstants);
            computeParams.UniformParameters.push_back(cbParam);

            // 纹理 SRV（输入）
            if (TestTextureSRV)
            {
                RHI::RHIShaderResourceParameter texParam;
                texParam.Type = RHI::RHIShaderResourceParameter::EType::Texture;
                texParam.Resource = TestTexture.get();
                texParam.Index = 0;  // t0
                computeParams.ResourceParameters.push_back(texParam);
            }

            // 纹理 UAV（输出）
            if (OutputTextureUAV)
            {
                RHI::RHIShaderResourceParameter texUavParam;
                texUavParam.Type = RHI::RHIShaderResourceParameter::EType::UAV;
                texUavParam.Resource = OutputTextureUAV.get();
                texUavParam.Index = 0;  // u0
                computeParams.ResourceParameters.push_back(texUavParam);
            }

            // 缓冲区 SRV（输入）
            if (TestBufferSRV)
            {
                RHI::RHIShaderResourceParameter bufSrvParam;
                bufSrvParam.Type = RHI::RHIShaderResourceParameter::EType::SRV;
                bufSrvParam.Resource = TestBufferSRV.get();
                bufSrvParam.Index = 1;  // t1
                computeParams.ResourceParameters.push_back(bufSrvParam);
            }

            // 缓冲区 UAV（输出）
            if (OutputBufferUAV)
            {
                RHI::RHIShaderResourceParameter bufUavParam;
                bufUavParam.Type = RHI::RHIShaderResourceParameter::EType::UAV;
                bufUavParam.Resource = OutputBufferUAV.get();
                bufUavParam.Index = 1;  // u1
                computeParams.ResourceParameters.push_back(bufUavParam);
            }

            // 设置批量参数
            cmdList.SetBatchedShaderParameters(ComputeShader.get(), computeParams);

            // 分发计算任务 (2x2 纹理，每个线程组 8x8，分发 1 个线程组)
            cmdList.Dispatch(1, 1, 1);

            cmdList.ExecuteAll();

            // 提交命令
            auto ptcmdlist = api->FinalizeCommandContex(cmdContext);
            api->SubmitPlatformCommandLists({ ptcmdlist });

            cmdList.Clear();
        }
    }

    void Teardown() override
    {
        ComputeShader.reset();
        ComputePipelineState.reset();
        TestTexture.reset();
        TestTextureSRV.reset();
        TestSampler.reset();
        TestBuffer.reset();
        TestBufferSRV.reset();
        TestBufferUAV.reset();
        OutputTexture.reset();
        OutputTextureUAV.reset();
        OutputBuffer.reset();
        OutputBufferUAV.reset();
        ConstantBuffer.reset();

        auto* api = RHI::GRHIApi;
        if (!api)
            return;

        api->Shutdown();
    }

private:
    // 用于测试 SetBatchedShaderParameters 的各种资源类型
    RHI::RHITextureSP TestTexture;
    RHI::RHIShaderResourceViewSP TestTextureSRV;
    RHI::RHISamplerSP TestSampler;
    RHI::RHIBufferSP TestBuffer;
    RHI::RHIShaderResourceViewSP TestBufferSRV;
    RHI::RHIUnorderedAccessViewSP TestBufferUAV;

    // 计算着色器相关资源
    RHI::RHIComputeShaderSP ComputeShader;
    RHI::RHIComputePipelineStateSP ComputePipelineState;
    RHI::RHITextureSP OutputTexture;
    RHI::RHIUnorderedAccessViewSP OutputTextureUAV;
    RHI::RHIBufferSP OutputBuffer;
    RHI::RHIUnorderedAccessViewSP OutputBufferUAV;
    RHI::RHIBufferSP ConstantBuffer;

    void CreateShaders(RHI::RHIApi* api)
    {
        ShaderCompiler compiler;

        // =============================
        // Compute Shader
        // =============================
        ShaderCompileInput csInput;
        csInput.Frequency = ERHIShaderFrequency::Compute;
        csInput.EntryPoint = "CSMain";
        csInput.Platform = GShaderPlatform;
        csInput.VirtualSourceFilePath = "param_cs.hlsl";

        csInput.Environment.VirtualIncludes["param_cs.hlsl"] = R"(
        Texture2D InputTexture : register(t0);
        RWTexture2D<unorm float4> OutputTexture : register(u0);
        RWStructuredBuffer<float4> InputBuffer : register(t1);
        RWStructuredBuffer<float4> OutputBuffer : register(u1);

        cbuffer ComputeConstants : register(b0)
        {
            uint bFlipTextureVertical;
            uint bFlipBufferHorizontal;
            uint2 padding;
        };

        [numthreads(8, 8, 1)]
        void CSMain(uint3 DTid : SV_DispatchThreadID)
        {
            uint2 texCoord = DTid.xy;
            
            // 处理纹理拷贝（带翻转控制）
            uint2 readCoord = texCoord;
            if (bFlipTextureVertical)
            {
                readCoord.y = 1 - texCoord.y;  // 上下颠倒
            }
            
            float4 texColor = InputTexture.Load(int3(readCoord, 0));
            OutputTexture[texCoord] = texColor;
            
            // 处理缓冲区拷贝（带左右翻转控制）
            if (DTid.x == 0 && DTid.y == 0)
            {
                float4 bufData = InputBuffer[0];
                if (bFlipBufferHorizontal)
                {
                    // 左右翻转：交换 x 和 y 坐标
                    bufData = float4(bufData.y, bufData.x, bufData.w, bufData.z);
                }
                OutputBuffer[0] = bufData;
            }
        }
        )";

        ShaderCompilationOutput csOutput = compiler.Compile(csInput);
        if (!csOutput.Success)
            return;

        ComputeShader = api->CreateComputeShader(csOutput.PackedBinaryData);
    }

    void CreateTestResources(RHI::RHIApi* api)
    {
        // 创建一个简单的 2x2 RGBA 纹理并上传颜色数据
        RHI::RHITextureDesc texDesc;
        texDesc.Width = 2;
        texDesc.Height = 2;
        texDesc.Format = RHI::ERHIFormat::R8G8B8A8_UNorm;
        texDesc.Usage = RHI::ERHITextureCreateFlags::ShaderResource | RHI::ERHITextureCreateFlags::CopyDest;
        TestTexture = api->CreateTexture(texDesc);

        uint8_t texData[16] = {
            0xFF, 0x00, 0x00, 0xFF,
            0x00, 0xFF, 0x00, 0xFF,
            0x00, 0x00, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF,
        };

        auto* cmdContext = api->GetDefualtCommandContex();
        auto& cmdList = cmdContext->GetCommandList();
        cmdList.SetImmediate(true);
        api->UpdateTexture(cmdList, TestTexture.get(), texData, RHI::RHITextureRegion::Create2DRegion(2, 2));
        cmdList.ExecuteAll();

        // 采样器
        RHI::RHISamplerDesc samplerDesc;
        TestSampler = api->CreateSampler(samplerDesc);

        // 创建 SRV
        RHI::RHITexSRVCreateInfo srvDesc;
        srvDesc.MipLevelCount = 1;
        srvDesc.Format = RHI::ERHIFormat::R8G8B8A8_UNorm;
        TestTextureSRV = api->CreateTextureShaderResourceView(TestTexture.get(), srvDesc);

        // 创建一个简单的 Buffer，用于测试 SRV/UAV 绑定
        RHI::RHIBufferDesc bufDesc;
        bufDesc.Size = sizeof(glm::vec4);
        bufDesc.Usage = RHI::ERHIBufferUsageFlags::ShaderResource | RHI::ERHIBufferUsageFlags::UnorderedAccess | RHI::ERHIBufferUsageFlags::TransferDst;
        TestBuffer = api->CreateBuffer(bufDesc);

        glm::vec4 bufferData = glm::vec4(0.25f, 0.5f, 0.75f, 1.0f);
        cmdList.SetImmediate(true);
        cmdList.UpdateBuffer(TestBuffer.get(), &bufferData, { 0, sizeof(bufferData) });
        cmdList.ExecuteAll();

        RHI::RHIBufferSRVCreateInfo bufSrvDesc;
        bufSrvDesc.Offset = 0;
        bufSrvDesc.NumElements = 1;
        bufSrvDesc.Stride = sizeof(glm::vec4);
        bufSrvDesc.Format = RHI::ERHIFormat::R32G32B32A32_Float;
        TestBufferSRV = api->CreateBufferShaderResourceView(TestBuffer.get(), bufSrvDesc);

        RHI::RHIBufferUAVCreateInfo bufUavDesc;
        bufUavDesc.Offset = 0;
        bufUavDesc.NumElements = 1;
        bufUavDesc.Stride = sizeof(glm::vec4);
        bufUavDesc.Format = RHI::ERHIFormat::R32G32B32A32_Float;
        TestBufferUAV = api->CreateBufferUnorderedAccessView(TestBuffer.get(), bufUavDesc);
    }

    void CreateComputeResources(RHI::RHIApi* api)
    {
        // 创建输出纹理（UAV）
        RHI::RHITextureDesc outputTexDesc;
        outputTexDesc.Width = 2;
        outputTexDesc.Height = 2;
        outputTexDesc.Format = RHI::ERHIFormat::R8G8B8A8_UNorm;
        outputTexDesc.Usage = RHI::ERHITextureCreateFlags::UAV;
        OutputTexture = api->CreateTexture(outputTexDesc);

        // 创建输出纹理的 UAV
        RHI::RHITexUAVCreateInfo texUavDesc;
        texUavDesc.Format = RHI::ERHIFormat::R8G8B8A8_UNorm;
        OutputTextureUAV = api->CreateTextureUnorderedAccessView(OutputTexture.get(), texUavDesc);

        // 创建输出缓冲区（UAV）
        RHI::RHIBufferDesc outputBufDesc;
        outputBufDesc.Size = sizeof(glm::vec4);
        outputBufDesc.Usage = RHI::ERHIBufferUsageFlags::UnorderedAccess;
        OutputBuffer = api->CreateBuffer(outputBufDesc);

        // 创建输出缓冲区的 UAV
        RHI::RHIBufferUAVCreateInfo bufUavDesc;
        bufUavDesc.Offset = 0;
        bufUavDesc.NumElements = 1;
        bufUavDesc.Stride = sizeof(glm::vec4);
        bufUavDesc.Format = RHI::ERHIFormat::R32G32B32A32_Float;
        OutputBufferUAV = api->CreateBufferUnorderedAccessView(OutputBuffer.get(), bufUavDesc);

        // 创建常量缓冲
        RHI::RHIBufferDesc cbDesc;
        cbDesc.Size = sizeof(ComputeShaderConstants);
        cbDesc.Usage = RHI::ERHIBufferUsageFlags::Constant | RHI::ERHIBufferUsageFlags::TransferDst;
        ConstantBuffer = api->CreateBuffer(cbDesc);

        // 初始化常量缓冲数据（第一次运行：不翻转）
        ComputeShaderConstants cbData = { 0, 0, 0, 0 };
        auto* cmdContext = api->GetDefualtCommandContex();
        auto& cmdList = cmdContext->GetCommandList();
        cmdList.SetImmediate(true);
        cmdList.UpdateBuffer(ConstantBuffer.get(), &cbData, { 0, sizeof(cbData) });
        cmdList.ExecuteAll();

        // 创建计算管线状态
        RHI::RHIComputePipelineStateDesc computeDesc;
        computeDesc.computeShader = ComputeShader.get();
        ComputePipelineState = api->CreateComputePipelineState(computeDesc);
    }

};
REGISTER_RENDER_TEST("RHIShaderParameter", RHIShaderParameterTest);

} // namespace Test
