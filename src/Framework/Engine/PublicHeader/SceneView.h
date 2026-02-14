#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <functional>
#include "BoxSphereBounds.h"
#include "EngineExport.h"
#include "SceneInterface.h"
#include "Viewport.h"
namespace Engine {

    struct Matrix4x4 { float m[16]; };
    struct Vector3 { float x, y, z; };

    struct ViewRect { int x; int y; int width; int height; };


    // Plane: ax + by + cz + d >= 0 is inside
    struct Plane { float a, b, c, d; };

    struct Frustum { std::array<Plane, 6> Planes; };


    struct ViewUniforms {
        Matrix4x4 ViewProj;
        Matrix4x4 InvViewProj; // optional
        Vector3 CameraPos;
        float Padding0;
    };

    enum ViewFlags : uint32_t {
        VF_None = 0,
        VF_EnableShadows = 1 << 0,
        VF_EnablePostFX = 1 << 1
    };

    class ENGINE_API SceneView {
    public:
        SceneView();
        ~SceneView();

        // Core transforms
        Matrix4x4 ViewMatrix;            // world -> view
        Matrix4x4 ProjectionMatrix;      // view -> clip
        Matrix4x4 ViewProjectionMatrix;  // precomputed Projection * View

        // Camera
        Vector3 CameraWorldPos;
        Vector3 CameraWorldDir;

        // Clip
        float NearClip;
        float FarClip;

        // Viewport / target
        ViewRect Viewport;
        float RenderTargetWidth;
        float RenderTargetHeight;

        // Frame/time
        uint64_t FrameIndex;
        double FrameTimeSeconds;

        // Flags
        uint32_t Flags;

        // Cached derived data
        Frustum CachedFrustum; // optional cached frustum

        // Utilities (thread-safe read-only after construction on RenderThread)
        void RebuildDerivedMatrices(); // recompute ViewProjectionMatrix
        void BuildFrustum();           // extract frustum from ViewProjectionMatrix
        bool IsBoxVisible(const Core::AABB& box) const; // frustum-AABB test
        void PackViewUniforms(ViewUniforms& out) const; // fill ViewUniforms
    };

    class ENGINE_API SceneViewCollection {
    public:
        SceneViewCollection() = default;
        ~SceneViewCollection() = default;

        // Add a copy of a view. Returns index of the added view.
        int AddView(const SceneView& view);

        // Add by move.
        int AddView(SceneView&& view);

        // Remove view by index. Returns true if removed.
        bool RemoveView(int index);

        // Accessors
        SceneView* GetView(int index);

        int Size() const;

        // Iterate views with a callable: void(SceneView&)
        template<typename Fn>
        void ForEachView(Fn&& fn) {
            for (auto& v : views) fn(v);
        }

        // Utilities
        void RebuildAllDerivedMatrices();
        void BuildAllFrustums();

        // Return indices of views that consider the box visible.
        std::vector<int> FindViewsThatSeeBox(const Core::AABB& box) const;
        SceneInterface* Scene;
        RenderTarget* RenderTarget;
    private:
        std::vector<SceneView> views;
    };
}