#include "DistanceFieldData.h"

namespace Engine
{


    DistanceFieldAtlas::~DistanceFieldAtlas()
    {
        Release();
    }



    bool DistanceFieldAtlas::Initialize(
        uint32_t InAtlasResolutionX,
        uint32_t InAtlasResolutionY,
        uint32_t InAtlasResolutionZ,
        uint32_t InBlockResolution)
    {
        Release();


        if (InBlockResolution == 0)
        {
            return false;
        }


        if (InAtlasResolutionX % InBlockResolution != 0 ||
            InAtlasResolutionY % InBlockResolution != 0 ||
            InAtlasResolutionZ % InBlockResolution != 0)
        {
            return false;
        }



        AtlasResolutionX = InAtlasResolutionX;
        AtlasResolutionY = InAtlasResolutionY;
        AtlasResolutionZ = InAtlasResolutionZ;


        BlockResolution = InBlockResolution;



        BlockCountX =
            AtlasResolutionX / BlockResolution;


        BlockCountY =
            AtlasResolutionY / BlockResolution;


        BlockCountZ =
            AtlasResolutionZ / BlockResolution;



        uint32_t TotalBlocks =
            BlockCountX *
            BlockCountY *
            BlockCountZ;



        AllocationMap.resize(
            TotalBlocks,
            0);



        NextAllocationId = 0;

        AtlasTexture = RenderCore::Create3DTexture(InAtlasResolutionX, InAtlasResolutionY, InAtlasResolutionZ, RHI::ERHIFormat::R32_Float, RHI::ERHITextureCreateFlag::ShaderResource | RHI::ERHITextureCreateFlag::TransferDest, "DistanceFieldAtlas");
        return true;
    }



    void DistanceFieldAtlas::Release()
    {
        AllocationMap.clear();


        AtlasResolutionX = 0;
        AtlasResolutionY = 0;
        AtlasResolutionZ = 0;


        BlockCountX = 0;
        BlockCountY = 0;
        BlockCountZ = 0;


        NextAllocationId = 0;

        AtlasTexture.reset();
    }



    uint32_t DistanceFieldAtlas::CalculateIndex(
        uint32_t X,
        uint32_t Y,
        uint32_t Z) const
    {
        return X +
            Y * BlockCountX +
            Z * BlockCountX * BlockCountY;
    }



    bool DistanceFieldAtlas::Allocate(
        DistanceFieldAllocation& OutAllocation)
    {
        for (uint32_t Z = 0; Z < BlockCountZ; Z++)
        {
            for (uint32_t Y = 0; Y < BlockCountY; Y++)
            {
                for (uint32_t X = 0; X < BlockCountX; X++)
                {
                    uint32_t Index =
                        CalculateIndex(
                            X,
                            Y,
                            Z);



                    if (AllocationMap[Index] != 0)
                    {
                        continue;
                    }



                    AllocationMap[Index] = 1;



                    OutAllocation.Id =
                        NextAllocationId++;



                    OutAllocation.X =
                        X * BlockResolution;


                    OutAllocation.Y =
                        Y * BlockResolution;


                    OutAllocation.Z =
                        Z * BlockResolution;



                    OutAllocation.SizeX =
                        BlockResolution;


                    OutAllocation.SizeY =
                        BlockResolution;


                    OutAllocation.SizeZ =
                        BlockResolution;



                    return true;
                }
            }
        }


        return false;
    }



    void DistanceFieldAtlas::Free(
        const DistanceFieldAllocation& Allocation)
    {
        if (!Allocation.IsValid())
        {
            return;
        }



        uint32_t X =
            Allocation.X / BlockResolution;


        uint32_t Y =
            Allocation.Y / BlockResolution;


        uint32_t Z =
            Allocation.Z / BlockResolution;



        if (X >= BlockCountX ||
            Y >= BlockCountY ||
            Z >= BlockCountZ)
        {
            return;
        }



        uint32_t Index =
            CalculateIndex(
                X,
                Y,
                Z);



        AllocationMap[Index] = 0;
    }



    bool DistanceFieldAtlas::IsAllocated(
        uint32_t Id) const
    {
        /*
         * 当前版本:
         *
         * Allocation ID
         * 只用于调试和追踪
         *
         * 没有维护ID->Slot映射
         *
         */


        return Id != InvalidDistanceFieldAllocation;
    }
    void DistanceFieldAtlas::UploadData(
        const DistanceFieldAllocation& Allocation,
        const float* Data) {

    }


}