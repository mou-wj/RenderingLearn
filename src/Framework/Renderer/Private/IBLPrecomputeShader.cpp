#include "IBLPrecomputeShader.h"
#include "RenderGraphBuilder.h"
#include "RHIPipelineStateCache.h"
#include "ShaderParameter.h"
#include "GlobalShader.h"

namespace Renderer {
    RenderCore::RenderTextureSP GlobalIBLLutTexture = nullptr;

    IMPLEMENT_GLOBAL_SHADER_TYPE(
        IBLPrecomputeCS,
        "IBLPrecomputeCS",
        "/tools/IBLPrecomputeCS.sf",
        "MainCS",
        RHI::ERHIShaderFrequency::Compute
    );


} // namespace Renderer
