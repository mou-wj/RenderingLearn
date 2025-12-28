#pragma once

#include "GlobalShader.h"
#include "ShaderParameter.h"

namespace RenderCore {

// ----------------------------
// FullScreenVS
// ----------------------------
BEGIN_SHADER_PARAMETER_STRUCT(FFullScreenVSParameters)
END_SHADER_PARAMETER_STRUCT(FFullScreenVSParameters);

class RENDERCORE_API FFullScreenVS : public GlobalShader
{
public:
    FFullScreenVS() = default;

    DECLARE_GLOBAL_VERTEX_SHADER(FFullScreenVS, "tools/FullScreenVS.sf", "FullScreenVS")
};




// ----------------------------
// DrawTexturePS
// ----------------------------
BEGIN_SHADER_PARAMETER_STRUCT(FDrawTexturePSParameters)
    SHADER_PARAMETER_TEXTURE_SRV(InputTexture)
END_SHADER_PARAMETER_STRUCT(FDrawTexturePSParameters);

class RENDERCORE_API DrawTexturePS : public Shader
{
public:
    DrawTexturePS() = default;
    DECLARE_GLOBAL_VERTEX_SHADER(DrawTexturePS, "tools/DrawTexturePS.sf", "DrawTexturePS")
};



// ----------------------------
// CopyTextureCS
// ----------------------------
BEGIN_SHADER_PARAMETER_STRUCT(FCopyTextureCB)
    SHADER_PARAMETER(int, SrcSizeX)
    SHADER_PARAMETER(int, SrcSizeY)
END_SHADER_PARAMETER_STRUCT(FCopyTextureCB);

BEGIN_SHADER_PARAMETER_STRUCT(FCopyTextureCSParameters)
    SHADER_PARAMETER_TEXTURE_SRV(InputTexture)
    SHADER_PARAMETER_TEXTURE_UAV(OutputTexture)
    SHADER_PARAMETER_UNIFORM_BUFFER(CopyCB)
END_SHADER_PARAMETER_STRUCT(FCopyTextureCSParameters);

class RENDERCORE_API FCopyTextureCS : public Shader
{
public:
    FCopyTextureCS() = default;
    DECLARE_GLOBAL_COMPUTE_SHADER(FCopyTextureCS, "tools/CopyTextureCS.sf", "CopyTextureCS")

};



// ----------------------------
// BlitTextureCS
// ----------------------------
// Matches shaders/tools/BlitTextureCS.sf
BEGIN_SHADER_PARAMETER_STRUCT(FBlitTextureCB)
    SHADER_PARAMETER(float, SrcSizeX)
    SHADER_PARAMETER(float, SrcSizeY)
    SHADER_PARAMETER(float, DstSizeX)
    SHADER_PARAMETER(float, DstSizeY)
END_SHADER_PARAMETER_STRUCT(FBlitTextureCB);

BEGIN_SHADER_PARAMETER_STRUCT(FBlitTextureCSParameters)
    SHADER_PARAMETER_TEXTURE_SRV(SrcTexture)
    SHADER_PARAMETER_TEXTURE_SRV(LinearSampler) // sampler may be represented separately in RHI
    SHADER_PARAMETER_TEXTURE_UAV(DstTexture)
    SHADER_PARAMETER_UNIFORM_BUFFER(BlitParams) // cbuffer BlitParams : register(b0)
END_SHADER_PARAMETER_STRUCT(FBlitTextureCSParameters);

class RENDERCORE_API FBlitTextureCS : public Shader
{
public:
    FBlitTextureCS() = default;
    DECLARE_GLOBAL_COMPUTE_SHADER(FBlitTextureCS, "tools/BlitTextureCS.sf", "BlitTextureCS")
};


} // namespace RenderCore