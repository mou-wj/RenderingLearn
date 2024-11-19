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
	//dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/skybox/bottom.jpg";//+y
	//textureInfos["skybox"].textureDataSources.push_back(dataSource);
	//dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/skybox/top.jpg";//-y
	//textureInfos["skybox"].textureDataSources.push_back(dataSource);
	//因为采样使用的是glsl的texture函数，其计算纹理坐标的方式是根据opengl的NDC坐标进行的，即按照opengl的NDC，其+y方向应该是向上的，而vulkan的+y是向下的，为了适配glsl的texture函数，这里反转上下所对应的图片
	dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/skybox/top.jpg";//+y
	textureInfos["skybox"].textureDataSources.push_back(dataSource);
	dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/skybox/bottom.jpg";//-y
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
	buffer.world = glm::mat4(1.0);
	buffer.view = Transform::GetViewMatrix(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));;
	buffer.proj = Transform::GetPerspectiveProj(0.1,100,-90,1);
	//ShowMat(buffer.view);
	//ShowMat(buffer.proj);
	//ShowVec(buffer.proj* buffer.view* glm::vec4(1, 1, 1, 1));

	FillBuffer(uniformBuffers["Buffer"], 0, sizeof(Buffer), (const char*)&buffer);


	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	buffer.view = Transform::GetEularRotateMatrix(0, -90, 0) * buffer.view;
	FillBuffer(uniformBuffers["Buffer"], 0, sizeof(Buffer), (const char*)&buffer);

	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		//确保presentFence在创建时已经触发
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);



		CmdListWaitFinish(graphicCommandList);
		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		
		CmdOpsDrawGeom(graphicCommandList);
		
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachment.attachmentImage, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsCopyWholeImageToImage(graphicCommandList, renderTargets.colorAttachment.attachmentImage, swapchainImages[nexIndex]);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachment.attachmentImage, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro
		Present(nexIndex, { finishCopyTargetToSwapchain });



	}

}


void SkyBoxExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
