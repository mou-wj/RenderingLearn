#include "DrawSkyPass.h"

namespace Renderer {
    IMPLEMENT_GLOBAL_SHADER_TYPE(
        DrawSkyVS,
        "DrawSkyVS",
        "/tools/DrawSkyVS.sf",
        "MainVS",
        RHI::ERHIShaderFrequency::Vertex
    );

    IMPLEMENT_GLOBAL_SHADER_TYPE(
        DrawSkyPS,
        "DrawSkyPS",
        "/tools/DrawSkyPS.sf",
        "MainPS",
        RHI::ERHIShaderFrequency::Fragment
    );
}
