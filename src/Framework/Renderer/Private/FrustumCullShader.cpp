#include "FrustumCullShader.h"

namespace Renderer {

    IMPLEMENT_GLOBAL_SHADER_TYPE(
        FrustumCullCS,
        "FrustumCullCS",
        "/tools/FrustumCullCS.sf",
        "MainCS",
        RHI::ERHIShaderFrequency::Compute
    );

} // namespace Renderer
