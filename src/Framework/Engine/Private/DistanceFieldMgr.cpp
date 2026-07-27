#include "DistanceFieldMgr.h"
#include "StaticMeshResources.h"
#include "BVH.hpp"
#include "DistanceFieldProcess.h"
#include <limits>
#include <cmath>
using namespace Core;
namespace Engine
{
    DistanceFieldManager GDistanceFieldMgr;

    Float3 ClosestPointOnTriangle(
        const Float3& P,
        const Float3& A,
        const Float3& B,
        const Float3& C)
    {
        Float3 AB = B - A;
        Float3 AC = C - A;
        Float3 AP = P - A;


        float D1 = Dot(AB, AP);
        float D2 = Dot(AC, AP);


        if (D1 <= 0.0f && D2 <= 0.0f)
        {
            return A;
        }



        Float3 BP = P - B;

        float D3 = Dot(AB, BP);
        float D4 = Dot(AC, BP);


        if (D3 >= 0.0f && D4 <= D3)
        {
            return B;
        }



        float VC = D1 * D4 - D3 * D2;


        if (VC <= 0.0f && D1 >= 0.0f && D3 <= 0.0f)
        {
            float V = D1 / (D1 - D3);

            return A + AB * V;
        }



        Float3 CP = P - C;


        float D5 = Dot(AB, CP);
        float D6 = Dot(AC, CP);



        if (D6 >= 0.0f && D5 <= D6)
        {
            return C;
        }



        float VB = D5 * D2 - D1 * D6;


        if (VB <= 0.0f && D2 >= 0.0f && D6 <= 0.0f)
        {
            float W = D2 / (D2 - D6);

            return A + AC * W;
        }



        float VA = D3 * D6 - D5 * D4;


        if (VA <= 0.0f && (D4 - D3) >= 0.0f && (D5 - D6) >= 0.0f)
        {
            Float3 BC = C - B;

            float W = (D4 - D3) / ((D4 - D3) + (D5 - D6));

            return B + BC * W;
        }



        float Denom = 1.0f / (VA + VB + VC);

        float V = VB * Denom;

        float W = VC * Denom;


        return A + AB * V + AC * W;
    }

    Core::BoxSphereBounds DistanceFieldTriangleBVHTraits::GetBounds(const DistanceFieldTriangle& Triangle)
    {
        Core::AABB Box;
        Box.SetEmpty();
        Box.ExpandBy(Triangle.V0);
        Box.ExpandBy(Triangle.V1);
        Box.ExpandBy(Triangle.V2);
        return Core::BoxSphereBounds(Box);
    }


    Core::Float3 DistanceFieldTriangleBVHTraits::GetCentroid(const DistanceFieldTriangle& Triangle)
    {
        return Core::Float3((Triangle.V0.x + Triangle.V1.x + Triangle.V2.x) / 3.0f, (Triangle.V0.y + Triangle.V1.y + Triangle.V2.y) / 3.0f, (Triangle.V0.z + Triangle.V1.z + Triangle.V2.z) / 3.0f);
    }


    float DistanceFieldTriangleBVHTraits::ClosestDistance(const DistanceFieldTriangle& Triangle, const Core::Float3& Position)
    {
        Core::Float3 Point = ClosestPointOnTriangle(Position, Triangle.V0, Triangle.V1, Triangle.V2);

        Core::Float3 Delta = Point - Position;

        return sqrt(Delta.x * Delta.x + Delta.y * Delta.y + Delta.z * Delta.z);
    }

    bool DistanceFieldTriangleBVHTraits::RayIntersect(const DistanceFieldTriangle& Primitive, const Core::Ray& Ray, float& OutDistance)
    {
        constexpr float Epsilon = 1e-6f;

        const Core::Float3 Edge1 = Primitive.V1 - Primitive.V0;
        const Core::Float3 Edge2 = Primitive.V2 - Primitive.V0;

        const Core::Float3 P = Core::Cross(Ray.Direction, Edge2);

        const float Det = Core::Dot(Edge1, P);

        if (std::fabs(Det) < Epsilon)
        {
            return false;
        }

        const float InvDet = 1.0f / Det;

        const Core::Float3 T = Ray.Origin - Primitive.V0;

        const float U = Core::Dot(T, P) * InvDet;

        if (U < 0.0f || U > 1.0f)
        {
            return false;
        }

        const Core::Float3 Q = Core::Cross(T, Edge1);

        const float V = Core::Dot(Ray.Direction, Q) * InvDet;

        if (V < 0.0f || U + V > 1.0f)
        {
            return false;
        }

        const float Distance = Core::Dot(Edge2, Q) * InvDet;

        if (Distance < Epsilon)
        {
            return false;
        }

        OutDistance = Distance;

        return true;
    }

    bool DistanceFieldTriangleBVHTraits::IntersectVoxel(const DistanceFieldTriangle& Triangle, const Core::AABB& VoxelBounds)
    {
		BoxSphereBounds TriangleSphere = GetBounds(Triangle);
		return TriangleSphere.Box.Intersects(VoxelBounds) || VoxelBounds.Contains(TriangleSphere.Box);
    }
    

    namespace
    {
        constexpr int32_t GLocalSDFResolution = 64;

        void FillTexture3D(RenderCore::RenderTexture* texture, float fillValue)
        {
            if (!texture || !texture->GetRHI())
            {
                return;
            }

            const auto& desc = texture->GetRHI()->GetDesc();
            const size_t voxelCount =
                static_cast<size_t>(desc.Width) *
                static_cast<size_t>(desc.Height) *
                static_cast<size_t>(desc.Depth);
            std::vector<float> initData(voxelCount, fillValue);
            texture->UploadData(initData.data(), 0, 0);
        }

        Core::Float4x4 BuildWorldToVoxel(const Core::AABB& bounds, const Core::Int3& resolution)
        {
            Core::Float4x4 m = Core::Float4x4::Identity();
            if (bounds.IsEmpty())
            {
                return m;
            }

            const float sizeX = CORE_MAX(bounds.Max.x - bounds.Min.x, 1e-4f);
            const float sizeY = CORE_MAX(bounds.Max.y - bounds.Min.y, 1e-4f);
            const float sizeZ = CORE_MAX(bounds.Max.z - bounds.Min.z, 1e-4f);

            const float sx = static_cast<float>(CORE_MAX(1, resolution.x - 1)) / sizeX;
            const float sy = static_cast<float>(CORE_MAX(1, resolution.y - 1)) / sizeY;
            const float sz = static_cast<float>(CORE_MAX(1, resolution.z - 1)) / sizeZ;

            m(0, 0) = sx;
            m(1, 1) = sy;
            m(2, 2) = sz;
            m(0, 3) = -bounds.Min.x * sx;
            m(1, 3) = -bounds.Min.y * sy;
            m(2, 3) = -bounds.Min.z * sz;
            return m;
        }

        std::shared_ptr<RenderCore::RenderBuffer> CreateStructuredBufferFromCPU(
            const void* data,
            uint64_t elementCount,
            uint32_t stride,
            const char* debugName)
        {
            RHI::RHIBufferDesc desc;
            desc.Size = CORE_MAX(1ull, elementCount * static_cast<uint64_t>(stride));
            desc.Stride = stride;
            desc.Usage =
                RHI::ERHIBufferUsageFlag::Structured |
                RHI::ERHIBufferUsageFlag::ShaderResource |
                RHI::ERHIBufferUsageFlag::TransferDst;
            desc.InitialQueueType = RHI::EQueueType::Compute;
            desc.DebugName = debugName;

            auto buffer = std::make_shared<RenderCore::RenderBuffer>(desc);
            buffer->InitRHIResource();
            if (data && elementCount > 0)
            {
                buffer->UploadData(data, static_cast<uint32_t>(elementCount * static_cast<uint64_t>(stride)));
            }
            return buffer;
        }
    }

    DistanceFieldManager::DistanceFieldManager(Core::TaskPool* InTaskPool)
        : TaskPool(InTaskPool)
    {
        
    }



    DistanceFieldManager::~DistanceFieldManager()
    {
    }

    void DistanceFieldManager::Initialize()
    {
        if (IsInitialized) {
            return;
        }
        IsInitialized = true;
        Atlas.Initialize(128, 128, 128, 64);
    }

    void DistanceFieldManager::Release()
    {
        IsInitialized = false;
		Atlas.Release();
    }



    void DistanceFieldManager::BuildMeshDistanceField(StaticMeshRenderData& Mesh)
    {
        bool BuildInCPU = true;
        if (BuildInCPU) {
            BuildCPUData(Mesh);
            for (auto& LOD : Mesh.LODResources) {
                
                if (LOD.DistanceFieldData.CPUData.Distance.empty()) {
                    continue;
                }
				auto AllocationSuccess = Atlas.Allocate(LOD.DistanceFieldData.Allocation);
                Atlas.UploadData(LOD.DistanceFieldData.Allocation, LOD.DistanceFieldData.CPUData.Distance.data());
                
            }
        }
        else {
			for (auto& LOD : Mesh.LODResources) {
				auto SurfaceMask = BuildSurfaceMaskTexture(Mesh.Bounds, LOD);
                
			}
        }
    }



    bool DistanceFieldManager::BuildCPUData(StaticMeshRenderData& Mesh)
    {
        const uint32_t Resolution = GLocalSDFResolution;


        const Core::AABB& Bounds = Mesh.Bounds.Box;



        bool res = true;
        for (auto& LOD : Mesh.LODResources) {
            auto& OutData = LOD.DistanceFieldData.CPUData;
            OutData.Volume.ResolutionX = Resolution;
            OutData.Volume.ResolutionY = Resolution;
            OutData.Volume.ResolutionZ = Resolution;


            OutData.Volume.MinBounds = Bounds.Min;
            OutData.Volume.MaxBounds = Bounds.Max;


            Core::Float3 Size = Bounds.Max - Bounds.Min;


            OutData.Volume.VoxelSize = Core::Float3(
                Size.x / Resolution,
                Size.y / Resolution,
                Size.z / Resolution);


            OutData.Resize(
                Resolution,
                Resolution,
                Resolution);

            std::vector<DistanceFieldTriangle> Triangles;


            if (!BuildTriangles(LOD, Triangles))
            {
                continue;
            }



            DistanceFieldBVH BVH;

            BVH.Build(Triangles);



            res &= GenerateDistanceCPU(
                OutData,
                BVH);
        }

        return res;
    }



    bool DistanceFieldManager::BuildTriangles(const LODResource& LOD, std::vector<DistanceFieldTriangle>& OutTriangles)
    {

        const auto& Positions = LOD.VertexBuffers.PositionBuffer.Vertices;

        const auto& Indices = LOD.IndexBuffer.Indices;


        if (Positions.empty() || Indices.empty())
        {
            return false;
        }



        OutTriangles.reserve(Indices.size() / 3);



        for (size_t Index = 0; Index < Indices.size(); Index += 3)
        {
            uint32_t I0 = Indices[Index];
            uint32_t I1 = Indices[Index + 1];
            uint32_t I2 = Indices[Index + 2];


            DistanceFieldTriangle Triangle;


            Triangle.V0 = Core::Float3(
                Positions[I0 * 3],
                Positions[I0 * 3 + 1],
                Positions[I0 * 3 + 2]);


            Triangle.V1 = Core::Float3(
                Positions[I1 * 3],
                Positions[I1 * 3 + 1],
                Positions[I1 * 3 + 2]);


            Triangle.V2 = Core::Float3(
                Positions[I2 * 3],
                Positions[I2 * 3 + 1],
                Positions[I2 * 3 + 2]);


            OutTriangles.push_back(Triangle);
        }


        return true;
    }



    bool DistanceFieldManager::GenerateDistanceCPU(DistanceFieldData& Data, const DistanceFieldBVH& BVH)
    {
        std::vector<Core::TaskHandle> Tasks;


        for (uint32_t Z = 0; Z < Data.Volume.ResolutionZ; Z++)
        {
            if (TaskPool)
            {
                Tasks.emplace_back(TaskPool->AddTask(
                    [&, Z]()
                    {
                        GenerateDistanceSlice(Data, Z, BVH);
                    }));
            }
            else
            {
                GenerateDistanceSlice(Data, Z, BVH);
            }
        }



        if (TaskPool)
        {
            TaskPool->WaitAll(Tasks);
        }



        return true;
    }



    void DistanceFieldManager::GenerateDistanceSlice(DistanceFieldData& Data, uint32_t Z, const DistanceFieldBVH& BVH)
    {
        const uint32_t SizeX = Data.Volume.ResolutionX;
        const uint32_t SizeY = Data.Volume.ResolutionY;



        for (uint32_t Y = 0; Y < SizeY; Y++)
        {
            for (uint32_t X = 0; X < SizeX; X++)
            {
                uint32_t Index = Data.GetIndex(X, Y, Z);



                Core::Float3 Position(
                    Data.Volume.MinBounds.x + X * Data.Volume.VoxelSize.x,
                    Data.Volume.MinBounds.y + Y * Data.Volume.VoxelSize.y,
                    Data.Volume.MinBounds.z + Z * Data.Volume.VoxelSize.z);



                Data.Distance[Index] = BVH.ClosestDistance(Position);
                Core::Ray Hit;
                Hit.Origin = Position;
				Core::Float3 center = BVH.GetRootNode().Bounds.Box.GetCenter();
                Hit.Direction = center - Hit.Origin;
                Hit.Direction = Core::Normalize(Hit.Direction);
                std::vector<uint32_t> intersectIds;
                BVH.RayIntersect(Hit, intersectIds);
                if (intersectIds.size() == 1) {
                    Data.Distance[Index] = -Data.Distance[Index];
                }
            }
        }
    }
    static Core::Float3 GetVoxelPosition(const DistanceFieldVolumeDesc& Volume, uint32_t X, uint32_t Y, uint32_t Z)
    {
        return Core::Float3(
            Volume.MinBounds.x + (X + 0.5f) * Volume.VoxelSize.x,
            Volume.MinBounds.y + (Y + 0.5f) * Volume.VoxelSize.y,
            Volume.MinBounds.z + (Z + 0.5f) * Volume.VoxelSize.z
        );
    }
    static Core::AABB GetVoxelBounds(const DistanceFieldVolumeDesc& Volume, uint32_t X, uint32_t Y, uint32_t Z)
    {
        Core::Float3 Min(
            Volume.MinBounds.x + X * Volume.VoxelSize.x,
            Volume.MinBounds.y + Y * Volume.VoxelSize.y,
            Volume.MinBounds.z + Z * Volume.VoxelSize.z
        );

        Core::Float3 Max = Min + Volume.VoxelSize;

        return Core::AABB(Min, Max);
    }
    RenderCore::RenderTextureSP DistanceFieldManager::BuildSurfaceMaskTexture(const Core::BoxSphereBounds& bounds, const LODResource& LOD)
    {
        std::vector<DistanceFieldTriangle> Triangles;


        if (!BuildTriangles(LOD, Triangles))
        {
            return nullptr;
        }



        DistanceFieldBVH BVH;

        BVH.Build(Triangles);
        const uint32_t Resolution = GLocalSDFResolution;
		std::vector<uint8_t> SurfaceMask(Resolution * Resolution * Resolution, 255);
        
        //²¹³ä
        DistanceFieldVolumeDesc Volume;

        Volume.ResolutionX = Resolution;
        Volume.ResolutionY = Resolution;
        Volume.ResolutionZ = Resolution;

        Volume.MinBounds = bounds.Box.Min;
        Volume.MaxBounds = bounds.Box.Max;


        Core::Float3 Size =
            bounds.Box.Max - bounds.Box.Min;


        Volume.VoxelSize =
            Core::Float3(
                Size.x / Resolution,
                Size.y / Resolution,
                Size.z / Resolution);



        constexpr uint32_t BlockSize = 8;


        std::vector<Core::TaskHandle> Tasks;



        auto ProcessBlock =
            [&](uint32_t BX, uint32_t BY, uint32_t BZ)
            {
                uint32_t StartX = BX * BlockSize;
                uint32_t StartY = BY * BlockSize;
                uint32_t StartZ = BZ * BlockSize;


                uint32_t EndX = CORE_MIN(StartX + BlockSize, Resolution);
                uint32_t EndY = CORE_MIN(StartY + BlockSize, Resolution);
                uint32_t EndZ = CORE_MIN(StartZ + BlockSize, Resolution);



                Core::AABB BlockBounds(
                    Core::Float3(
                        Volume.MinBounds.x + StartX * Volume.VoxelSize.x,
                        Volume.MinBounds.y + StartY * Volume.VoxelSize.y,
                        Volume.MinBounds.z + StartZ * Volume.VoxelSize.z),
                    Core::Float3(
                        Volume.MinBounds.x + EndX * Volume.VoxelSize.x,
                        Volume.MinBounds.y + EndY * Volume.VoxelSize.y,
                        Volume.MinBounds.z + EndZ * Volume.VoxelSize.z)
                );


                Core::BoxSphereBounds QueryBounds(BlockBounds);



                std::vector<uint32_t> PrimitiveIds;


                BVH.GetPrimitiveIdsInsideBounds(
                    QueryBounds,
                    PrimitiveIds);



                if (PrimitiveIds.empty())
                    return;



                for (uint32_t Z = StartZ; Z < EndZ; Z++)
                {
                    for (uint32_t Y = StartY; Y < EndY; Y++)
                    {
                        for (uint32_t X = StartX; X < EndX; X++)
                        {

                            Core::AABB VoxelBounds =
                                GetVoxelBounds(
                                    Volume,
                                    X, Y, Z);


                            uint32_t Index =
                                Z * Resolution * Resolution +
                                Y * Resolution +
                                X;


                            for (uint32_t PrimitiveId : PrimitiveIds)
                            {

                                const auto& Triangle =
                                    BVH.GetPrimitive(
                                        PrimitiveId);



                                if (DistanceFieldTriangleBVHTraits::IntersectVoxel(
                                    Triangle,
                                    VoxelBounds))
                                {

                                    SurfaceMask[Index] =
                                        static_cast<uint8_t>(
                                            PrimitiveId < 128 ?
                                            PrimitiveId :
                                            127);


                                    goto NextVoxel;
                                }
                            }


                        NextVoxel:;
                        }
                    }
                }
            };



        uint32_t BlockCount =
            (Resolution + BlockSize - 1) / BlockSize;



        for (uint32_t Z = 0; Z < BlockCount; Z++)
        {
            for (uint32_t Y = 0; Y < BlockCount; Y++)
            {
                for (uint32_t X = 0; X < BlockCount; X++)
                {

                    if (TaskPool)
                    {
                        Tasks.emplace_back(
                            TaskPool->AddTask(
                                [&, X, Y, Z]()
                                {
                                    ProcessBlock(X, Y, Z);
                                }));
                    }
                    else
                    {
                        ProcessBlock(X, Y, Z);
                    }

                }
            }
        }



        if (TaskPool)
        {
            TaskPool->WaitAll(Tasks);
        }



		RenderCore::RenderTextureSP SurfaceMaskTex = RenderCore::Create3DTexture(
			Resolution,
			Resolution,
			Resolution,
			RHI::ERHIFormat::B8G8R8_UNorm,
            RHI::ERHITextureCreateFlag::ShaderResource | RHI::ERHITextureCreateFlag::TransferDest,
			"SurfaceMaskTest");
		SurfaceMaskTex->UploadData(SurfaceMask.data(), 0, 0);
        return SurfaceMaskTex;
    }

    RenderCore::RenderTextureSP DistanceFieldManager::BuildSDFTexture(RenderCore::RenderTexture* SurfaceMask)
    {
        const uint32_t Resolution = GLocalSDFResolution;
        RenderCore::RenderTextureSP SDFTex = RenderCore::Create3DTexture(
            Resolution,
            Resolution,
            Resolution,
            RHI::ERHIFormat::B8G8R8_UNorm,
            RHI::ERHITextureCreateFlag::ShaderResource | RHI::ERHITextureCreateFlag::TransferDest,
            "SDFTexture");
        DistanceFieldJumpFlood3DPassInput input;
        input.SurfaceMaskTexture = SurfaceMask;
		input.OutputDistanceTexture = SDFTex.get();
        input.GridResolution = Core::Int3(Resolution, Resolution, Resolution);
        ExecuteDistanceFieldJumpFlood3DPass(input);
        return SDFTex;
    }



    


}