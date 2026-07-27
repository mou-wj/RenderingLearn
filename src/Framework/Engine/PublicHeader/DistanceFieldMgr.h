#pragma once

#include "EngineExport.h"
#include "DistanceFieldData.h"
#include "TaskPool.h"
#include "BVH.hpp"

#include <unordered_map>
#include <memory>


namespace Engine
{
    struct LODResource;
    struct DistanceFieldTriangle
    {
        Core::Float3 V0;
        Core::Float3 V1;
        Core::Float3 V2;
    };


    struct DistanceFieldTriangleBVHTraits
    {
        static Core::BoxSphereBounds GetBounds(const DistanceFieldTriangle& Triangle);


        static Core::Float3 GetCentroid(const DistanceFieldTriangle& Triangle);


        static float ClosestDistance(const DistanceFieldTriangle& Triangle, const Core::Float3& Position);

        static bool RayIntersect(const DistanceFieldTriangle& Primitive, const Core::Ray& Ray, float& OutDistance);

        static bool IntersectVoxel(const DistanceFieldTriangle& Triangle, const Core::AABB& VoxelBounds);
    };

    using DistanceFieldBVH = Core::BVH<DistanceFieldTriangle, DistanceFieldTriangleBVHTraits>;
    

    class StaticMeshRenderData;




    class ENGINE_API DistanceFieldManager
    {
    public:

        explicit DistanceFieldManager(Core::TaskPool* InTaskPool = nullptr);

        ~DistanceFieldManager();

        void Initialize();

        void Release();


        void BuildMeshDistanceField(
            StaticMeshRenderData& Mesh);

		const DistanceFieldAtlas& GetAtlas() { return Atlas; }


    private:

        bool BuildCPUData(StaticMeshRenderData& Mesh);



        bool BuildTriangles(
            const LODResource& LOD,
            std::vector<DistanceFieldTriangle>& OutTriangles);



        bool GenerateDistanceCPU(
            DistanceFieldData& Data,
            const DistanceFieldBVH& BVH);



        void GenerateDistanceSlice(
            DistanceFieldData& Data,
            uint32_t Z,
            const DistanceFieldBVH& BVH);

        RenderCore::RenderTextureSP BuildSurfaceMaskTexture(const Core::BoxSphereBounds& bounds, const LODResource& LOD);

        RenderCore::RenderTextureSP BuildSDFTexture(RenderCore::RenderTexture* SurfaceMask);

    private:

        Core::TaskPool* TaskPool = nullptr;
        DistanceFieldAtlas Atlas;
        bool IsInitialized = false;

    };

    ENGINE_API extern DistanceFieldManager GDistanceFieldMgr;
}