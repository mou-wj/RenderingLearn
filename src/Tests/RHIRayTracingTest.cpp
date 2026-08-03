#include "TestBase.h"
#include "RHIApi.h"
#include "VulkanRHIApi.h"
#include "ShaderCompiler.h"
#include "Window.h"

#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace RenderCore;

namespace Test {

namespace {
std::unordered_map<const RHI::RHIViewableResource*, RHI::ERHIResourceAccess> GTrackedViewableAccess;
std::unordered_map<const RHI::RHIRayTracingAccelerationStructure*, RHI::ERHIResourceAccess> GTrackedBvhAccess;

void EmitTransition(
    RHI::RHIApi* api,
    RHI::RHICommandListBase& cmdList,
    std::vector<RHI::RHITransitionInfo>&& infos)
{
    if (!api || infos.empty())
    {
        return;
    }

    char* transitionMem = new char[RHI::G_RHITransition_TotalSize];
    auto* transition = new (transitionMem) RHI::RHITransition();
    api->RHICreateTransition(
        transition,
        RHI::RHITransitionCreateInfo(RHI::ERHITransitionCreateFlags::None, std::move(infos)));

    cmdList.BeginTransitions({ transition });
    cmdList.EndTransitions({ transition });

    api->RHIReleaseTransition(transition);
    delete[] transitionMem;
}

void TransitionViewableResource(
    RHI::RHIApi* api,
    RHI::RHICommandListBase& cmdList,
    RHI::RHIViewableResource* resource,
    RHI::ERHIResourceAccess targetAccess)
{
    if (!api || !resource)
    {
        return;
    }

    const auto it = GTrackedViewableAccess.find(resource);
    const RHI::ERHIResourceAccess currentAccess =
        it != GTrackedViewableAccess.end() ? it->second : RHI::ERHIResourceAccess::Unknown;

    if (currentAccess == targetAccess)
    {
        return;
    }

    std::vector<RHI::RHITransitionInfo> infos;
    if (auto* texture = dynamic_cast<RHI::RHITexture*>(resource))
    {
        infos.emplace_back(texture, currentAccess, targetAccess);
    }
    else if (auto* buffer = dynamic_cast<RHI::RHIBuffer*>(resource))
    {
        infos.emplace_back(buffer, currentAccess, targetAccess);
    }
    else
    {
        return;
    }

    EmitTransition(api, cmdList, std::move(infos));
    GTrackedViewableAccess[resource] = targetAccess;
}

void TransitionAccelerationStructure(
    RHI::RHIApi* api,
    RHI::RHICommandListBase& cmdList,
    RHI::RHIRayTracingAccelerationStructure* accelerationStructure,
    RHI::ERHIResourceAccess targetAccess)
{
    if (!api || !accelerationStructure)
    {
        return;
    }

    const auto it = GTrackedBvhAccess.find(accelerationStructure);
    const RHI::ERHIResourceAccess currentAccess =
        it != GTrackedBvhAccess.end() ? it->second : RHI::ERHIResourceAccess::Unknown;

    if (currentAccess == targetAccess)
    {
        return;
    }

    std::vector<RHI::RHITransitionInfo> infos;
    infos.emplace_back(accelerationStructure, currentAccess, targetAccess);

    EmitTransition(api, cmdList, std::move(infos));
    GTrackedBvhAccess[accelerationStructure] = targetAccess;
}
}

class RHIRayTracingTest : public TestBase
{
public:
    RHIRayTracingTest() = default;
    ~RHIRayTracingTest() override = default;

    void Setup() override
    {
        RHI::GRHIApi = new RHIVulkan::VulkanRHIApi();
        RHI::GRHIApi->Init();

        auto* api = RHI::GRHIApi;
        if (!api)
        {
            return;
        }

        CreateSwapchain(api);
        CreateGeometryBuffers(api);
        CreateAccelerationStructures(api);
        CreateRayTracingPipeline(api);
    }

    void Run() override
    {
        auto* api = RHI::GRHIApi;
        if (!api || !TopLevelAS)
        {
            return;
        }

        auto* queue = api->GetQueue(RHI::EQueueType::Graphics);
        if (!queue)
        {
            return;
        }

        constexpr int kFrameCount = 300;
        for (int frameIndex = 0; frameIndex < kFrameCount; ++frameIndex)
        {
            auto* cmdContext = queue->AcquireCommandContext();
            if (!cmdContext)
            {
                return;
            }

            auto* graphicContext = dynamic_cast<RHI::RHIGraphicContex*>(cmdContext);
            if (!graphicContext)
            {
                return;
            }

            RHI::RHIGraphicCommandList cmdList(graphicContext);
            cmdList.SetImmediate(true);
            cmdList.Begin();

            if (!bAccelerationBuilt)
            {
                if (BottomLevelAS)
                {
                    TransitionAccelerationStructure(api, cmdList, BottomLevelAS.get(), RHI::ERHIResourceAccess::BVHWrite);
                    cmdList.BuildAccelerationStructure(BottomLevelAS.get());
                    TransitionAccelerationStructure(api, cmdList, BottomLevelAS.get(), RHI::ERHIResourceAccess::BVHRead);
                }

                TransitionAccelerationStructure(api, cmdList, TopLevelAS.get(), RHI::ERHIResourceAccess::BVHWrite);
                cmdList.BuildAccelerationStructure(TopLevelAS.get());
                TransitionAccelerationStructure(api, cmdList, TopLevelAS.get(), RHI::ERHIResourceAccess::BVHRead);
                bAccelerationBuilt = true;
            }

            auto swapchainSlot = Swapchain->AcquireNextSlot();
            auto* backTexture = swapchainSlot.Texture;
            if (backTexture)
            {
                TransitionViewableResource(api, cmdList, backTexture, RHI::ERHIResourceAccess::Present);
            }

            if (RayTracingPipelineState)
            {
                cmdList.SetRayTracingPipelineState(RayTracingPipelineState.get());
                cmdList.SetRayTracingAccelerationStructure(TopLevelAS.get());
                cmdList.TraceRays(static_cast<uint32_t>(FrameWidth), static_cast<uint32_t>(FrameHeight), 1);
            }

            cmdList.End();
            cmdList.ExecuteAll();

            std::vector<RHI::RHIWaitInfo> waitInfos;
            if (swapchainSlot.ReadySync)
            {
                waitInfos.push_back({ swapchainSlot.ReadySync, RHI::EQueueType::Graphics, 0, RHI::ERHIPipelineStage::ColorAttachmentOutput });
            }

            RHI::RHIFence submitResult = queue->ExecuteContext({ graphicContext }, waitInfos);
            queue->WaitFence(submitResult);

            if (Swapchain)
            {
                RHI::RHIWaitInfo presentWait;
                presentWait.SyncPoint = queue->GetSyncPoint();
                presentWait.Value = submitResult.Value;
                presentWait.WaitStage = RHI::ERHIPipelineStage::ColorAttachmentOutput;
                RHI::GRHIApi->GetPresentExecutor()->Present(Swapchain.get(), presentWait);
            }

            cmdList.Clear();
        }

        queue->WaitIdle();
    }

    void Teardown() override
    {
        RayTracingPipelineState.reset();
        RayGenShader.reset();
        MissShader.reset();
        ClosestHitShader.reset();
        TopLevelAS.reset();
        BottomLevelAS.reset();
        IndexBuffer.reset();
        VertexBuffer.reset();
        Swapchain.reset();
        Window.reset();

        auto* api = RHI::GRHIApi;
        if (!api)
        {
            return;
        }

        api->Shutdown();
    }

private:
    void CreateSwapchain(RHI::RHIApi* api)
    {
        if (!api)
        {
            return;
        }

        Window = SlateCore::WindowFactory::CreateWindowSP(WindowWidth, WindowHeight, "RHIRayTracingTest");
        Window->Show();

        void* windowHandle = Window->GetNativeHandle();
        Swapchain = api->CreateSwapchain(windowHandle, static_cast<uint32_t>(WindowWidth), static_cast<uint32_t>(WindowHeight), RHI::ERHIFormat::B8G8R8A8_UNorm);

        auto frameSize = Window->GetFramebufferSize();
        FrameWidth = frameSize.x;
        FrameHeight = frameSize.y;
    }

    void CreateGeometryBuffers(RHI::RHIApi* api)
    {
        if (!api)
        {
            return;
        }

        struct TriangleVertex
        {
            float X;
            float Y;
            float Z;
        };

        const TriangleVertex vertices[3] = {
            { 0.0f,  0.6f, 0.0f },
            {-0.6f, -0.6f, 0.0f },
            { 0.6f, -0.6f, 0.0f }
        };

        const uint32_t indices[3] = { 0, 1, 2 };

        RHI::RHIBufferDesc vertexDesc{};
        vertexDesc.Size = sizeof(vertices);
        vertexDesc.Stride = sizeof(TriangleVertex);
        vertexDesc.Usage = RHI::ERHIBufferUsageFlag::Vertex |
            RHI::ERHIBufferUsageFlag::ShaderResource |
            RHI::ERHIBufferUsageFlag::TransferDst;
        vertexDesc.InitialQueueType = RHI::EQueueType::Graphics;
        VertexBuffer = api->CreateBuffer(vertexDesc);

        RHI::RHIBufferDesc indexDesc{};
        indexDesc.Size = sizeof(indices);
        indexDesc.Stride = sizeof(uint32_t);
        indexDesc.Usage = RHI::ERHIBufferUsageFlag::Index |
            RHI::ERHIBufferUsageFlag::ShaderResource |
            RHI::ERHIBufferUsageFlag::TransferDst;
        indexDesc.InitialQueueType = RHI::EQueueType::Graphics;
        IndexBuffer = api->CreateBuffer(indexDesc);

        auto* queue = api->GetQueue(RHI::EQueueType::Graphics);
        if (!queue || !VertexBuffer || !IndexBuffer)
        {
            return;
        }

        auto* cmdContext = queue->AcquireCommandContext();
        auto* graphicContext = dynamic_cast<RHI::RHIGraphicContex*>(cmdContext);
        if (!graphicContext)
        {
            return;
        }

        RHI::RHIGraphicCommandList cmdList(graphicContext);
        cmdList.SetImmediate(true);
        cmdList.Begin();

        TransitionViewableResource(api, cmdList, VertexBuffer.get(), RHI::ERHIResourceAccess::TransferDest);
        api->UpdateBuffer(cmdList, VertexBuffer.get(), vertices, { 0, sizeof(vertices) });
        TransitionViewableResource(api, cmdList, VertexBuffer.get(), RHI::ERHIResourceAccess::SRV);

        TransitionViewableResource(api, cmdList, IndexBuffer.get(), RHI::ERHIResourceAccess::TransferDest);
        api->UpdateBuffer(cmdList, IndexBuffer.get(), indices, { 0, sizeof(indices) });
        TransitionViewableResource(api, cmdList, IndexBuffer.get(), RHI::ERHIResourceAccess::SRV);

        cmdList.End();
        cmdList.ExecuteAll();
        queue->ExecuteContext(graphicContext);
    }

    void CreateAccelerationStructures(RHI::RHIApi* api)
    {
        if (!api || !VertexBuffer || !IndexBuffer)
        {
            return;
        }

        RHI::RHIRayTracingGeometryDesc geometryDesc{};
        geometryDesc.VertexBuffer = VertexBuffer.get();
        geometryDesc.IndexBuffer = IndexBuffer.get();
        geometryDesc.VertexStride = sizeof(float) * 3;
        geometryDesc.VertexCount = 3;
        geometryDesc.IndexCount = 3;
        geometryDesc.AllowUpdate = true;
        BottomLevelAS = api->CreateRayTracingGeometry(geometryDesc);

        RHI::RHIRayTracingInstancesDesc instancesDesc{};
        if (BottomLevelAS)
        {
            RHI::RHIRayTracingInstanceDesc instanceDesc{};
            instanceDesc.Geometry = BottomLevelAS.get();
            instanceDesc.Transform = Core::Float4x4::Identity();
            instanceDesc.InstanceID = 0;
            instanceDesc.Mask = 0xFF;
            instancesDesc.Instances.push_back(instanceDesc);
        }

        TopLevelAS = api->CreateRayTracingInstance(instancesDesc);
    }

    void CreateRayTracingPipeline(RHI::RHIApi* api)
    {
        if (!api)
        {
            return;
        }

        ShaderCompileInput rayGenInput;
        rayGenInput.Frequency = RHI::ERHIShaderFrequency::RayGen;
        rayGenInput.EntryPoint = "RayGenMain";
        rayGenInput.Platform = RHI::GShaderPlatform;
        rayGenInput.VirtualSourceFilePath = "basic_rt_raygen.hlsl";
        rayGenInput.Environment.VirtualIncludes["basic_rt_raygen.hlsl"] = R"(
            [shader("raygeneration")]
            void RayGenMain()
            {
            }
        )";

        ShaderCompileInput missInput;
        missInput.Frequency = RHI::ERHIShaderFrequency::Miss;
        missInput.EntryPoint = "MissMain";
        missInput.Platform = RHI::GShaderPlatform;
        missInput.VirtualSourceFilePath = "basic_rt_miss.hlsl";
        missInput.Environment.VirtualIncludes["basic_rt_miss.hlsl"] = R"(
            [shader("miss")]
            void MissMain()
            {
            }
        )";

        ShaderCompileInput closestHitInput;
        closestHitInput.Frequency = RHI::ERHIShaderFrequency::ClosestHit;
        closestHitInput.EntryPoint = "ClosestHitMain";
        closestHitInput.Platform = RHI::GShaderPlatform;
        closestHitInput.VirtualSourceFilePath = "basic_rt_closesthit.hlsl";
        closestHitInput.Environment.VirtualIncludes["basic_rt_closesthit.hlsl"] = R"(
            struct Payload
            {
                float3 Color;
            };

            [shader("closesthit")]
            void ClosestHitMain(inout Payload payload)
            {
                payload.Color = float3(1.0, 0.0, 0.0);
            }
        )";

        ShaderCompilationOutput rayGenOutput = ShaderCompiler::Compile(rayGenInput);
        ShaderCompilationOutput missOutput = ShaderCompiler::Compile(missInput);
        ShaderCompilationOutput closestHitOutput = ShaderCompiler::Compile(closestHitInput);

        if (!rayGenOutput.Success || !missOutput.Success || !closestHitOutput.Success)
        {
            std::cout << "RHIRayTracingTest: ray tracing shader compile failed.\n";
            return;
        }

        RayGenShader = api->CreateRayGenShader(rayGenOutput.PackedBinaryData);
        MissShader = api->CreateMissShader(missOutput.PackedBinaryData);
        ClosestHitShader = api->CreateCloseHitShader(closestHitOutput.PackedBinaryData);

        if (!RayGenShader || !MissShader || !ClosestHitShader)
        {
            std::cout << "RHIRayTracingTest: ray tracing shader creation failed.\n";
            return;
        }

        RHI::RHIRayTracingPipelineStateDesc pipelineDesc{};
        pipelineDesc.RayGenTable.push_back(RayGenShader.get());
        pipelineDesc.MissTable.push_back(MissShader.get());
        pipelineDesc.HitGroupTable.push_back(ClosestHitShader.get());

        RayTracingPipelineState = api->CreateRayTracingsPipelineState(pipelineDesc);
        if (!RayTracingPipelineState)
        {
            std::cout << "RHIRayTracingTest: CreateRayTracingsPipelineState returned null (backend may be unfinished).\n";
        }
    }

private:
    int WindowWidth = 512;
    int WindowHeight = 512;
    int FrameWidth = 512;
    int FrameHeight = 512;

    bool bAccelerationBuilt = false;

    SlateCore::WindowSP Window;
    RHI::RHISwapchainSP Swapchain;

    RHI::RHIBufferSP VertexBuffer;
    RHI::RHIBufferSP IndexBuffer;

    RHI::RHIRayTracingGeometrySP BottomLevelAS;
    RHI::RHIRayTracingInstanceSP TopLevelAS;

    RHI::RHIRayGenShaderSP RayGenShader;
    RHI::RHIMissShaderSP MissShader;
    RHI::RHICloseHitShaderSP ClosestHitShader;
    RHI::RHIRayTracingPipelineStateSP RayTracingPipelineState;
};

REGISTER_RENDER_TEST("RHIRayTracingTest", RHIRayTracingTest);

} // namespace Test
