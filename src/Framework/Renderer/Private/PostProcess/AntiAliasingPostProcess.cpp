#include "AntiAliasingPostProcess.h"

namespace Renderer {

    IMPLEMENT_GLOBAL_SHADER_TYPE(
        taaTAA,
        "taaTAA",
        "/tools/taaTAA.sf",
        "MainCS",
        RHI::ERHIShaderFrequency::Compute
    );

    IMPLEMENT_GLOBAL_SHADER_TYPE(
        FxaaCS,
        "FxaaCS",
        "/tools/FxaaCS.sf",
        "MainCS",
        RHI::ERHIShaderFrequency::Compute
    );

} // namespace Renderer
