#pragma once

#include "GlobalShader.h"
#include "ShaderParameter.h"
#include "RHICommandList.h"
namespace RenderCore {

// ----------------------------
// FullScreenVS
// ----------------------------


class RENDERCORE_API FFullScreenVS : public GlobalShader
{
public:
    FFullScreenVS() = default;

    //DECLARE_GLOBAL_VERTEX_SHADER(FFullScreenVS, "tools/FullScreenVS.sf", "FullScreenVS")
};




// ----------------------------
// DrawTexturePS
// ----------------------------


class RENDERCORE_API DrawTexturePS : public Shader
{
public:
    DrawTexturePS() = default;
    //DECLARE_GLOBAL_FRAGMENT_SHADER(DrawTexturePS, "tools/DrawTexturePS.sf", "DrawTexturePS")
};



// ----------------------------
// CopyTextureCS
// ----------------------------



class RENDERCORE_API FCopyTextureCS : public Shader
{
public:
    FCopyTextureCS() = default;
    //DECLARE_GLOBAL_COMPUTE_SHADER(FCopyTextureCS, "tools/CopyTextureCS.sf", "CopyTextureCS")

};



// ----------------------------
// BlitTextureCS
// ----------------------------
// Matches shaders/tools/BlitTextureCS.sf




class RENDERCORE_API FBlitTextureCS : public Shader
{
public:
    FBlitTextureCS() = default;
    //DECLARE_GLOBAL_COMPUTE_SHADER(FBlitTextureCS, "tools/BlitTextureCS.sf", "BlitTextureCS")
};



} // namespace RenderCore