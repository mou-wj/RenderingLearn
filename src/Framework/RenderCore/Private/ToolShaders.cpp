#include "ToolShaders.h"
namespace RenderCore {

    IMPLEMENT_GLOBAL_SHADER_TYPE(
        BlitTextureCS,
        "BlitTextureCS",                     
        "/tools/BlitTextureCS.sf",   
        "MainCS",                                
        RHI::ERHIShaderFrequency::Compute 
    );
}


