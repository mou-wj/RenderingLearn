// SceneInterface.h
// Interface-only scene abstraction for Renderer-side access.
#pragma once
#include "EngineExport.h"
#include <cstdint>
#include <functional>
#include <vector>

namespace Engine {
    // ǰ��������ֻ������Ⱦ�߳̿ɼ��Ĵ�������ͼ�ṹ
    class PrimitiveComponent;
    class PrimitiveSceneProxy; // �����ع���� Proxy ����
    class LightComponent;
    class SceneComponent;
    class SceneView;          // ������Ⱦ�ӿ���ϵ

    using FrameIndex = uint64_t;

    enum class EFeatureLevel : uint8_t {
        Low = 0,
        Medium = 1,
        High = 2
    };

    /*
    ===============================================================================
        SceneInterface
        ������ࣺ��������Ⱦ����Renderer������Ⱦ�̴߳�һ������ʵ������ȡ���ݵ�������Լ
    ===============================================================================
    */
    class ENGINE_API SceneInterface {
    public:
        virtual ~SceneInterface() = default;

        enum class ECullingMethod : uint8_t {
            CPU = 0,
            GPU = 1
        };

        // -------------------------------------------------------------------------
        // ��Ϸ�߳����ע��ӿ� (ʵ�����ڲ����뽫���װΪ����Ͷ�ݵ� Pending ����)
        // -------------------------------------------------------------------------
        virtual void AddPrimitive(PrimitiveComponent* Component) = 0;
        virtual void RemovePrimitive(PrimitiveComponent* Component) = 0;
        virtual void AddLight(LightComponent* Component) = 0;
        virtual void RemoveLight(LightComponent* Component) = 0;
        virtual void FlushPendingUpdates() = 0;
        virtual void NotifyComponentChanged(SceneComponent* Component) = 0;

        // -------------------------------------------------------------------------
        // 剔除查询接口
        // -------------------------------------------------------------------------
        virtual std::vector<PrimitiveSceneProxy*> GatherVisiblePrimitivesCPU(const SceneView& View) const = 0;
        virtual std::vector<PrimitiveSceneProxy*> GatherVisiblePrimitivesGPU(const SceneView& View) const = 0;
        virtual std::vector<PrimitiveSceneProxy*> GatherVisiblePrimitives(const SceneView& View, ECullingMethod Method) const = 0;
    };
} // namespace Engine