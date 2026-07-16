// Scene.h
#pragma once
#include "SceneInterface.h"
#include "RenderResource.h"
#include "Flags.h"
#include "ShadowMapAllocator.h"
#include "BoxSphereBounds.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
namespace Engine
{
    class PrimitiveComponent;
    class PrimitiveSceneProxy;

    class LightComponent;
    class LightSceneProxy;

    class SceneView;
}
namespace Renderer {
    //====================================================
    // Primitive Scene Info
    //====================================================

    class PrimitiveSceneInfo
    {
    public:

        explicit PrimitiveSceneInfo(
            std::unique_ptr<
            Engine::PrimitiveSceneProxy>
            InProxy);

        Engine::PrimitiveSceneProxy*
            GetProxy() const
        {
            return Proxy.get();
        }

    public:

        bool bVisible = true;
        bool bCastShadow = true;
        bool bDynamic = true;

        uint32_t VisibilityFrame = 0;

    private:

        std::unique_ptr<
            Engine::PrimitiveSceneProxy>
            Proxy;
    };

    //====================================================
    // Light Scene Info
    //====================================================

    class LightSceneInfo
    {
    public:

        explicit LightSceneInfo(
            std::unique_ptr<
            Engine::LightSceneProxy>
            InProxy);

        Engine::LightSceneProxy*
            GetProxy() const
        {
            return Proxy.get();
        }

    public:

        bool bVisible = true;

    private:

        std::unique_ptr<
            Engine::LightSceneProxy>
            Proxy;
    };

    struct LightShadowInfo
    {
        ShadowAllocation Allocation;

        std::vector<Core::Float4x4> ShadowMatrices;

        bool bDirty = true;
    };

    //====================================================
    // Scene Commands
    //====================================================

    enum class ESceneCommandType
    {
        AddPrimitive,
        RemovePrimitive,

        AddLight,
        RemoveLight
    };

    struct SceneCommand
    {
        ESceneCommandType Type;

        union
        {
            Engine::PrimitiveComponent*
                PrimitiveComponent;

            Engine::LightComponent*
                LightComponent;
        };

        
        std::unique_ptr<Engine::PrimitiveSceneProxy>
            PrimitiveProxy;

        
        std::unique_ptr<Engine::LightSceneProxy>
            LightProxy;
    };


    enum class ESceneGPUResourceDirty : uint32_t
    {
        None = 0,

        DirectionalLight = 1 << 0,
        PointLight = 1 << 1,
        SpotLight = 1 << 2,
        RectLight = 1 << 3,
        SkyLight = 1 << 4,
        Primitive = 1 << 5,

        AllLight =
        DirectionalLight |
        PointLight |
        SpotLight |
        RectLight |
        SkyLight,

        All = 0xFFFFFFFF
    };
    ENUM_CLASS_FLAGS(ESceneGPUResourceDirty, ESceneGPUResourceDirtys);

    struct SceneGPULightResourceInfo {
        uint32_t PointLightCount = 0;
        uint32_t SpotLightCount = 0;
        uint32_t DirectionalLightCount = 0;
        uint32_t RectLightCount = 0;
        RenderCore::RenderBufferSP PointLightBuffer = nullptr;
        RenderCore::RenderBufferSP SpotLightBuffer = nullptr;
        RenderCore::RenderBufferSP DirectionalLightBuffer = nullptr;
        RenderCore::RenderBufferSP RectLightBuffer = nullptr;
        RenderCore::RenderTexture* IBLDiffuseTexture = nullptr;
        RenderCore::RenderTexture* IBLSpecularTexture = nullptr;
    };
    struct SceneShadowResourceInfo
    {
        std::unordered_map<Engine::LightSceneProxy*, LightShadowInfo> LightShadowInfos;
        RenderCore::RenderBufferSP LightShadowInfoBuffer = nullptr;
        RenderCore::RenderBufferSP AtlasAccessInfoBuffer = nullptr;
        RenderCore::RenderBufferSP DirectionalLightShadowViewInfoBuffer = nullptr;
        RenderCore::RenderBufferSP SplitBuffer = nullptr;   
    };
    struct SceneGPUPrimitiveResourceInfo
    {
        uint32_t PrimitiveCount = 0;
        RenderCore::RenderBufferSP PrimitiveInstanceDataBuffer = nullptr;
        RenderCore::RenderBufferSP PrimitiveBoundsBuffer = nullptr;
        RenderCore::RenderBufferSP VisibilityFlagsBuffer = nullptr;
    };
    struct SceneGPUResourceInfo {
        SceneGPULightResourceInfo LightResourceInfo;
        SceneShadowResourceInfo ShadowResourceInfo;
        SceneGPUPrimitiveResourceInfo PrimitiveResourceInfo;
        ESceneGPUResourceDirtys DirtyFlags = ESceneGPUResourceDirty::None;
    };

    class SceneAccelerationStructure
    {
    public:
        void Build(const std::unordered_map<Engine::PrimitiveComponent*, std::unique_ptr<PrimitiveSceneInfo>>& PrimitiveInfos);
        void Query(const Engine::SceneView& View, std::vector<Engine::PrimitiveSceneProxy*>& OutPrimitives) const;
        void Clear();
        bool IsValid() const { return !Nodes.empty(); }

    private:
        struct Node
        {
            Core::BoxSphereBounds Bounds;
            int Start = 0;
            int Count = 0;
            int LeftChild = -1;
            int RightChild = -1;
            bool bLeaf = false;
        };

        std::vector<Node> Nodes;
        std::vector<Engine::PrimitiveSceneProxy*> Primitives;

        int BuildNode(int Start, int Count);
        int Partition(int Start, int Count, int Axis);
    };

    /*
    ===============================================================================
        Scene (SceneInterface ����ȫ�幤ҵ��ʵ��)
    ===============================================================================
    */
    class RENDERER_API Scene : public Engine::SceneInterface {
    public:
        Scene();
        ~Scene() override;

        // -------------------------------------------------------------------------
        // ��Ϸ�߳����ע��ӿ� (GameThread Only)
        // -------------------------------------------------------------------------
        void AddPrimitive(Engine::PrimitiveComponent* Component) override;
        void RemovePrimitive(Engine::PrimitiveComponent* Component) override;
        void AddLight(Engine::LightComponent* Component) override;
        void RemoveLight(Engine::LightComponent* Component) override;
        // -------------------------------------------------------------------------
        // ��Ⱦ�߳�ר����ѯ�ӿ� (RenderThread Only)
        // -------------------------------------------------------------------------
        void FlushPendingUpdates() override;
        void NotifyComponentChanged(Engine::SceneComponent* Component) override;

        std::vector<Engine::PrimitiveSceneProxy*> GatherVisiblePrimitivesCPU(const Engine::SceneView& View) const override;
        std::vector<Engine::PrimitiveSceneProxy*> GatherVisiblePrimitivesGPU(const Engine::SceneView& View) const override;
        std::vector<Engine::PrimitiveSceneProxy*> GatherVisiblePrimitives(const Engine::SceneView& View, Engine::SceneInterface::ECullingMethod Method) const override;

        void ForEachPrimitive(std::function<void(Engine::PrimitiveSceneProxy*)> Visitor);

        void ForEachLight(std::function<void(Engine::LightSceneProxy*)> Visitor);

        const SceneGPUResourceInfo& GetGPUResourceInfo() const;
        ShadowMapAllocator& GetShadowMapAllocator();
        const Core::BoxSphereBounds& GetSceneBounds() const;
        LightShadowInfo& GetLightShadowInfo(Engine::LightSceneProxy* Light);
    private:
		friend class SceneRenderer;
        void UpdateGPUResourceIfNeeded();
        void UpdatePrimitiveGPUResource();

        //=========================================
        // Render Thread Storage
        //=========================================

        std::unordered_map<
            Engine::PrimitiveComponent*,
            std::unique_ptr<
            PrimitiveSceneInfo>>
            PrimitiveInfos;

        std::vector<
            Engine::PrimitiveSceneProxy*>
            PrimitiveGPUProxyOrder;

        SceneAccelerationStructure AccelerationStructure;

        std::unordered_map<
            Engine::LightComponent*,
            std::unique_ptr<
            LightSceneInfo>>
            LightInfos;

        std::unordered_map<
            Engine::LightSceneProxy*,
            uint32_t>
            LightIndexs;

        //=========================================
        // GT -> RT command queue
        //=========================================

        std::vector<
            SceneCommand>
            PendingCommands;

        std::mutex
            PendingCommandMutex;
        SceneGPUResourceInfo GPUResourceInfo;
        ShadowMapAllocator ShadowMapAllocator;
        Core::BoxSphereBounds SceneBounds;
    };
} // namespace Renderer