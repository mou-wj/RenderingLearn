#include "SceneView.h"
#include "SceneInterface.h"
#include "SceneViewport.h"
#include <cassert>
#include <cstring>

namespace Engine {

    /*
    ===============================================================================
        SceneView 实现
    ===============================================================================
    */

    SceneView::SceneView()
        : NearClip(0.1f)
        , FarClip(1000.0f)
        , RenderTargetWidth(0.0f)
        , RenderTargetHeight(0.0f)
        , FrameIndex(0)
        , FrameTimeSeconds(0.0)
        , Flags(VF_None)
    {
        Viewport = { 0, 0, 0, 0 };
        // 初始化矩阵为单位阵
        ViewMatrix = Core::Float4x4::Identity();
        ProjectionMatrix = Core::Float4x4::Identity();
        ViewProjectionMatrix = Core::Float4x4::Identity();
        InvViewProjectionMatrix = Core::Float4x4::Identity();
    }

    SceneView::~SceneView() = default;

    void SceneView::RebuildDerivedMatrices()
    {
        // 计算 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
        // 针对你的行主序 (Row-Major) 矩阵模板实现标准乘法
        for (size_t r = 0; r < 4; ++r)
        {
            for (size_t c = 0; c < 4; ++c)
            {
                float Sum = 0.0f;
                for (size_t k = 0; k < 4; ++k)
                {
                    // 寻址公式: row * Cols + col
                    Sum += ProjectionMatrix(r, k) * ViewMatrix(k, c);
                }
                ViewProjectionMatrix(r, c) = Sum;
            }
        }

        // 工业级框架预留：在此处计算 InvViewProjectionMatrix 用于延迟管线或遮挡剔除
        // 为了保持核心骨架精简，此处先省略高阶矩阵求逆算法
    }

    void SceneView::BuildFrustum()
    {
        // 从行主序的 View-Projection 矩阵中提取 6 个视锥体裁剪面 (Gribb-Hartmann 方法)
        // 寻址格式: M(row, col)
        const auto& M = ViewProjectionMatrix;

        // 左裁剪面 (Left Plane)
        CachedFrustum.Planes[0] = { M(3,0) + M(0,0), M(3,1) + M(0,1), M(3,2) + M(0,2), M(3,3) + M(0,3) };
        // 右裁剪面 (Right Plane)
        CachedFrustum.Planes[1] = { M(3,0) - M(0,0), M(3,1) - M(0,1), M(3,2) - M(0,2), M(3,3) - M(0,3) };
        // 下裁剪面 (Bottom Plane)
        CachedFrustum.Planes[2] = { M(3,0) + M(1,0), M(3,1) + M(1,1), M(3,2) + M(1,2), M(3,3) + M(1,3) };
        // 上裁剪面 (Top Plane)
        CachedFrustum.Planes[3] = { M(3,0) - M(1,0), M(3,1) - M(1,1), M(3,2) - M(1,2), M(3,3) - M(1,3) };
        // 近裁剪面 (Near Plane)
        CachedFrustum.Planes[4] = { M(3,0) + M(2,0), M(3,1) + M(2,1), M(3,2) + M(2,2), M(3,3) + M(2,3) };
        // 远裁剪面 (Far Plane)
        CachedFrustum.Planes[5] = { M(3,0) - M(2,0), M(3,1) - M(2,1), M(3,2) - M(2,2), M(3,3) - M(2,3) };

        // 归一化所有裁剪面方程，确保距离测试准确
        for (auto& Plane : CachedFrustum.Planes)
        {
            float Length = std::sqrt(Plane.a * Plane.a + Plane.b * Plane.b + Plane.c * Plane.c);
            if (Length > 0.0f)
            {
                Plane.a /= Length;
                Plane.b /= Length;
                Plane.c /= Length;
                Plane.d /= Length;
            }
        }
    }

    bool SceneView::IsBoxVisible(const Core::AABB& Box) const
    {
        // 工业级标准：高效 Frustum-AABB 检测 (利用正负顶点测试，避免遍历 8 个顶点)
        // 传入的 Box 内含 Min 和 Max 两个三维向量
        for (const auto& Plane : CachedFrustum.Planes)
        {
            // 寻找在当前平面法线方向上，AABB 距离平面最近的“最远点 (Positive Vertex)”
            float PV_x = (Plane.a > 0.0f) ? Box.Max.x : Box.Min.x;
            float PV_y = (Plane.b > 0.0f) ? Box.Max.y : Box.Min.y;
            float PV_z = (Plane.c > 0.0f) ? Box.Max.z : Box.Min.z;

            // 如果连这个最容易留在平面内部的点都在平面外侧（即算出来的带符号距离 < 0），说明被彻底裁剪
            if (Plane.a * PV_x + Plane.b * PV_y + Plane.c * PV_z + Plane.d < 0.0f)
            {
                return false; // 隐藏，不可见
            }
        }
        return true; // 可见
    }

    void SceneView::PackViewUniforms(ViewUniforms& OutUniforms) const
    {
        // 1. 复制矩阵
        OutUniforms.ViewProj = ViewProjectionMatrix;
        OutUniforms.InvViewProj = InvViewProjectionMatrix;

        // 2. 复制相机位置，并对齐打入 Float4 内存中
        OutUniforms.CameraPos.x = CameraWorldPos.x;
        OutUniforms.CameraPos.y = CameraWorldPos.y;
        OutUniforms.CameraPos.z = CameraWorldPos.z;
        OutUniforms.CameraPos.w = 1.0f; // 常规齐次坐标预留
    }
    static Core::Float3 UnprojectFromNDC(
        const Core::Float4x4& invVP,
        float x,
        float y,
        float z)
    {
        Core::Float4 p(x, y, z, 1.0f);

        Core::Float4 world = invVP * p.Data;

        if (std::abs(world.w) > 1e-6f)
        {
            world.x /= world.w;
            world.y /= world.w;
            world.z /= world.w;
        }

        return Core::Float3(world.x, world.y, world.z);
    }
    std::array<Core::Float3, 8> SceneView::GetFrustumCornersWS(float nearDepth, float farDepth) const {
        std::array<Core::Float3, 8> corners;

        //----------------------------------------
        // 1. Full frustum corners in world space
        //----------------------------------------
        std::array<Core::Float3, 8> fullCorners;

        // Near plane (z=0)
        fullCorners[0] = UnprojectFromNDC(InvViewProjectionMatrix, -1.f, -1.f, 0.f);
        fullCorners[1] = UnprojectFromNDC(InvViewProjectionMatrix, 1.f, -1.f, 0.f);
        fullCorners[2] = UnprojectFromNDC(InvViewProjectionMatrix, 1.f, 1.f, 0.f);
        fullCorners[3] = UnprojectFromNDC(InvViewProjectionMatrix, -1.f, 1.f, 0.f);

        // Far plane (z=1)
        fullCorners[4] = UnprojectFromNDC(InvViewProjectionMatrix, -1.f, -1.f, 1.f);
        fullCorners[5] = UnprojectFromNDC(InvViewProjectionMatrix, 1.f, -1.f, 1.f);
        fullCorners[6] = UnprojectFromNDC(InvViewProjectionMatrix, 1.f, 1.f, 1.f);
        fullCorners[7] = UnprojectFromNDC(InvViewProjectionMatrix, -1.f, 1.f, 1.f);

        //----------------------------------------
        // 2. Convert depth -> ratio
        //----------------------------------------
        float nearRatio =
            (nearDepth - NearClip) /
            (FarClip - NearClip);

        float farRatio =
            (farDepth - NearClip) /
            (FarClip - NearClip);

        nearRatio = std::clamp(nearRatio, 0.0f, 1.0f);
        farRatio = std::clamp(farRatio, 0.0f, 1.0f);

        //----------------------------------------
        // 3. Interpolate cascade corners
        //----------------------------------------
        for (uint32_t i = 0; i < 4; i++)
        {
            Core::Float3 nearCorner = fullCorners[i];
            Core::Float3 farCorner = fullCorners[i + 4];

            Core::Float3 ray = farCorner - nearCorner;

            corners[i] =
                nearCorner + ray * nearRatio;

            corners[i + 4] =
                nearCorner + ray * farRatio;
        }

        return corners;
    }


    int SceneViewFamily::AddView(const SceneView& View)
    {
        Views.push_back(View);
        return static_cast<int>(Views.size()) - 1;
    }

    int SceneViewFamily::AddView(SceneView&& View)
    {
        Views.push_back(std::move(View));
        return static_cast<int>(Views.size()) - 1;
    }

    bool SceneViewFamily::RemoveView(int Index)
    {
        if (Index >= 0 && Index < static_cast<int>(Views.size()))
        {
            Views.erase(Views.begin() + Index);
            return true;
        }
        return false;
    }
    void SceneViewFamily::ClearViews() 
    {
        Views.clear();
    }

    SceneView* SceneViewFamily::GetView(int Index)
    {
        if (Index >= 0 && Index < static_cast<int>(Views.size()))
        {
            return &Views[Index];
        }
        return nullptr;
    }

    void SceneViewFamily::RebuildAllDerivedMatrices()
    {
        for (auto& View : Views)
        {
            View.RebuildDerivedMatrices();
        }
    }

    void SceneViewFamily::BuildAllFrustums()
    {
        for (auto& View : Views)
        {
            View.BuildFrustum();
        }
    }

    std::vector<int> SceneViewFamily::FindViewsThatSeeBox(const Core::AABB& Box) const
    {
        std::vector<int> VisibleViewIndices;
        VisibleViewIndices.reserve(Views.size());

        for (size_t i = 0; i < Views.size(); ++i)
        {
            if (Views[i].IsBoxVisible(Box))
            {
                VisibleViewIndices.push_back(static_cast<int>(i));
            }
        }
        return VisibleViewIndices;
    }

} // namespace Engine