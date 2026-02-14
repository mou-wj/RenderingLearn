#include "ScreenPass.h"
#include "RHIPipelineStateCache.h"
#include "GlobalShader.h"
namespace Renderer {
	//BEGIN_SHADER_PARAMETER_STRUCT(ScenePassShaderParameter)
	//	SHADER_PARAMETER_TEXTURE(InText)
	//	SHADER_PARAMETER_RENDER_TARGETS()
	//END_SHADER_PARAMETER_STRUCT(ScenePassShaderParameter);

	void addScreenPass(RenderGraphBuilder& builder, RenderGraphTexture* inTexture, RenderGraphTexture* outTexture) {
		auto& GShaderMap = RenderCore::GetGlobalShaderMap();
		auto drawTextureShaderType = GlobalShader::GetGlobalShaderType("DrawTexturePS");
        auto fullScreenQuadShaderType = GlobalShader::GetGlobalShaderType("FullScreenVS");
		
		auto drawTexture = GShaderMap.GetShader(drawTextureShaderType,0);
		auto fullScreenQuad = GShaderMap.GetShader(fullScreenQuadShaderType, 0);
		//获取默认管线状态
		RHIGraphicsPipelineStateDesc pipelineStateDesc;
		pipelineStateDesc.shaderStages[0].shader = drawTexture->GetRHIShader();
        pipelineStateDesc.shaderStages[1].shader = fullScreenQuad->GetRHIShader();
		auto graphicStates = RHIPipelineStateCache::GetGraphicsPipelineState(pipelineStateDesc);
		//auto parameter = builder.AllocateParameter<ScenePassShaderParameter>();
		//parameter->InText.Value = inTexture;
		//parameter->RenderTargets.ColorTargets[0].Texture = outTexture;
		//RenderGraphPassInfo info;
		//info.ShaderParmeters = (const ScenePassShaderParameter*) & parameter;
		//info.PassFlag = EPassFlag::Graphic;
		//
		//builder.AddPass("ScreenPass", info, [graphicStates,parameter, info, drawTextureShaderType, //fullScreenQuadShaderType, drawTexture](RHI::RHICommandList& cmdList) {
		//	cmdList.SetGraphicPipelineState(graphicStates.get());
		//	info.ShaderParmeters.SetShaderParameters(cmdList, drawTexture, drawTextureShaderType-//>ParameterBindingInfo);
		//	cmdList.SetStreamSource(0, nullptr, 0);
		//	cmdList.Draw(3, 1, 0, 0);
		//	});
		//
	}

}