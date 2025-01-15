#include "C7ShadowsExample.h"
#include "glm/mat4x4.hpp"

void C7ShadowsExample::InitSubPassInfo()
{
	ShaderCodePaths depthPassCodePath;
	depthPassCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C7ShadowsExampleDepthPass.vert";
	//drawSceenCodePath.geometryShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C7ShadowsExample.geom";
	depthPassCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C7ShadowsExampleDepthPass.frag";


	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C7ShadowsExample.vert";
	//drawSceenCodePath.geometryShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C7ShadowsExample.geom";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C7ShadowsExample.frag";

	renderPassInfos.resize(2);
	renderPassInfos[0].InitDefaultRenderPassInfo(depthPassCodePath, windowWidth, windowHeight);
	renderPassInfos[0].InitDefaultRenderPassInfo(drawSceenCodePath, windowWidth, windowHeight);



	
}

void C7ShadowsExample::InitResourceInfos()
{

	geoms.resize(1);
	geoms[0].useIndexBuffers = false;
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geoms[0]);

	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };



	bufferBindInfos["SimpleSceenExampleBuffer"].size = sizeof(glm::mat4) * 3;
	bufferBindInfos["SimpleSceenExampleBuffer"].binding = 0;
	bufferBindInfos["SimpleSceenExampleBuffer"].pipeId = 0;


	//一个纹理点光源阴影贴图
	TextureDataSource emptyDataSource;
	emptyDataSource.width = windowWidth;
	emptyDataSource.height = windowHeight;
	emptyDataSource.picturePath = "";
	textureBindInfos["pointShadowMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["pointShadowMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["pointShadowMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["pointShadowMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["pointShadowMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["pointShadowMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["pointShadowMap"].binding = 1;
	textureBindInfos["pointShadowMap"].viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	textureBindInfos["pointShadowMap"].format = VK_FORMAT_R32_SFLOAT;
}

void C7ShadowsExample::Loop()
{
	uint32_t i = 0;;
	uint32_t exampleType = 0;


	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");


	Camera camera(glm::vec3(0,0,0),glm::vec3(0,0,1),glm::vec3(0,1,0));
	//绑定camera响应按键的回调函数
	WindowEventHandler::SetEventCallBack(KEY_W_PRESS, [&camera]() {camera.Move(MoveDirection::FORWARD); }, "点击w 相机前移");
	WindowEventHandler::SetEventCallBack(KEY_S_PRESS, [&camera]() {camera.Move(MoveDirection::BACK); }, "点击s 相机后移");
	WindowEventHandler::SetEventCallBack(KEY_A_PRESS, [&camera]() {camera.Move(MoveDirection::LEFT); }, "点击a 相机左移");
	WindowEventHandler::SetEventCallBack(KEY_D_PRESS, [&camera]() {camera.Move(MoveDirection::RIGHT); }, "点击d 相机右移");
	WindowEventHandler::SetEventCallBack(KEY_UP_PRESS, [&camera]() {
		//往上看相当于所有点往下旋转，即z->y,即AROUND_X_NEGATIVE
		camera.Rotate(RotateAction::AROUND_X_NEGATIVE); }, "点击up 相机往上看");
	WindowEventHandler::SetEventCallBack(KEY_DOWN_PRESS, [&camera]() {camera.Rotate(RotateAction::AROUND_X_POSITIVE); }, "点击down 相机往下看");
	WindowEventHandler::SetEventCallBack(KEY_RIGHT_PRESS, [&camera]() {
		//往右看相当于所有点往左旋转，即x->z，即AROUND_Y_POSITIVE
		camera.Rotate(RotateAction::AROUND_Y_POSITIVE);
		}, "点击right 相机往右看");
	WindowEventHandler::SetEventCallBack(KEY_LEFT_PRESS, [&camera]() {camera.Rotate(RotateAction::AROUND_Y_NEGATIVE); }, "点击left 相机往左看");



	struct Buffer {
		glm::mat4 world;
		glm::mat4 view;
		glm::mat4 proj;
	} buffer;
	buffer.world = glm::mat4(1.0);
	buffer.view = camera.GetView();
	buffer.proj = camera.GetProj();




	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };


	//绑定uniform buffer
	BindBuffer("SimpleSceenExampleBuffer");
	const std::vector<glm::vec3> cubeViewTarget = {
		glm::vec3(1,0,0),
		glm::vec3(-1,0,0),
		glm::vec3(0,1,0),
		glm::vec3(0,-1,0),
		glm::vec3(0,0,1),
		glm::vec3(0,0,-1),
	
	};
	const std::vector<glm::vec3> cubeViewDown = {
	glm::vec3(0,1,0),
	glm::vec3(0,1,0),
	glm::vec3(0,0,-1),
	glm::vec3(0,0,1),
	glm::vec3(0,1,0),
	glm::vec3(0,1,0),

	};

	//glm::vec3 lightPos = glm::vec3(-3,-6,0);
	glm::vec3 lightPos = glm::vec3(0, 0, 0);
	for (uint32_t i = 0; i < 6; i++)
	{
		//获取阴影贴图
		camera.SetCamera(lightPos, cubeViewTarget[i], cubeViewDown[i]);
		buffer.view = camera.GetView();
		FillBuffer(buffers["SimpleSceenExampleBuffer"], 0, sizeof(Buffer), (const char*)&buffer);
		CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理

		CmdListReset(graphicCommandList);

		CmdListRecordBegin(graphicCommandList);
		//进行depth pass
		CmdOpsDrawGeom(graphicCommandList);
		auto& depthTargetImage = renderPassInfos[0].renderTargets.depthAttachment.attachmentImage;

		auto& depthOldLayout = depthTargetImage.currentLayout;
		auto& depthTextureOldLayout = textures["pointShadowMap"].image.currentLayout;

		CmdOpsImageMemoryBarrer(graphicCommandList, depthTargetImage, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,0,0);
		CmdOpsImageMemoryBarrer(graphicCommandList, textures["pointShadowMap"].image, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);
		
		//拷贝深度图到深度纹理
		CmdOpsCopyImageToImage(graphicCommandList, depthTargetImage, 0, 0, textures["pointShadowMap"].image, 0, 0);

		CmdOpsImageMemoryBarrer(graphicCommandList, depthTargetImage, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, depthOldLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0, 0);
		CmdOpsImageMemoryBarrer(graphicCommandList, textures["pointShadowMap"].image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, depthTextureOldLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);


	}
	textures["pointShadowMap"].image.WriteToJpgFloat("depthMap0.jpg", 0, 0);

	//绘制阴影
	auto& renderTargets = renderPassInfos[1].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;

		//确保presentFence在创建时已经触发
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
		buffer.view = camera.GetView();
		FillBuffer(buffers["SimpleSceenExampleBuffer"], 0, sizeof(Buffer), (const char*)&buffer);

		CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理
		WindowEventHandler::ProcessEvent();
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
	WaitIdle();
}


void C7ShadowsExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
