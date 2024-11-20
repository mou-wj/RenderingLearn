#include "SimpleSceenExample.h"
#include "glm/mat4x4.hpp"

void SimpleSceenExample::InitSubPassInfo()
{
	//InitDefaultGraphicSubpassInfo();
	//两个subpass 一个绘制天空盒，一个绘制场景
	subpassInfo.subpassDescs.resize(2);
	auto& subpassDesc1 = subpassInfo.subpassDescs[0];
	subpassDesc1.flags = 0;
	subpassDesc1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc1.inputAttachmentCount = 0;
	subpassDesc1.pInputAttachments = nullptr;
	subpassDesc1.colorAttachmentCount = 1;
	subpassDesc1.pColorAttachments = &renderTargets.colorRef;
	subpassDesc1.pResolveAttachments = nullptr;
	subpassDesc1.pDepthStencilAttachment = &renderTargets.depthRef;
	subpassDesc1.preserveAttachmentCount = 0;
	subpassDesc1.pPreserveAttachments = nullptr;

	auto& subpassDesc2 = subpassInfo.subpassDescs[1];
	subpassDesc2.flags = 0;
	subpassDesc2.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc2.inputAttachmentCount = 0;
	subpassDesc2.pInputAttachments = nullptr;
	subpassDesc2.colorAttachmentCount = 1;
	subpassDesc2.pColorAttachments = &renderTargets.colorRef;
	subpassDesc2.pResolveAttachments = nullptr;
	subpassDesc2.pDepthStencilAttachment = &renderTargets.depthRef;
	subpassDesc2.preserveAttachmentCount = 0;
	subpassDesc2.pPreserveAttachments = nullptr;


	subpassInfo.subpassDepends.resize(2);
	auto& subpassDepend1 = subpassInfo.subpassDepends[0];
	subpassDepend1.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDepend1.dstSubpass = 0;
	subpassDepend1.dependencyFlags = 0;
	subpassDepend1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDepend1.srcAccessMask = 0;
	subpassDepend1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDepend1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	auto& subpassDepend2 = subpassInfo.subpassDepends[1];
	subpassDepend2.srcSubpass = 0;
	subpassDepend2.dstSubpass = 1;
	subpassDepend2.dependencyFlags = 0;
	subpassDepend2.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDepend2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	subpassDepend2.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDepend2.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


}

void SimpleSceenExample::InitResourceInfos()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SkyBoxExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SkyBoxExample.frag";
	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SimpleSceenExample.vert";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SimpleSceenExample.frag";
	
	
	pipelinesShaderCodePaths = { shaderCodePath,drawSceenCodePath };
	geoms.resize(2);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geoms[0]);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/SimpleSceen.obj", geoms[1]);

	subpassDrawGeoInfos[0] = { 0 };
	subpassDrawGeoInfos[1] = { 1 };

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

	uniformBufferInfos["SimpleSceenExampleBuffer"].size = sizeof(glm::mat4) * 3;
	uniformBufferInfos["SimpleSceenExampleBuffer"].binding = 0;
	uniformBufferInfos["SimpleSceenExampleBuffer"].pipeId = 1;
	
}

void SimpleSceenExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");


	Camera camera(glm::vec3(0,0,6),glm::vec3(0,0,7),glm::vec3(0,1,0));
	struct Buffer {
		glm::mat4 world;
		glm::mat4 view;
		glm::mat4 proj;
	} buffer;
	buffer.world = glm::mat4(1.0);
	buffer.view = Transform::GetViewMatrix(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));;
	buffer.proj = camera.GetProj();
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


	WindowEventHandler::SetActiveCamera(&camera);
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();

		//确保presentFence在创建时已经触发
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
		buffer.view = camera.GetView();
		FillBuffer(uniformBuffers["Buffer"], 0, sizeof(Buffer), (const char*)&buffer);
		FillBuffer(uniformBuffers["SimpleSceenExampleBuffer"], 0, sizeof(Buffer), (const char*)&buffer);

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


void SimpleSceenExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
