#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>
#include "ShaderParameter.h"
#include "Common.h"
#include "BoxSphereBounds.h"

#include "DistanceFieldData.h"


namespace Renderer
{


    static constexpr uint32_t GDFInvalidBlockId = UINT32_MAX;
    BEGIN_SHADER_PARAMETER_STRUCT(GlobalDistanceFieldBlockIndex)
        SHADER_PARAMETER(uint32_t, Valid)
        SHADER_PARAMETER(uint32_t, AllocationX)
        SHADER_PARAMETER(uint32_t, AllocationY)
        SHADER_PARAMETER(uint32_t, AllocationZ)
    END_SHADER_PARAMETER_STRUCT(GlobalDistanceFieldBlockIndex)


    /**
     * Global Distance Field Block
     *
     * 一个世界空间区域对应一个Block
     *
     * 负责:
     *
     * 1. 世界范围
     * 2. Grid坐标
     * 3. Atlas Allocation引用
     */
    struct GlobalDistanceFieldBlock
    {
        uint32_t BlockId = GDFInvalidBlockId;

        //--------------------------------
        // PageTable Index
        //--------------------------------

        uint32_t PageIndex = GDFInvalidBlockId;

        //--------------------------------
        // 世界空间Block坐标
        //--------------------------------

        int32_t GridX = 0;
        int32_t GridY = 0;
        int32_t GridZ = 0;



        //--------------------------------
        // 世界空间范围
        //--------------------------------

        Core::AABB Bounds;



        //--------------------------------
        // DistanceField Atlas空间
        //--------------------------------

        Engine::DistanceFieldAllocation Allocation;



        bool IsValid() const
        {
            return BlockId != GDFInvalidBlockId;
        }
    };





    /**
     * 一次世界区域分配产生的Block集合
     *
     * 例如:
     *
     * 一个区域:
     *
     * 100m * 100m * 100m
     *
     * 划分:
     *
     * 5*5*5 Block
     */
    struct GlobalDistanceFieldBlockClipMap
    {
        Core::BoxSphereBounds Bounds;



        int32_t MinBlockX = 0;
        int32_t MinBlockY = 0;
        int32_t MinBlockZ = 0;



        uint32_t SizeX = 0;
        uint32_t SizeY = 0;
        uint32_t SizeZ = 0;



        std::vector<uint32_t> BlockIds;



        bool IsValid() const
        {
            return !BlockIds.empty();
        }
    };






    /**
     * Global Distance Field (有限范围的全局距离场，以center为中心的前后左右上下100m范围)
     *
     * 管理:
     *
     * World Block
     *
     *       |
     *
     * DistanceFieldAllocation
     *
     *       |
     *
     * DistanceFieldAtlas
     *
     *
     * 不负责:
     *
     * GPU Texture
     * Atlas物理分配
     */
    class GlobalDistanceField
    {
    public:

        GlobalDistanceField() = default;

        ~GlobalDistanceField();



    public:


        bool Initialize(
            float InVoxelSize = 5/64.0);



        void Release();



    public:


        /**
         * 根据世界范围分配Block
         */
        bool Allocate(
            const Core::AABB& Bounds,
            GlobalDistanceFieldBlockClipMap& OutClipMap);



        /**
         * 释放区域对应Block
         */
        void Release(
            const GlobalDistanceFieldBlockClipMap& ClipMap);



        /**
         * 根据世界位置获取Block
         */
        bool GetBlock(
            const Core::Float3& Position,
            GlobalDistanceFieldBlock& OutBlock) const;



        /**
         * 获取范围内所有Block
         */
        void GetBlocksInsideBounds(
            const Core::AABB& Bounds,
            std::vector<uint32_t>& OutBlockIds) const;



        /**
         * 获取Block
         */
        const GlobalDistanceFieldBlock* GetBlock(
            uint32_t BlockId) const;



        /**
         * 更新Block数据
         *
         * 实际上传交给Renderer
         */
        bool GetBlockAllocation(
            uint32_t BlockId,
            Engine::DistanceFieldAllocation& OutAllocation) const;

		void UploadBlockIndexBufferToGPU();

		RenderCore::RenderBuffer* GetBlockIndexBufferGPU() const
		{
			return BlockIndexBufferGPU.get();
		}

        const Core::Float3& GetCenter() const {
            return Bounds.GetCenter();
        }

        const Engine::DistanceFieldAtlas& GetAtlas() const {
            return *Atlas;
        }

    public:


        uint32_t GetBlockResolution() const
        {
            return Atlas->GetBlockResolution();
        }


        float GetBlockWorldSize() const
        {
            return BlockWorldSize;
        }



    private:


        uint32_t CalculateBlockId(
            int32_t X,
            int32_t Y,
            int32_t Z) const;



        void CalculateBlockCoordinate(
            const Core::Float3& Position,
            int32_t& X,
            int32_t& Y,
            int32_t& Z) const;



        Core::BoxSphereBounds CalculateBlockBounds(
            int32_t X,
            int32_t Y,
            int32_t Z) const;



    private:


        //--------------------------------
        // Physical Atlas
        //--------------------------------

        std::unique_ptr<Engine::DistanceFieldAtlas> Atlas = nullptr;



        //--------------------------------
        // Block参数
        //--------------------------------

        float VoxelSize = 1.0f;


        float BlockWorldSize = 64.0f;

        uint32_t GridSizeX = 0;
        uint32_t GridSizeY = 0;
        uint32_t GridSizeZ = 0;

        std::vector<GlobalDistanceFieldBlockIndex> BlockIndexBufferCPU;
        RenderCore::RenderBufferSP BlockIndexBufferGPU;

        //--------------------------------
        // 当前World Block
        //--------------------------------

        std::unordered_map<uint32_t, GlobalDistanceFieldBlock> Blocks;

        Core::AABB Bounds;
        bool IsInitialized = false;

    };

    BEGIN_SHADER_PARAMETER_STRUCT(DistanceFieldSourceParameters)
        SHADER_PARAMETER(Core::Float4x4, LocalToWorld)
        SHADER_PARAMETER(Core::Float4x4, WorldToLocal)
        SHADER_PARAMETER(Core::UInt3, Offset)
        SHADER_PARAMETER(float,Padding0)
        SHADER_PARAMETER(Core::UInt3, InputResolution)
        SHADER_PARAMETER(float, Padding1)
        SHADER_PARAMETER(Core::Float3, BoundsMin)
        SHADER_PARAMETER(float, Padding2)
        SHADER_PARAMETER(Core::Float3, BoundsMax)
        SHADER_PARAMETER(float, Padding3)
    END_SHADER_PARAMETER_STRUCT(DistanceFieldSourceParameters)
    BEGIN_SHADER_PARAMETER_STRUCT(DistanceFieldOutParameters)
        SHADER_PARAMETER(Core::Int3, Offset)
        SHADER_PARAMETER(float, Padding0)
        SHADER_PARAMETER(Core::Float3, BoundsMin)
        SHADER_PARAMETER(float, Padding1)
        SHADER_PARAMETER(Core::Float3, BoundsMax)
        SHADER_PARAMETER(float, Padding2)
    END_SHADER_PARAMETER_STRUCT(DistanceFieldOutParameters)
    struct DistanceFieldMergePassInput
    {
        std::vector<DistanceFieldSourceParameters> sourceParams;
        std::vector<DistanceFieldOutParameters> outputParams;
        RenderCore::RenderTexture* InputSDFTexture = nullptr;
        RenderCore::RenderTexture* OutputSDFTexture = nullptr;
    };
    bool ExecuteDistanceFieldMergePass(const DistanceFieldMergePassInput& Input);

}