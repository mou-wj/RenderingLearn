#include "PBRArtifactExample.h"
#include "glm/mat4x4.hpp"

void PBRArtifactExample::InitSubPassInfo()
{


	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/PBRArtifactExample.vert";
	drawSceenCodePath.geometryShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/PBRArtifactExample.geom";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/PBRArtifactExample.frag";

	ShaderCodePaths bgCodePath;
	bgCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/PBRArtifactExampleBG.vert";
	bgCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/PBRArtifactExampleBG.frag";

	
	renderPassInfos.resize(1);

	renderPassInfos[0].InitDefaultRenderPassInfo({ drawSceenCodePath,bgCodePath }, windowWidth, windowHeight);
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//写入深度附件

	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//写入深度附件
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_NONE;
}

void PBRArtifactExample::InitResourceInfos()
{

	geoms.resize(2);
	geoms[0].useIndexBuffers = false;
	geoms[1].useIndexBuffers = true;
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/sphere.obj",geoms[0]);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj", geoms[1]);


	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	renderPassInfos[0].subpassDrawGeoInfos[1] = { 1 };






	std::vector<std::string> skyboxImages = {
		//std::string(PROJECT_DIR) + "/resources/pic/skybox/right.jpg",//+x
		//std::string(PROJECT_DIR) + "/resources/pic/skybox/left.jpg",//-x
		////因为采样使用的是glsl的texture函数，其计算纹理坐标的方式是根据opengl的NDC坐标进行的，即按照opengl的NDC，其+y方向应该是向上的，而vulkan的+y是向下的，为了适配glsl的texture函数，这里可以反转+-y所对应的图片，但是反转+-y图片其他方向任然存在上下颠倒问题，所以不采取反转+-y图片而进行所有面图片的反转读取
		//std::string(PROJECT_DIR) + "/resources/pic/skybox/bottom.jpg",//+y
		//std::string(PROJECT_DIR) + "/resources/pic/skybox/top.jpg",//-y
		//std::string(PROJECT_DIR) + "/resources/pic/skybox/front.jpg",//+z
		//std::string(PROJECT_DIR) + "/resources/pic/skybox/back.jpg"//-z
		std::string(PROJECT_DIR) + "/resources/pic/skybox1/right.png",//+x
		std::string(PROJECT_DIR) + "/resources/pic/skybox1/left.png",//-x
		//因为采样使用的是glsl的texture函数，其计算纹理坐标的方式是根据opengl的NDC坐标进行的，即按照opengl的NDC，其+y方向应该是向上的，而vulkan的+y是向下的，为了适配glsl的texture函数，这里可以反转+-y所对应的图片，但是反转+-y图片其他方向任然存在上下颠倒问题，所以不采取反转+-y图片而进行所有面图片的反转读取
		std::string(PROJECT_DIR) + "/resources/pic/skybox1/bottom.png",//+y
		std::string(PROJECT_DIR) + "/resources/pic/skybox1/top.png",//-y
		std::string(PROJECT_DIR) + "/resources/pic/skybox1/front.png",//+z
		std::string(PROJECT_DIR) + "/resources/pic/skybox1/back.png"//-z
	};
	textureBindInfos["bgEnvTexture"] = TextureBindInfo(skyboxImages);
	textureBindInfos["bgEnvTexture"].binding = 1;
	textureBindInfos["bgEnvTexture"].viewType = VK_IMAGE_VIEW_TYPE_CUBE;



	TextureDataSource emptyDataSource;
	emptyDataSource.picturePath = std::string(PROJECT_DIR) + "/resources/material/Wood066_1K-JPG/Wood066_1K-JPG_Color.jpg";
	textureBindInfos["baseColorTexture"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["baseColorTexture"].binding = 2;

	emptyDataSource.picturePath = std::string(PROJECT_DIR) + "/resources/material/Wood066_1K-JPG/Wood066_1K-JPG_NormalGL.jpg";
	textureBindInfos["normalTexture"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["normalTexture"].binding = 4;


	emptyDataSource.picturePath = std::string(PROJECT_DIR) + "/resources/material/Wood066_1K-JPG/Wood066_1K-JPG_Roughness.jpg";
	textureBindInfos["roughnessTexture"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["roughnessTexture"].binding = 3;

	bufferBindInfos["TransformBuffer"].size = sizeof(glm::mat4) * 3;
	bufferBindInfos["TransformBuffer"].binding = 0;
	bufferBindInfos["TransformBuffer"].pipeId = 0;

	bufferBindInfos["Info"].size = sizeof(glm::vec4) * 1;
	bufferBindInfos["Info"].binding = 1;
	bufferBindInfos["Info"].pipeId = 0;
}

void PBRArtifactExample::Loop()
{
	uint32_t i = 0;;


	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	CaptureNum(3)


	Camera camera(glm::vec3(0,0,-3),glm::vec3(0,0,0),glm::vec3(0,1,0));
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


	

	//绑定uniform buffer
	BindBuffer("TransformBuffer");
	bufferBindInfos["TransformBuffer"].pipeId = 1;
	BindBuffer("TransformBuffer");

	BindBuffer("Info");

	//绑定纹理
	BindTexture("baseColorTexture");
	BindTexture("normalTexture");
	BindTexture("roughnessTexture");
	
	textureBindInfos["bgEnvTexture"].pipeId = 0;
	textureBindInfos["bgEnvTexture"].binding = 6;
	BindTexture("bgEnvTexture");
	textureBindInfos["bgEnvTexture"].pipeId = 1;
	textureBindInfos["bgEnvTexture"].binding = 1;
	BindTexture("bgEnvTexture");



	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();

		//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
		buffer.view = camera.GetView();
		FillBuffer(buffers["TransformBuffer"], 0, sizeof(Buffer), (const char*)&buffer);
		FillBuffer(buffers["Info"], 0, sizeof(glm::vec3), (const char*)&camera.GetPos());

		CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理
		//确保presentFence在创建时已经触发
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		
		CmdOpsDrawGeom(graphicCommandList);
		
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachments[0].attachmentImage, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsCopyWholeImageToImage(graphicCommandList, renderTargets.colorAttachments[0].attachmentImage, swapchainImages[nexIndex]);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachments[0].attachmentImage, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro
		Present(nexIndex, { finishCopyTargetToSwapchain });



	}
	WaitIdle();
}


void PBRArtifactExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
