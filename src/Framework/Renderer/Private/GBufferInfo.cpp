#include "GBufferInfo.h"
namespace Renderer {

    FGBufferInfo CreateGBufferInfo(const FGBufferParams& Params)
    {
        FGBufferInfo Info;

        Info.Layout =
            Params.bHasVelocity
            ? GBL_ForceVelocity
            : GBL_Default;

        Info.NumTargets = 0;

        //=========================================================================
        // Targets
        //=========================================================================

        Info.Targets[0] =
            FGBufferTarget(
                RHI::ERHIFormat::R8G8B8A8_UNorm,
                "GBufferA");

        Info.Targets[1] =
            FGBufferTarget(
                RHI::ERHIFormat::R16G16B16A16_Float,
                "GBufferB");

        Info.Targets[2] =
            FGBufferTarget(
                RHI::ERHIFormat::R8G8B8A8_UNorm,
                "GBufferC");

        Info.NumTargets = 3;

        if (Params.bHasVelocity)
        {
            Info.Targets[3] =
                FGBufferTarget(
                    RHI::ERHIFormat::R16G16_Float,
                    "Velocity");

            Info.NumTargets++;
        }

        //=========================================================================
        // Slot Mapping
        //=========================================================================

        Info.Slots[GBS_BaseColor] =
            FGBufferItem(GBS_BaseColor, 0, 0, 3);

        Info.Slots[GBS_Metallic] =
            FGBufferItem(GBS_Metallic, 0, 3, 1);

        Info.Slots[GBS_WorldNormal] =
            FGBufferItem(GBS_WorldNormal, 1, 0, 3);

        Info.Slots[GBS_Roughness] =
            FGBufferItem(GBS_Roughness, 1, 3, 1);
        Info.Slots[GBS_ShadingModelId] =
            FGBufferItem(GBS_ShadingModelId, 2, 2, 1);

        if (Params.bHasSpecular)
        {
            Info.Slots[GBS_Specular] =
                FGBufferItem(GBS_Specular, 2, 0, 1);
        }

        if (Params.bHasAO)
        {
            Info.Slots[GBS_AO] =
                FGBufferItem(GBS_AO, 2, 1, 1);
        }

        if (Params.bHasVelocity)
        {
            Info.Slots[GBS_Velocity] =
                FGBufferItem(GBS_Velocity, 3, 0, 2);
        }

        return Info;
    }

    const FGBufferItem* FindGBufferSlot(
        const FGBufferInfo& GBufferInfo,
        EGBufferSlot Slot)
    {
        if (Slot <= GBS_Invalid || Slot >= GBS_Num)
        {
            return nullptr;
        }

        const FGBufferItem& Item =
            GBufferInfo.Slots[Slot];

        if (!Item.bIsValid)
        {
            return nullptr;
        }

        return &Item;
    }

    const FGBufferTarget* FindGBufferTargetByName(
        const FGBufferInfo& GBufferInfo,
        const std::string& Name)
    {
        for (int i = 0; i < GBufferInfo.NumTargets; i++)
        {
            const FGBufferTarget& Target =
                GBufferInfo.Targets[i];

            if (Target.TargetName == Name)
            {
                return &Target;
            }
        }

        return nullptr;
    }


}