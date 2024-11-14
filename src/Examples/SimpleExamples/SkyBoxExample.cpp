#include "SkyBoxExample.h"
#include "glm/mat4x4.hpp"

void SkyBoxExample::InitSubPassInfo()
{
	InitDefaultGraphicSubpassInfo();


}

void SkyBoxExample::InitResourceInfos()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SkyBoxExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SkyBoxExample.frag";
	pipelinesShaderCodePaths = { shaderCodePath };
	geoms.resize(1);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geoms[0]);

	//	
	TextureDataSource dataSource;
	dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/skybox/right.jpg";//+x
	textureInfos["skybox"].textureDataSources.push_back(dataSource);
	dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/skybox/left.jpg";//-x
	textureInfos["skybox"].textureDataSources.push_back(dataSource);
	dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/skybox/bottom.jpg";//+y
	textureInfos["skybox"].textureDataSources.push_back(dataSource);
	dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/skybox/top.jpg";//-y
	textureInfos["skybox"].textureDataSources.push_back(dataSource);
	dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/skybox/front.jpg";//+z
	textureInfos["skybox"].textureDataSources.push_back(dataSource);
	dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/skybox/back.jpg";//-z
	textureInfos["skybox"].textureDataSources.push_back(dataSource);
	textureInfos["skybox"].binding = 1;
	textureInfos["skybox"].viewType = VK_IMAGE_VIEW_TYPE_CUBE;




	uniformBufferInfos["Buffer"].size = sizeof(glm::mat4) * 3;
	uniformBufferInfos["Buffer"].binding = 0;
}

void SkyBoxExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	//std::vector<char> testImageData;

	//testImageData.resize(512 * 512 * 4);
	//for (uint32_t i = 0; i < 512; i++)
	//{
	//	for (uint32_t j = 0; j < 512; j++)
	//	{
	//		testImageData[(512 * i + j) * 4] = 255;
	//		testImageData[(512 * i + j) * 4 + 1] = 0;
	//		testImageData[(512 * i + j) * 4 + 2] = 0;
	//		testImageData[(512 * i + j) * 4 + 3] = 255;
	//	}


	//}

	//Texture testTexture = Create2DTexture(512, 512, testImageData.data());
	struct Buffer {
		glm::mat4 world;
		glm::mat4 view;
		glm::mat4 proj;
	} buffer;
	buffer.world = Transform::GetTransformMatrixFromRH();
	buffer.view = Transform::GetViewMatrix(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));;
	buffer.proj = Transform::GetPerspectiveProj(0.1,100,90,1);

	FillBuffer(uniformBuffers["Buffer"], 0, sizeof(Buffer), (const char*)&buffer);

	uint32_t numCap = 4;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		CaptureBeginMacro
		DrawGeom({}, { drawSemaphore });
		CaptureEndMacro;

		auto nexIndex = GetNextPresentImageIndex(swapchainImageValidSemaphore);
		CopyImageToImage(renderTargets.colorAttachment.attachmentImage, swapchainImages[nexIndex], { swapchainImageValidSemaphore,drawSemaphore }, { presentValidSemaphore });

		//CopyImageToImage(testTexture.image, swapchainImages[nexIndex], { swapchainImageValidSemaphore }, { presentValidSemaphore });
		//CopyImageToImage(renderTargets.colorAttachment.attachmentImage,testTexture.image, { drawSemaphore }, { presentValidSemaphore });
		//WaitIdle();
		//auto rgba = (const char*)testTexture.image.hostMapPointer;
		//auto r = rgba[0];
		//auto g = rgba[1];
		//auto b = rgba[2];
		//auto a = rgba[3];

		Present({ presentValidSemaphore }, { presentFinishSemaphore }, nexIndex);
		//if (numCap != 0)
		//{
		;
		//	numCap--;
		//}

	}

}
