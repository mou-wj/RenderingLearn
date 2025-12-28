#pragma once

#include "RHIResource.h"
#include "RHIDefine.h"
#include <array>

namespace RHI {

/**
 * 渲染目标绑定信息结构体
 * 描述颜色/深度/模板附件绑定的资源及其参数
 */
struct RHI_API RHIRenderTargetInfo
{
    /** 绑定的纹理资源 */
    RHITextureSP Texture = nullptr;
    
    /** 绑定的纹理数组层索引 */
    uint32_t ArraySlice = 0;
    
    /** 绑定的纹理Mip层索引 */ 
    uint32_t MipLevel = 0;

    ERHILoadAction LoadAction = ERHILoadAction::DontCare;

    ERHIStoreAction StoreAction = ERHIStoreAction::DontCare;
    
    /** 清除颜色值(仅当LoadAction为Clear时有效) */
    RHIColor ClearColor;
    
    /** 清除深度值(仅当LoadAction为Clear时有效) */
    float ClearDepth = 1.0f;
    
    /** 清除模板值(仅当LoadAction为Clear时有效) */
    uint32_t ClearStencil = 0;
    
    // 构造函数
    RHIRenderTargetInfo() = default;
    
    // 颜色附件构造函数
    RHIRenderTargetInfo(RHITextureSP InTexture, 
                       uint32_t InArraySlice = 0, uint32_t InMipLevel = 0,
                       ERHILoadAction InLoadAction = ERHILoadAction::Load,
                       ERHIStoreAction InStoreAction = ERHIStoreAction::Store,
                        const RHIColor& InClearColor = RHIColor{})
        : Texture(InTexture)
        , ArraySlice(InArraySlice)
        , MipLevel(InMipLevel)
        , LoadAction(InLoadAction)
        , StoreAction(InStoreAction)
        , ClearColor(InClearColor)
    {}
    
    // 深度/模板附件构造函数
    RHIRenderTargetInfo(RHITextureSP InTexture,
                       uint32_t InArraySlice = 0, uint32_t InMipLevel = 0,
                       ERHILoadAction InLoadAction = ERHILoadAction::Load,
                       ERHIStoreAction InStoreAction = ERHIStoreAction::Store,
                       float InClearDepth = 1.0f, uint32_t InClearStencil = 0)
        : Texture(InTexture)
        , ArraySlice(InArraySlice)
        , MipLevel(InMipLevel)
        , LoadAction(InLoadAction)
        , StoreAction(InStoreAction)
        , ClearDepth(InClearDepth)
        , ClearStencil(InClearStencil)
    {}
    
    // 比较运算符
    bool operator==(const RHIRenderTargetInfo& Other) const
    {
        return Texture == Other.Texture &&
               ArraySlice == Other.ArraySlice &&
               MipLevel == Other.MipLevel &&
               LoadAction == Other.LoadAction &&
               StoreAction == Other.StoreAction;
    }
    
    bool operator!=(const RHIRenderTargetInfo& Other) const
    {
        return !(*this == Other);
    }
};


/**
 * 渲染目标集合，包含所有颜色附件和深度/模板附件
 */
struct RHI_API RHIRenderTargetsInfo
{
    // 最大颜色附件数量
    static constexpr int32_t MaxColorRenderTargets = 8;
    
    // 颜色附件数组
    std::array<RHIRenderTargetInfo*, MaxColorRenderTargets> ColorRenderTargets;
    
    // 深度附件
    RHIRenderTargetInfo* DepthStencilRenderTarget = nullptr;
    
    // 视口尺寸
    RHIIntRect ViewRect;
    
    // 构造函数
    RHIRenderTargetsInfo()
    {
        ColorRenderTargets.fill(nullptr);
    }
    
    // 添加颜色附件
    void SetColorRenderTarget(int32_t Index, RHIRenderTargetInfo* RTInfo)
    {
        ColorRenderTargets[Index] = RTInfo;
    }
    
    // 设置深度附件
    void SetDepthStencilRenderTarget(RHIRenderTargetInfo* RTInfo)
    {
        DepthStencilRenderTarget = RTInfo;
    }
};



} // namespace WR::RHI
