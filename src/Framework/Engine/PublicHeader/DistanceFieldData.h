#pragma once

#include "EngineExport.h"
#include "RenderResource.h"
#include "Math.hpp"

#include <cstdint>
#include <vector>
#include <memory>

namespace Engine
{

    struct ENGINE_API DistanceFieldVolumeDesc
    {
        uint32_t ResolutionX = 0;
        uint32_t ResolutionY = 0;
        uint32_t ResolutionZ = 0;

        Core::Float3 MinBounds;
        Core::Float3 MaxBounds;

        Core::Float3 VoxelSize;

        uint32_t GetVoxelCount() const { return ResolutionX * ResolutionY * ResolutionZ; }
    };


    struct ENGINE_API DistanceFieldData
    {
        DistanceFieldVolumeDesc Volume;

        std::vector<float> Distance;
        std::vector<uint8_t> SurfaceMask;


        bool IsValid() const
        {
            return Distance.size() == Volume.GetVoxelCount();
        }


        void Resize(uint32_t X, uint32_t Y, uint32_t Z)
        {
            Volume.ResolutionX = X;
            Volume.ResolutionY = Y;
            Volume.ResolutionZ = Z;

            Distance.resize(Volume.GetVoxelCount());
            SurfaceMask.resize(Volume.GetVoxelCount());
        }


        uint32_t GetIndex(uint32_t X, uint32_t Y, uint32_t Z) const
        {
            return X + Y * Volume.ResolutionX + Z * Volume.ResolutionX * Volume.ResolutionY;
        }


        float GetDistance(uint32_t X, uint32_t Y, uint32_t Z) const
        {
            return Distance[GetIndex(X, Y, Z)];
        }


        uint8_t GetSurfaceMask(uint32_t X, uint32_t Y, uint32_t Z) const
        {
            return SurfaceMask[GetIndex(X, Y, Z)];
        }
    };

    static constexpr uint32_t InvalidDistanceFieldAllocation = UINT32_MAX;

    /**
     * Distance Field Atlas中的一块物理空间
     *
     * 只描述Texture中的位置
     *
     * 不包含任何World信息
     */
    struct DistanceFieldAllocation
    {
        uint32_t Id = InvalidDistanceFieldAllocation;


        uint32_t X = 0;
        uint32_t Y = 0;
        uint32_t Z = 0;


        uint32_t SizeX = 0;
        uint32_t SizeY = 0;
        uint32_t SizeZ = 0;



        bool IsValid() const
        {
            return Id != InvalidDistanceFieldAllocation;
        }
    };

    /*
     * Runtime GPU resource
     *
     * CPU DistanceFieldData -> GPU Texture3D
     */
    struct ENGINE_API DistanceFieldResource
    {
        DistanceFieldData CPUData;

        DistanceFieldAllocation Allocation;
    };






    /**
     * Distance Field Atlas Allocator
     *
     * 管理Texture3D内部空间分配
     *
     * 不关心:
     *
     * - 世界坐标
     * - Mesh
     * - Scene
     * - Bounds
     *
     * 只负责:
     *
     * Allocation <-> Physical Atlas Space
     */
    class ENGINE_API DistanceFieldAtlas
    {
    public:

        DistanceFieldAtlas() = default;

        ~DistanceFieldAtlas();



    public:

        /**
         * 初始化Atlas空间
         *
         * AtlasResolution:
         *
         * Texture3D尺寸
         *
         * BlockResolution:
         *
         * 最小分配单位
         */
        bool Initialize(
            uint32_t AtlasResolutionX,
            uint32_t AtlasResolutionY,
            uint32_t AtlasResolutionZ,
            uint32_t BlockResolution,
            RHI::ERHITextureCreateFlags usage = RHI::ERHITextureCreateFlag::ShaderResource | RHI::ERHITextureCreateFlag::TransferDest);



        void Release();



        /**
         * 分配一个Block空间
         */
        bool Allocate(
            DistanceFieldAllocation& OutAllocation);



        /**
         * 释放空间
         */
        void Free(
            const DistanceFieldAllocation& Allocation);



        /**
         * 查询Allocation是否有效
         */
        bool IsAllocated(
            uint32_t Id) const;


		void UploadData(
			const DistanceFieldAllocation& Allocation,
			const float* Data);

    public:


        uint32_t GetAtlasResolutionX() const
        {
            return AtlasResolutionX;
        }


        uint32_t GetAtlasResolutionY() const
        {
            return AtlasResolutionY;
        }


        uint32_t GetAtlasResolutionZ() const
        {
            return AtlasResolutionZ;
        }



        uint32_t GetBlockResolution() const
        {
            return BlockResolution;
        }

        RenderCore::RenderTexture* GetAtlasTexture() const
        {
            return AtlasTexture.get();
        }

    private:


        uint32_t CalculateIndex(
            uint32_t X,
            uint32_t Y,
            uint32_t Z) const;



    private:


        //---------------------------------
        // Atlas Texture尺寸
        //---------------------------------

        uint32_t AtlasResolutionX = 0;
        uint32_t AtlasResolutionY = 0;
        uint32_t AtlasResolutionZ = 0;



        //---------------------------------
        // 单个Allocation大小
        //---------------------------------

        uint32_t BlockResolution = 64;



        //---------------------------------
        // Block数量
        //---------------------------------

        uint32_t BlockCountX = 0;
        uint32_t BlockCountY = 0;
        uint32_t BlockCountZ = 0;



        //---------------------------------
        // 空间占用
        //
        // 0: free
        // 1: used
        //---------------------------------

        std::vector<uint8_t> AllocationMap;



        //---------------------------------
        // Allocation ID
        //---------------------------------

        uint32_t NextAllocationId = 0;

        RenderCore::RenderTextureSP AtlasTexture;
    };


}