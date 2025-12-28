#pragma once
#include "ShaderParameter.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphResource.h"
namespace Renderer {
	using namespace RenderCore;

    BEGIN_SHADER_PARAMETER_STRUCT(ScenePassShaderParameter)
        SHADER_PARAMETER_TEXTURE(InText)
        SHADER_PARAMETER_RENDER_TARGETS()
    END_SHADER_PARAMETER_STRUCT(ScenePassShaderParameter);

	RENDERER_API void addScreenPass(RenderGraphBuilder& builder, RenderGraphTextureSP inTexture);


}
