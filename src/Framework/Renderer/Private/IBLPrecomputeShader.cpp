#include "IBLPrecomputeShader.h"
#include "RenderGraphBuilder.h"
#include "RHIPipelineStateCache.h"
#include "ShaderParameter.h"
#include "GlobalShader.h"

namespace Renderer {
    IMPLEMENT_GLOBAL_SHADER_TYPE(
        IBLPrecomputeCS,
        "IBLPrecomputeCS",
        "/tools/IBLPrecomputeBRDFLut.sf",
        "MainCS",
        RHI::ERHIShaderFrequency::Compute
    );


} // namespace Renderer
