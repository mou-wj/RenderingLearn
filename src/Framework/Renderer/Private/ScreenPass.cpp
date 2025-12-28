#include "ScreenPass.h"
#include "RHIPipelineStateCache.h"
#include "GlobalShader.h"
namespace Renderer {

	void addScreenPass(RenderGraphBuilder& builder, RenderGraphTextureSP inTexture) {
		auto& GShaderMap = RenderCore::GetGlobalShaderMap();
		auto drawTextureShaderType = GlobalShader::GetGlobalShaderType("DrawTexturePS");
        auto fullScreenQuadShaderType = GlobalShader::GetGlobalShaderType("FullScreenVS");
		
		auto drawTexture = GShaderMap.GetShader(nullptr,0);
		auto fullScreenQuad = GShaderMap.GetShader(nullptr, 0);
		//获取默认管线状态
		RHIGraphicsPipelineStateDesc pipelineStateDesc;
		pipelineStateDesc.shaderStages[0].shader = drawTexture->GetRHIShader();
        pipelineStateDesc.shaderStages[1].shader = fullScreenQuad->GetRHIShader();
		auto graphicStates = RHIPipelineStateCache::GetGraphicsPipelineState(pipelineStateDesc);
		auto parameter = builder.AllocateParameter<ScenePassShaderParameter>();
		parameter->InText.Value = inTexture;
		RenderGraphPassInfo info;
		info.ShaderParmeters = (const ScenePassShaderParameter*) & parameter;
		info.PassFlag = EPassFlag::Graphic;

		builder.AddPass("ScreenPass", info, [graphicStates,parameter, info, drawTextureShaderType, fullScreenQuadShaderType](RHI::RHICommandList& cmdList) {
			cmdList.SetGraphicPipelineState(graphicStates);
			info.ShaderParmeters.SetShaderParameters(cmdList, nullptr, &drawTextureShaderType->ParameterBindingInfo);
			info.ShaderParmeters.SetShaderParameters(cmdList, nullptr, &fullScreenQuadShaderType->ParameterBindingInfo);
			cmdList.SetStreamSource(0, nullptr, 0);
			cmdList.Draw(3, 1, 0, 0);
			});

	}

}