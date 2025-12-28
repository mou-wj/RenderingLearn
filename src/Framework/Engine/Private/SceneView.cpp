#include "SceneView.h"
#include <vector>
namespace Engine {
// Add a copy of a view. Returns index of the added view.
int SceneViewCollection::AddView(const SceneView& view) {
    views.emplace_back(view);
    return int(views.size() - 1);
}

// Add by move.
int SceneViewCollection::AddView(SceneView&& view) {
    views.emplace_back(std::move(view));
    return int(views.size() - 1);
}

// Remove view by index. Returns true if removed.
bool SceneViewCollection::RemoveView(int index) {
    if (index < 0 || index >= int(views.size())) return false;
    views.erase(views.begin() + index);
    return true;
}

// Accessors
SceneView* SceneViewCollection::GetView(int index) {
    if (index < 0 || index >= int(views.size())) return nullptr;
    return &views[index];
}


int SceneViewCollection::Size() const {
    return int(views.size());
}

void SceneViewCollection::RebuildAllDerivedMatrices() {
    for (auto &v : views) v.RebuildDerivedMatrices();
}

void SceneViewCollection::BuildAllFrustums() {
    for (auto &v : views) v.BuildFrustum();
}

std::vector<int> SceneViewCollection::FindViewsThatSeeBox(const Core::AABB& box) const {
    std::vector<int> out;
    for (size_t i = 0; i < views.size(); ++i) {
        if (views[i].IsBoxVisible(box)) out.push_back(int(i));
    }
    return out;
}
// SceneView.cpp
#include "../PublicHeader/SceneView.h"
#include <cmath>

static inline Matrix4x4 Mul(const Matrix4x4& A, const Matrix4x4& B) {
    Matrix4x4 R;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            float v = 0.0f;
            for (int k = 0; k < 4; ++k) {
                v += A.m[r*4 + k] * B.m[k*4 + c];
            }
            R.m[r*4 + c] = v;
        }
    }
    return R;
}

static inline float PlaneDot(const Plane& p, float x, float y, float z) {
    return p.a*x + p.b*y + p.c*z + p.d;
}

static inline void NormalizePlane(Plane& p) {
    float len = std::sqrt(p.a*p.a + p.b*p.b + p.c*p.c);
    if (len > 1e-6f) {
        p.a /= len; p.b /= len; p.c /= len; p.d /= len;
    }
}

SceneView::SceneView()
    : ViewMatrix(), ProjectionMatrix(), ViewProjectionMatrix(),
      CameraWorldPos{0,0,0}, CameraWorldDir{0,0,-1},
      NearClip(0.1f), FarClip(1000.0f), Viewport{0,0,1280,720},
      RenderTargetWidth(1280.0f), RenderTargetHeight(720.0f),
      FrameIndex(0), FrameTimeSeconds(0.0), Flags(VF_None) {
}

SceneView::~SceneView() {}

void SceneView::RebuildDerivedMatrices() {
    // In engine math, ViewProjection = Projection * View (column/row major depends on convention)
    ViewProjectionMatrix = Mul(ProjectionMatrix, ViewMatrix);
}

void SceneView::BuildFrustum() {
    // Extract frustum planes from ViewProjectionMatrix (clip-space plane extraction)
    const Matrix4x4& m = ViewProjectionMatrix;
    // Left plane: row4 + row1
    CachedFrustum.Planes[0].a = m.m[3] + m.m[0];
    CachedFrustum.Planes[0].b = m.m[7] + m.m[4];
    CachedFrustum.Planes[0].c = m.m[11] + m.m[8];
    CachedFrustum.Planes[0].d = m.m[15] + m.m[12];
    // Right: row4 - row1
    CachedFrustum.Planes[1].a = m.m[3] - m.m[0];
    CachedFrustum.Planes[1].b = m.m[7] - m.m[4];
    CachedFrustum.Planes[1].c = m.m[11] - m.m[8];
    CachedFrustum.Planes[1].d = m.m[15] - m.m[12];
    // Top: row4 - row2
    CachedFrustum.Planes[2].a = m.m[3] - m.m[1];
    CachedFrustum.Planes[2].b = m.m[7] - m.m[5];
    CachedFrustum.Planes[2].c = m.m[11] - m.m[9];
    CachedFrustum.Planes[2].d = m.m[15] - m.m[13];
    // Bottom: row4 + row2
    CachedFrustum.Planes[3].a = m.m[3] + m.m[1];
    CachedFrustum.Planes[3].b = m.m[7] + m.m[5];
    CachedFrustum.Planes[3].c = m.m[11] + m.m[9];
    CachedFrustum.Planes[3].d = m.m[15] + m.m[13];
    // Near: row4 + row3
    CachedFrustum.Planes[4].a = m.m[3] + m.m[2];
    CachedFrustum.Planes[4].b = m.m[7] + m.m[6];
    CachedFrustum.Planes[4].c = m.m[11] + m.m[10];
    CachedFrustum.Planes[4].d = m.m[15] + m.m[14];
    // Far: row4 - row3
    CachedFrustum.Planes[5].a = m.m[3] - m.m[2];
    CachedFrustum.Planes[5].b = m.m[7] - m.m[6];
    CachedFrustum.Planes[5].c = m.m[11] - m.m[10];
    CachedFrustum.Planes[5].d = m.m[15] - m.m[14];

    for (auto &p : CachedFrustum.Planes) NormalizePlane(p);
}

bool SceneView::IsBoxVisible(const Core::AABB& box) const {
    // For each plane, test the AABB against the plane (using the 'positive vertex' method)
    for (const Plane& p : CachedFrustum.Planes) {
        // choose vertex furthest in direction of plane normal
        float vx = (p.a >= 0.0f) ? box.Max.x : box.Min.x;
        float vy = (p.b >= 0.0f) ? box.Max.y : box.Min.y;
        float vz = (p.c >= 0.0f) ? box.Max.z : box.Min.z;
        if (PlaneDot(p, vx, vy, vz) < 0.0f) {
            // positive vertex outside
            return false;
        }
    }
    return true;
}

void SceneView::PackViewUniforms(ViewUniforms& out) const {
    out.ViewProj = ViewProjectionMatrix;
    out.CameraPos = CameraWorldPos;
    // InvViewProj left as identity / zero if not computed by caller
}
} // namespace Engine