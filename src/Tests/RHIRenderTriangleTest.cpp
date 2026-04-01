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

// 简单的顶点结构体
struct SimpleVertex
{
    glm::vec3 Position;
    glm::vec3 Color;
};

class RHIRenderTriangleTest : public TestBase
{
public:
    RHIRenderTriangleTest() = default;
    virtual ~RHIRenderTriangleTest() = default;

    // ==========================================
    // Setup - 初始化所有渲染资源
    // ==========================================
    void Setup() override
    {
        RHI::GRHIApi = new RHIVulkan::VulkanRHIApi();
        RHI::GRHIApi->Init();
		RenderCore::GShaderCompilationCache = new RenderCore::ShaderCompilationCache();
        auto* api = RHI::GRHIApi;
        if (!api)
        {
            return;
        }

        // 1. 创建顶点缓冲区
        CreateVertexBuffer(api);

        // 2. 创建着色器
        CreateShaders(api);

        // 3. 创建顶点描述
        CreateVertexDescription(api);

        // 4. 创建管线状态
        CreatePipelineState(api);

        // 5. 创建交换链
        CreateSwapchain(api);

        // 6. 创建深度缓冲区
        CreateRenderTarget(api);

    }

    // ==========================================
    // Tick - 每帧执行渲染
    // ==========================================
    void Run() override
    {
        auto* api = RHI::GRHIApi;
        if (!api || !TriangleGraphicsPipelineState)
        {
            return;
        }

        auto* queue = api->GetQueue(RHI::EQueueType::Graphics);
        if (!queue)
        {
            return;
        }

        auto* cmdContext = queue->AcquireCommandContext();
        if (!cmdContext)
        {
            return;
        }

        auto& cmdList = cmdContext->GetCommandList();
        int i = 0;
        while (true) {
            i++;
            // 设置为即时执行模式
            cmdList.SetImmediate(true);

            // 设置图形管线状态
            cmdList.SetGraphicPipelineState(TriangleGraphicsPipelineState.get());

            // 设置顶点缓冲区流
            cmdList.SetStreamSource(0, VertexBuffer, 0);

            // 设置视口
            RHI::RHIRect viewport;
            viewport.X = 0;
            viewport.Y = 0;
            viewport.Width = 512;
            viewport.Height = 512;
            cmdList.SetViewport(0,0,512,512,0,1);
            cmdList.SetScissor(0, 0, 512, 512);

            auto swapchainSlot = Swapchain->AcquireNextSlot();
            auto* backTexture = swapchainSlot.Texture;
            if (!backTexture)
            {
                continue;
            }

            RHIRenderPassInfo passInfo;
            passInfo.RenderTargets.Bound(TriangleGraphicsPipelineState->GetDesc().attachmentDesc, backTexture, depthStencilTexture.get());
            passInfo.RenderTargets.ColorAttachments[0].ClearBinding.Color[0] = 1;
            passInfo.RenderTargets.ColorAttachments[0].ClearBinding.Color[3] = 1;
			passInfo.RenderTargets.DepthStencil.ClearBinding.Depth = 1.0f;
            passInfo.RenderArea.Width = FrameWidth;
            passInfo.RenderArea.Height = FrameHeight;
            cmdList.Begin();
            cmdList.BeginRenderPass(passInfo);
            // 绘制三角形 (3个顶点, 1个实例)
            cmdList.Draw(3, 1, 0, 0);
            cmdList.EndRenderPass();
            // 执行所有命令
            cmdList.ExecuteAll();
            RHI::RHICmdBuffer cmdBuffer = cmdList.End();
            RHI::RHISyncPoint* syncPoint = queue->Submit(cmdBuffer);
            delete syncPoint;
            GRHIApi->GetPresentExecutor()->Present(Swapchain.get());
            cmdList.Clear();
        }
    }

    // ==========================================
    // Teardown - 清理资源
    // ==========================================
    void Teardown() override
    {
        // 释放所有创建的资源
        VertexBuffer.reset();
        VertexShader.reset();
        FragmentShader.reset();
        TriangleGraphicsPipelineState.reset();
        VertexDescState.reset();
        depthStencilTexture.reset();
        Swapchain.reset();
		Window.reset();
        auto* api = RHI::GRHIApi;
        if (!api)
        {
            return;
        }
		api->Shutdown();
        delete api;
    }

private:
    // ==========================================
    // 私有资源成员
    // ==========================================
    RHI::RHIBufferSP VertexBuffer;
    RHI::RHIVertexShaderSP VertexShader;
    RHI::RHIFragmentShaderSP FragmentShader;
    RHI::RHIGraphicsPipelineStateSP TriangleGraphicsPipelineState;
    RHI::RHIVertexDescStateSP VertexDescState;
    RHI::RHISwapchainSP Swapchain;
    Slate::WindowSP Window;
    int WindiwWidth = 512;
    int WindowHeight = 512;
    int FrameWidth = 0;
    int FrameHeight = 0;
    RHI::RHITextureSP depthStencilTexture;

    // ==========================================
    // 辅助函数：创建顶点缓冲区
    // ==========================================
    void CreateVertexBuffer(RHI::RHIApi* api)
    {
        // 定义三角形顶点数据
        SimpleVertex vertices[] = {
            // 位置                    // 颜色
            {{0.0f, 0.5f, 0.0f},      {1.0f, 0.0f, 0.0f}},  // 顶点
            {{-0.5f, -0.5f, 0.0f},    {0.0f, 1.0f, 0.0f}},  // 左下
            {{0.5f, -0.5f, 0.0f},     {0.0f, 0.0f, 1.0f}}   // 右下
        };

        // 创建顶点缓冲区描述
        RHI::RHIBufferDesc bufferDesc;
        bufferDesc.Size = sizeof(vertices);
        bufferDesc.Usage = RHI::ERHIBufferUsageFlags::Vertex | RHI::ERHIBufferUsageFlags::TransferDst;

        // 创建缓冲区
        VertexBuffer = api->CreateBuffer(bufferDesc);

        // 将顶点数据拷贝到缓冲区
		auto* queue = api->GetQueue(RHI::EQueueType::Graphics);
		if (!queue)
		{
			return;
		}
		auto commandContext = queue->AcquireCommandContext();
        auto& commandList = commandContext->GetCommandList();
        commandList.SetImmediate(true);
        commandContext->Begin();
        commandList.UpdateBuffer(VertexBuffer.get(), vertices, { 0, sizeof(vertices) });
        commandList.ExecuteAll();
        RHI::RHICmdBuffer cmdBuffer = commandList.End();
        RHI::RHISyncPoint* syncPoint = queue->Submit(cmdBuffer);
        delete syncPoint;

    }

    // ==========================================
    // 辅助函数：创建着色器
    // ==========================================
    void CreateShaders(RHI::RHIApi* api)
    {
        ShaderCompiler compiler;

        // =============================
        // Vertex Shader
        // =============================
        ShaderCompileInput vsInput;
        vsInput.Frequency = ERHIShaderFrequency::Vertex;
        vsInput.EntryPoint = "VSMain";
        vsInput.Platform = GShaderPlatform;
        vsInput.VirtualSourceFilePath = "simple_vs.hlsl";

        // HLSL源码（真实项目一般从文件读取）
        vsInput.Environment.VirtualIncludes["simple_vs.hlsl"] = R"(
        struct VSInput
        {
            float3 pos : POSITION;
        };

        struct VSOutput
        {
            float4 pos : SV_POSITION;
        };

        VSOutput VSMain(VSInput input)
        {
            VSOutput o;
            o.pos = float4(input.pos, 1.0);
            return o;
        }
    )";



        ShaderCompilationOutput vsOutput;
        vsOutput = compiler.Compile(vsInput);

        if (!vsOutput.Success)
        {
            return;
        }

        VertexShader = api->CreateVertexShader(vsOutput.PackedBinaryData);


        // =============================
        // Fragment Shader
        // =============================
        ShaderCompileInput psInput;
        psInput.Frequency = ERHIShaderFrequency::Fragment;
        psInput.EntryPoint = "PSMain";
        psInput.Platform = GShaderPlatform;
        psInput.VirtualSourceFilePath = "simple_ps.hlsl";

        psInput.Environment.VirtualIncludes["simple_ps.hlsl"] = R"(
        float4 PSMain() : SV_Target0
        {
            return float4(1,1,0,1);
        }
        )";

        ShaderCompilationOutput psOutput;
        psOutput = compiler.Compile(psInput);

        if (!psOutput.Success)
        {
            
            return;
        }

        FragmentShader = api->CreateFragmentShader(psOutput.PackedBinaryData);
    }

    // ==========================================
    // 辅助函数：创建顶点描述
    // ==========================================
    void CreateVertexDescription(RHI::RHIApi* api)
    {
        // 创建顶点绑定描述
        RHI::RHIVertexBindingDesc bindingDesc;
        bindingDesc.binding = 0;
        bindingDesc.stride = sizeof(SimpleVertex);
        bindingDesc.inputRate = RHI::ERHIInputRate::PerVertex;

        // 创建顶点属性描述
        std::vector<RHI::RHIVertexAttributeDesc> attributes;

        // 位置属性（vec3）
        RHI::RHIVertexAttributeDesc posAttr;
        posAttr.location = 0;
        posAttr.format = RHI::ERHIFormat::R32G32B32_Float;
        posAttr.offset = offsetof(SimpleVertex, Position);
        RHI::RHIVertexAttributeDesc colorAttr;
        colorAttr.location = 1;
        colorAttr.format = RHI::ERHIFormat::R32G32B32_Float;
        colorAttr.offset = offsetof(SimpleVertex, Color);

        attributes.push_back(posAttr);
        attributes.push_back(colorAttr);


        // 创建顶点描述状态
        RHI::RHIVertexDescStateDesc descDesc;
        descDesc.bindings = {bindingDesc};
        descDesc.attributes = attributes;

        VertexDescState = RHIPipelineStateCache::GetOrCreateVertexDescState(descDesc);
    }

    // ==========================================
    // 辅助函数：创建管线状态
    // ==========================================
    void CreatePipelineState(RHI::RHIApi* api)
    {
        // 创建光栅化状态
        RHI::RHIRasterizerStateDesc rasterizerDesc;
        rasterizerDesc.polygonMode = RHI::ERHIPolygonMode::Fill;
        rasterizerDesc.cullMode = RHI::ERHICullMode::Back;
        rasterizerDesc.frontFace = RHI::ERHIFrontFace::Clockwise;
        rasterizerDesc.lineWidth = 1.0f;
        rasterizerDesc.depthBiasEnable = false;

        auto rasterizerState = RHIPipelineStateCache::GetOrCreateRasterizerState(rasterizerDesc);

        // 创建颜色混合状态
        RHI::RHIColorBlendAttachmentDesc blendAttachDesc;
        blendAttachDesc.blendEnable = false;

        std::vector<RHI::RHIColorBlendAttachmentDesc> attachments = {blendAttachDesc};

        RHI::RHIColorBlendStateDesc blendDesc;
        blendDesc.attachments = attachments;

        auto colorBlendState = RHIPipelineStateCache::GetOrCreateColorBlendState(blendDesc);

        // 创建深度模板状态
        RHI::RHIDepthStencilStateDesc depthStencilDesc;
        depthStencilDesc.depthTestEnable = true;
        depthStencilDesc.depthWriteEnable = true;
        depthStencilDesc.depthCompareOp = RHI::ERHICompareOp::Less;

        auto depthStencilState = RHIPipelineStateCache::GetOrCreateDepthStencilState(depthStencilDesc);

        // 创建图形管线状态
        RHI::RHIGraphicsPipelineStateDesc pipelineDesc;
        pipelineDesc.shaderStages.vertexShader = VertexShader.get();
        pipelineDesc.shaderStages.fragmentShader = FragmentShader.get();
        // 这里可以设置更多管线配置...
        pipelineDesc.rasterizerState = rasterizerState.get();
        pipelineDesc.colorBlendState = colorBlendState.get();
        pipelineDesc.depthStencilState = depthStencilState.get();
        pipelineDesc.vertexDescState = VertexDescState.get();

        //rendertarget info
		pipelineDesc.attachmentDesc.colorAttachmentCount = 1;
		pipelineDesc.attachmentDesc.colorAttachments[0].format = RHI::ERHIFormat::B8G8R8A8_UNorm;
		pipelineDesc.attachmentDesc.colorAttachments[0].actions = ERenderTargetActions::Clear_Store;
        pipelineDesc.attachmentDesc.depthActions  = ERenderTargetActions::Clear_Store;
        pipelineDesc.attachmentDesc.enableDepth = true;
        pipelineDesc.attachmentDesc.depthStencilFormat = RHI::ERHIFormat::D32_Float;
        
        TriangleGraphicsPipelineState = RHIPipelineStateCache::GetOrCreateGraphicsPipelineState(pipelineDesc);
    }

    void CreateSwapchain(RHI::RHIApi* api)
	{
        // 
        Window = Slate::WindowFactory::CreateWindowSP(WindiwWidth, WindowHeight, "RHIRenderTriangleTest");
        Window->Show();
        // 创建交换链
		void* windowHandle = Window->GetNativeHandle(); // 获取窗口句柄的函数
		uint32_t width = WindiwWidth;
		uint32_t height = WindowHeight;
		ERHIFormat format = ERHIFormat::B8G8R8A8_UNorm;
        Swapchain = api->CreateSwapchain(windowHandle, width, height, format);
        auto frameSize = Window->GetFramebufferSize();
        FrameWidth = frameSize.x;
        FrameHeight = frameSize.y;

	}

    void CreateRenderTarget(RHI::RHIApi* api) {
        // 创建渲染目标
        RHI::RHITextureDesc depthTargetDesc;
        depthTargetDesc.Width = FrameWidth;
        depthTargetDesc.Height = FrameHeight;
        depthTargetDesc.Format = RHI::ERHIFormat::D32_Float;
        depthTargetDesc.Usage = RHI::ERHITextureCreateFlags::DepthStencil;
        depthStencilTexture = api->CreateTexture(depthTargetDesc);

    }

};

// ==========================================
// 注册测试用例
// ==========================================
REGISTER_RENDER_TEST("RHIRenderTriangleTest", RHIRenderTriangleTest);

} // namespace Test
