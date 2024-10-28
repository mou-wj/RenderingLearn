#include "DrawSimpleTriangleSample.h"

void DrawSimpleTriangleSample::InitSubPassInfo()
{
	InitDefaultGraphicSubpassInfo();


}

void DrawSimpleTriangleSample::InitResourceInfos()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/resources/shader/glsl/DrawSimpleTriangle.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/resources/shader/glsl/DrawSimgleTriangle.frag";
	ParseShaderFiles({ shaderCodePath });
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geom);





}

void DrawSimpleTriangleSample::Loop()
{
	while (1)
	{
		//确保presentFence在创建时已经触发
		WaitAllFence({ presentFence });
		DrawGeom({}, { drawSemaphore });
		auto nexIndex = GetNextPresentImageIndex(swapchainImageValidSemaphore);
		CopyImageToImage(renderTargets.colorAttachment.attachmentImage, swapchainImages[nexIndex], { swapchainImageValidSemaphore,drawSemaphore }, { presentValidSemaphore });
		Present({ presentValidSemaphore }, {presentFinishSemaphore}, nexIndex);
		WaitSemaphrore({ presentFinishSemaphore }, presentFence);
		ResetAllFence({ presentFence });

	}

}
