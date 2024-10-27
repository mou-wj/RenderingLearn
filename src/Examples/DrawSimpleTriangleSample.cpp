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
		DrawGeom({}, { graphicSemaphore });
		auto nexIndex = GetNextPresentImageIndex();
		CopyImageToImage(renderTargets.colorAttachment.attachmentImage, swapchainImages[nexIndex], { swapchainImageValidSemaphore }, { graphicSemaphore });
		Present({ graphicSemaphore }, {}, nexIndex);


	}

}
