#include "ScreenPass.h"
#include "RHIPipelineStateCache.h"
namespace Renderer {

	void addScreenPass(RenderGraphBuilder& builder, RenderGraphTextureSP inTexture, RenderGraphTextureSP renderTargetTexture) {
		ScenePassShaderParameter param;
		param.InText.Value = inTexture;
		param.RenderTargets.ColorTargets[0].Texture = renderTargetTexture;
		//获取默认管线状态
		RHIGraphicsPipelineStateDesc pipelineStateDesc;
		auto graphicStates = RHIPipelineStateCache::GetGraphicsPipelineState(pipelineStateDesc);
		auto lamdaFunc = [graphicStates](RHI::RHICommandList& cmdList) {
			cmdList.SetGraphicPipelineState(graphicStates);
			cmdList.SetStreamSource(0, nullptr, 0);
			cmdList.Draw(3, 1, 0, 0);
		};
		RenderGraphPassInfo info;
		info.ShaderMetaDatas = param.GetMetaDatas();
		info.PipelineType = ERHIPipelineType::Graphics;
		RenderGraphLambdaPass *scenePass = new RenderGraphLambdaPass("ScreenPass", info, lamdaFunc);
		builder.AddPass(scenePass);

	}

}