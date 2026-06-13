#include "Shape.h"
#include "StaticMeshResources.h"
#include "BoxSphereBounds.h"
#include <memory>
#include <vector>
#include <cmath>

namespace Engine {

namespace {

static void FillVertexBuffer(VertexBuffer& Buffer, std::vector<float>&& Data, uint32_t NumComponents)
{
    Buffer.Vertices = std::move(Data);
    Buffer.NumComponents = NumComponents;
    Buffer.Valid = !Buffer.Vertices.empty();
}

static void FillLODFromData(
    LODResource& OutLOD,
    std::vector<float>&& Positions,
    std::vector<float>&& UVs,
    std::vector<float>&& Normals,
    std::vector<float>&& Tangents,
    std::vector<uint32_t>&& Indices)
{
    FillVertexBuffer(OutLOD.VertexBuffers.PositionBuffer, std::move(Positions), 3);
    FillVertexBuffer(OutLOD.VertexBuffers.UVBuffer, std::move(UVs), 2);
    FillVertexBuffer(OutLOD.VertexBuffers.NormalBuffer, std::move(Normals), 3);
    FillVertexBuffer(OutLOD.VertexBuffers.TangentBuffer, std::move(Tangents), 4);
    OutLOD.IndexBuffer.Indices = std::move(Indices);

    SectionInfo Section;
    Section.FirstIndex = 0;
    Section.NumIndices = static_cast<uint32_t>(OutLOD.IndexBuffer.Indices.size());
    Section.MaterialIndex = 0;
    OutLOD.Sections.push_back(Section);
}

static void ComputeBoundsFromPositions(Core::BoxSphereBounds& OutBounds, const std::vector<float>& Positions)
{
    Core::AABB Bounds;
    Bounds.SetEmpty();

    const size_t VertexCount = Positions.size() / 3;
    for (size_t Index = 0; Index < VertexCount; ++Index) {
        Core::Float3 Position(
            Positions[Index * 3 + 0],
            Positions[Index * 3 + 1],
            Positions[Index * 3 + 2]);
        Bounds.ExpandBy(Position);
    }

    OutBounds.UpdateFromAABB(Bounds);
}

static std::shared_ptr<StaticMesh> CreateStaticMeshFromData(
    std::vector<float>&& Positions,
    std::vector<float>&& UVs,
    std::vector<float>&& Normals,
    std::vector<float>&& Tangents,
    std::vector<uint32_t>&& Indices)
{
    auto RenderData = std::make_shared<StaticMeshRenderData>();
    LODResource LOD;
    FillLODFromData(LOD, std::move(Positions), std::move(UVs), std::move(Normals), std::move(Tangents), std::move(Indices));
    LOD.InitializeResources();
    ComputeBoundsFromPositions(RenderData->Bounds, LOD.VertexBuffers.PositionBuffer.Vertices);
    RenderData->AddLOD(std::move(LOD));
    return std::make_shared<StaticMesh>(RenderData);
}

static void BuildCubeData(
    std::vector<float>& OutPositions,
    std::vector<float>& OutUVs,
    std::vector<float>& OutNormals,
    std::vector<float>& OutTangents,
    std::vector<uint32_t>& OutIndices)
{
    const float FacePositions[] = {
        // +X face
         1, -1, -1,  1, -1,  1,  1,  1,  1,  1,  1, -1,
        // -X face
        -1, -1,  1, -1, -1, -1, -1,  1, -1, -1,  1,  1,
        // +Y face
        -1,  1,  1,  1,  1,  1,  1,  1, -1, -1,  1, -1,
        // -Y face
        -1, -1, -1,  1, -1, -1,  1, -1,  1, -1, -1,  1,
        // +Z face
        -1, -1,  1,  1, -1,  1,  1,  1,  1, -1,  1,  1,
        // -Z face
         1, -1, -1, -1, -1, -1, -1,  1, -1,  1,  1, -1,
    };

    const float FaceNormals[] = {
        // +X
         1, 0, 0,  1, 0, 0,  1, 0, 0,  1, 0, 0,
        // -X
        -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0,
        // +Y
         0, 1, 0,  0, 1, 0,  0, 1, 0,  0, 1, 0,
        // -Y
         0,-1, 0,  0,-1, 0,  0,-1, 0,  0,-1, 0,
        // +Z
         0, 0, 1,  0, 0, 1,  0, 0, 1,  0, 0, 1,
        // -Z
         0, 0,-1,  0, 0,-1,  0, 0,-1,  0, 0,-1,
    };

    const float FaceUVs[] = {
        0, 0,  1, 0,  1, 1,  0, 1,
        0, 0,  1, 0,  1, 1,  0, 1,
        0, 0,  1, 0,  1, 1,  0, 1,
        0, 0,  1, 0,  1, 1,  0, 1,
        0, 0,  1, 0,  1, 1,  0, 1,
        0, 0,  1, 0,  1, 1,  0, 1,
    };

    const float FaceTangents[] = {
        // +X
         0, 0, 1, 0,  0, 0, 1, 0,  0, 0, 1, 0,  0, 0, 1, 0,
        // -X
         0, 0,-1, 0,  0, 0,-1, 0,  0, 0,-1, 0,  0, 0,-1, 0,
        // +Y
         1, 0, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0,
        // -Y
         1, 0, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0,
        // +Z
         1, 0, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0,
        // -Z
        -1, 0, 0, 0, -1, 0, 0, 0, -1, 0, 0, 0, -1, 0, 0, 0,
    };

    OutPositions.assign(std::begin(FacePositions), std::end(FacePositions));
    OutNormals.assign(std::begin(FaceNormals), std::end(FaceNormals));
    OutUVs.assign(std::begin(FaceUVs), std::end(FaceUVs));
    OutTangents.assign(std::begin(FaceTangents), std::end(FaceTangents));

    static const uint32_t FaceIndices[] = {
        0,  1,  2,  0,  2,  3,
        4,  5,  6,  4,  6,  7,
        8,  9, 10,  8, 10, 11,
       12, 13, 14, 12, 14, 15,
       16, 17, 18, 16, 18, 19,
       20, 21, 22, 20, 22, 23,
    };
    OutIndices.assign(std::begin(FaceIndices), std::end(FaceIndices));
}

static void BuildPlaneData(
    std::vector<float>& OutPositions,
    std::vector<float>& OutUVs,
    std::vector<float>& OutNormals,
    std::vector<float>& OutTangents,
    std::vector<uint32_t>& OutIndices)
{
    OutPositions = {
        -1, 0, -1,
         1, 0, -1,
         1, 0,  1,
        -1, 0,  1,
    };

    OutNormals = {
         0, 1, 0,
         0, 1, 0,
         0, 1, 0,
         0, 1, 0,
    };

    OutUVs = {
        0, 0,
        1, 0,
        1, 1,
        0, 1,
    };

    OutTangents = {
        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 0, 0, 0,
        1, 0, 0, 0,
    };

    OutIndices = {0, 1, 2, 0, 2, 3};
}

static void BuildSphereData(
    std::vector<float>& OutPositions,
    std::vector<float>& OutUVs,
    std::vector<float>& OutNormals,
    std::vector<float>& OutTangents,
    std::vector<uint32_t>& OutIndices,
    int LatSegments = 16,
    int LonSegments = 16)
{
    const float Pi = 3.14159265359f;
    const int LatCount = LatSegments + 1;
    const int LonCount = LonSegments + 1;

    OutPositions.clear();
    OutNormals.clear();
    OutUVs.clear();
    OutTangents.clear();
    OutIndices.clear();

    OutPositions.reserve(LatCount * LonCount * 3);
    OutNormals.reserve(LatCount * LonCount * 3);
    OutUVs.reserve(LatCount * LonCount * 2);
    OutTangents.reserve(LatCount * LonCount * 4);

    for (int Lat = 0; Lat < LatCount; ++Lat) {
        const float Theta = Lat * Pi / LatSegments;
        const float SinTheta = std::sin(Theta);
        const float CosTheta = std::cos(Theta);

        for (int Lon = 0; Lon < LonCount; ++Lon) {
            const float Phi = Lon * 2.0f * Pi / LonSegments;
            const float SinPhi = std::sin(Phi);
            const float CosPhi = std::cos(Phi);

            const float X = SinTheta * CosPhi;
            const float Y = CosTheta;
            const float Z = SinTheta * SinPhi;

            OutPositions.push_back(X);
            OutPositions.push_back(Y);
            OutPositions.push_back(Z);

            OutNormals.push_back(X);
            OutNormals.push_back(Y);
            OutNormals.push_back(Z);

            OutUVs.push_back(static_cast<float>(Lon) / LonSegments);
            OutUVs.push_back(1.0f - static_cast<float>(Lat) / LatSegments);

            OutTangents.push_back(-SinPhi);
            OutTangents.push_back(0.0f);
            OutTangents.push_back(CosPhi);
            OutTangents.push_back(0.0f);
        }
    }

    for (int Lat = 0; Lat < LatSegments; ++Lat) {
        for (int Lon = 0; Lon < LonSegments; ++Lon) {
            const uint32_t First = static_cast<uint32_t>(Lat * LonCount + Lon);
            const uint32_t Second = static_cast<uint32_t>((Lat + 1) * LonCount + Lon);

            OutIndices.push_back(First);
            OutIndices.push_back(Second);
            OutIndices.push_back(Second + 1);

            OutIndices.push_back(First);
            OutIndices.push_back(Second + 1);
            OutIndices.push_back(First + 1);
        }
    }
}

static void BuildCylinderData(
    std::vector<float>& OutPositions,
    std::vector<float>& OutUVs,
    std::vector<float>& OutNormals,
    std::vector<float>& OutTangents,
    std::vector<uint32_t>& OutIndices,
    int Segments = 16)
{
    const float Pi = 3.14159265359f;
    const float HalfHeight = 1.0f;
    const int RingSize = Segments + 1;

    OutPositions.clear();
    OutUVs.clear();
    OutNormals.clear();
    OutTangents.clear();
    OutIndices.clear();

    OutPositions.reserve((RingSize * 2 + RingSize * 2 + 2) * 3);
    OutUVs.reserve((RingSize * 2 + RingSize * 2 + 2) * 2);
    OutNormals.reserve((RingSize * 2 + RingSize * 2 + 2) * 3);
    OutTangents.reserve((RingSize * 2 + RingSize * 2 + 2) * 4);

    // Side vertices
    for (int Slice = 0; Slice < RingSize; ++Slice) {
        const float Theta = Slice * 2.0f * Pi / Segments;
        const float CosTheta = std::cos(Theta);
        const float SinTheta = std::sin(Theta);

        OutPositions.push_back(CosTheta);
        OutPositions.push_back(HalfHeight);
        OutPositions.push_back(SinTheta);
        OutNormals.push_back(CosTheta);
        OutNormals.push_back(0.0f);
        OutNormals.push_back(SinTheta);
        OutUVs.push_back(static_cast<float>(Slice) / Segments);
        OutUVs.push_back(0.0f);
        OutTangents.push_back(-SinTheta);
        OutTangents.push_back(0.0f);
        OutTangents.push_back(CosTheta);
        OutTangents.push_back(0.0f);
    }

    for (int Slice = 0; Slice < RingSize; ++Slice) {
        const float Theta = Slice * 2.0f * Pi / Segments;
        const float CosTheta = std::cos(Theta);
        const float SinTheta = std::sin(Theta);

        OutPositions.push_back(CosTheta);
        OutPositions.push_back(-HalfHeight);
        OutPositions.push_back(SinTheta);
        OutNormals.push_back(CosTheta);
        OutNormals.push_back(0.0f);
        OutNormals.push_back(SinTheta);
        OutUVs.push_back(static_cast<float>(Slice) / Segments);
        OutUVs.push_back(1.0f);
        OutTangents.push_back(-SinTheta);
        OutTangents.push_back(0.0f);
        OutTangents.push_back(CosTheta);
        OutTangents.push_back(0.0f);
    }

    const uint32_t TopCenterIndex = static_cast<uint32_t>(OutPositions.size() / 3);
    OutPositions.push_back(0.0f);
    OutPositions.push_back(HalfHeight);
    OutPositions.push_back(0.0f);
    OutNormals.push_back(0.0f);
    OutNormals.push_back(1.0f);
    OutNormals.push_back(0.0f);
    OutUVs.push_back(0.5f);
    OutUVs.push_back(0.5f);
    OutTangents.push_back(1.0f);
    OutTangents.push_back(0.0f);
    OutTangents.push_back(0.0f);
    OutTangents.push_back(0.0f);

    const uint32_t TopRingIndex = static_cast<uint32_t>(OutPositions.size() / 3);
    for (int Slice = 0; Slice < RingSize; ++Slice) {
        const float Theta = Slice * 2.0f * Pi / Segments;
        const float CosTheta = std::cos(Theta);
        const float SinTheta = std::sin(Theta);

        OutPositions.push_back(CosTheta);
        OutPositions.push_back(HalfHeight);
        OutPositions.push_back(SinTheta);
        OutNormals.push_back(0.0f);
        OutNormals.push_back(1.0f);
        OutNormals.push_back(0.0f);
        OutUVs.push_back((CosTheta + 1.0f) * 0.5f);
        OutUVs.push_back((SinTheta + 1.0f) * 0.5f);
        OutTangents.push_back(-SinTheta);
        OutTangents.push_back(0.0f);
        OutTangents.push_back(CosTheta);
        OutTangents.push_back(0.0f);
    }

    const uint32_t BottomCenterIndex = static_cast<uint32_t>(OutPositions.size() / 3);
    OutPositions.push_back(0.0f);
    OutPositions.push_back(-HalfHeight);
    OutPositions.push_back(0.0f);
    OutNormals.push_back(0.0f);
    OutNormals.push_back(-1.0f);
    OutNormals.push_back(0.0f);
    OutUVs.push_back(0.5f);
    OutUVs.push_back(0.5f);
    OutTangents.push_back(1.0f);
    OutTangents.push_back(0.0f);
    OutTangents.push_back(0.0f);
    OutTangents.push_back(0.0f);

    const uint32_t BottomRingIndex = static_cast<uint32_t>(OutPositions.size() / 3);
    for (int Slice = 0; Slice < RingSize; ++Slice) {
        const float Theta = Slice * 2.0f * Pi / Segments;
        const float CosTheta = std::cos(Theta);
        const float SinTheta = std::sin(Theta);

        OutPositions.push_back(CosTheta);
        OutPositions.push_back(-HalfHeight);
        OutPositions.push_back(SinTheta);
        OutNormals.push_back(0.0f);
        OutNormals.push_back(-1.0f);
        OutNormals.push_back(0.0f);
        OutUVs.push_back((CosTheta + 1.0f) * 0.5f);
        OutUVs.push_back((SinTheta + 1.0f) * 0.5f);
        OutTangents.push_back(-SinTheta);
        OutTangents.push_back(0.0f);
        OutTangents.push_back(CosTheta);
        OutTangents.push_back(0.0f);
    }

    for (int Slice = 0; Slice < Segments; ++Slice) {
        const uint32_t Top0 = static_cast<uint32_t>(Slice);
        const uint32_t Top1 = static_cast<uint32_t>(Slice + 1);
        const uint32_t Bottom0 = static_cast<uint32_t>(RingSize + Slice);
        const uint32_t Bottom1 = static_cast<uint32_t>(RingSize + Slice + 1);

        OutIndices.push_back(Top0);
        OutIndices.push_back(Bottom0);
        OutIndices.push_back(Bottom1);
        OutIndices.push_back(Top0);
        OutIndices.push_back(Bottom1);
        OutIndices.push_back(Top1);

        OutIndices.push_back(TopCenterIndex);
        OutIndices.push_back(TopRingIndex + static_cast<uint32_t>(Slice + 1));
        OutIndices.push_back(TopRingIndex + static_cast<uint32_t>(Slice));

        OutIndices.push_back(BottomCenterIndex);
        OutIndices.push_back(BottomRingIndex + static_cast<uint32_t>(Slice));
        OutIndices.push_back(BottomRingIndex + static_cast<uint32_t>(Slice + 1));
    }
}

static std::shared_ptr<StaticMesh> CreateCubeMesh()
{
    std::vector<float> Positions;
    std::vector<float> UVs;
    std::vector<float> Normals;
    std::vector<float> Tangents;
    std::vector<uint32_t> Indices;
    BuildCubeData(Positions, UVs, Normals, Tangents, Indices);
    return CreateStaticMeshFromData(std::move(Positions), std::move(UVs), std::move(Normals), std::move(Tangents), std::move(Indices));
}

static std::shared_ptr<StaticMesh> CreatePlaneMesh()
{
    std::vector<float> Positions;
    std::vector<float> UVs;
    std::vector<float> Normals;
    std::vector<float> Tangents;
    std::vector<uint32_t> Indices;
    BuildPlaneData(Positions, UVs, Normals, Tangents, Indices);
    return CreateStaticMeshFromData(std::move(Positions), std::move(UVs), std::move(Normals), std::move(Tangents), std::move(Indices));
}

static std::shared_ptr<StaticMesh> CreateSphereMesh()
{
    std::vector<float> Positions;
    std::vector<float> UVs;
    std::vector<float> Normals;
    std::vector<float> Tangents;
    std::vector<uint32_t> Indices;
    BuildSphereData(Positions, UVs, Normals, Tangents, Indices);
    return CreateStaticMeshFromData(std::move(Positions), std::move(UVs), std::move(Normals), std::move(Tangents), std::move(Indices));
}

static std::shared_ptr<StaticMesh> CreateCylinderMesh()
{
    std::vector<float> Positions;
    std::vector<float> UVs;
    std::vector<float> Normals;
    std::vector<float> Tangents;
    std::vector<uint32_t> Indices;
    BuildCylinderData(Positions, UVs, Normals, Tangents, Indices);
    return CreateStaticMeshFromData(std::move(Positions), std::move(UVs), std::move(Normals), std::move(Tangents), std::move(Indices));
}

} // namespace

StaticMeshSP GStaticMesh_Cube;
StaticMeshSP GStaticMesh_Sphere;
StaticMeshSP GStaticMesh_Plane;
StaticMeshSP GStaticMesh_Cylinder;

ENGINE_API bool InitializeShapeStaticMeshes()
{
    ReleaseShapeStaticMeshes();

    GStaticMesh_Cube = CreateCubeMesh();
    GStaticMesh_Plane = CreatePlaneMesh();
    GStaticMesh_Sphere = CreateSphereMesh();
    GStaticMesh_Cylinder = CreateCylinderMesh();

    return GStaticMesh_Cube && GStaticMesh_Plane && GStaticMesh_Sphere && GStaticMesh_Cylinder;
}

ENGINE_API void ReleaseShapeStaticMeshes()
{
    GStaticMesh_Cube.reset();
    GStaticMesh_Plane.reset();
    GStaticMesh_Sphere.reset();
    GStaticMesh_Cylinder.reset();
}

} // namespace Engine
