#include "C20EfficientShadingExample.h"
#include "glm/mat4x4.hpp"

void C20EfficientShadingExample::InitSubPassInfo()
{
	ShaderCodePaths gBufferPath;
	gBufferPath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C20EfficientShadingExampleGBuffer.vert";
	gBufferPath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C20EfficientShadingExampleGBuffer.frag";
	
	ShaderCodePaths decalCodePath;
	decalCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C20EfficientShadingExampleDecal.vert";
	decalCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C20EfficientShadingExampleDecal.frag";
	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C20EfficientShadingExample.vert";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C20EfficientShadingExample.frag";

	
	renderPassInfos.resize(1);
	renderPassInfos[0].InitDefaultRenderPassInfo({ gBufferPath,decalCodePath,drawSceenCodePath }, windowWidth, windowHeight);

	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_FALSE;//��д����ȸ���

	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_FALSE;//��д����ȸ���
	

	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[2].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//д����ȸ���

}

void C20EfficientShadingExample::InitResourceInfos()
{

	geoms.resize(2);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/ScaledCylinder.obj",geoms[0]);
	geoms[1].InitAsScreenFillRect();
	//LoadObj(std::string(PROJECT_DIR) + "/resources/obj/moved_cube.obj", geoms[1]);
	
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	renderPassInfos[0].subpassDrawGeoInfos[1] = { 0 };
	renderPassInfos[0].subpassDrawGeoInfos[2] = { 1 };

	bufferBindInfos["Transform"].size = sizeof(glm::mat4);
	bufferBindInfos["Transform"].binding = 0;

	bufferBindInfos["TransformParallel"].size = sizeof(glm::mat4);
	bufferBindInfos["TransformParallel"].binding = 0;
	bufferBindInfos["TransformParallel"].pipeId = 1;



	TextureDataSource emptyDataSource;
	emptyDataSource.width = windowWidth;
	emptyDataSource.height = windowHeight;
	emptyDataSource.picturePath = "";
	textureBindInfos["GBuffer1"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["GBuffer1"].binding = 2;
	textureBindInfos["GBuffer1"].viewType = VK_IMAGE_VIEW_TYPE_2D;
	textureBindInfos["GBuffer1"].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	textureBindInfos["GBuffer1"].pipeId = 0;
	textureBindInfos["GBuffer1"].usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;


	TextureDataSource decalTextureDataSource;
	decalTextureDataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/OIP.jpg";
	textureBindInfos["decalTexture"].textureDataSources.push_back(decalTextureDataSource);
	textureBindInfos["decalTexture"].binding = 4;
	textureBindInfos["decalTexture"].pipeId = 1;


	
}

void C20EfficientShadingExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");


	Camera camera(glm::vec3(0,0,-5),glm::vec3(0,0,0),glm::vec3(0,1,0));
	//��camera��Ӧ�����Ļص�����
	WindowEventHandler::SetEventCallBack(KEY_W_PRESS, [&camera]() {camera.Move(MoveDirection::FORWARD); }, "���w ���ǰ��");
	WindowEventHandler::SetEventCallBack(KEY_S_PRESS, [&camera]() {camera.Move(MoveDirection::BACK); }, "���s �������");
	WindowEventHandler::SetEventCallBack(KEY_A_PRESS, [&camera]() {camera.Move(MoveDirection::LEFT); }, "���a �������");
	WindowEventHandler::SetEventCallBack(KEY_D_PRESS, [&camera]() {camera.Move(MoveDirection::RIGHT); }, "���d �������");
	WindowEventHandler::SetEventCallBack(KEY_UP_PRESS, [&camera]() {
		//���Ͽ��൱�����е�������ת����z->y,��AROUND_X_NEGATIVE
		camera.Rotate(RotateAction::AROUND_X_NEGATIVE); }, "���up ������Ͽ�");
	WindowEventHandler::SetEventCallBack(KEY_DOWN_PRESS, [&camera]() {camera.Rotate(RotateAction::AROUND_X_POSITIVE); }, "���down ������¿�");
	WindowEventHandler::SetEventCallBack(KEY_RIGHT_PRESS, [&camera]() {
		//���ҿ��൱�����е�������ת����x->z����AROUND_Y_POSITIVE
		camera.Rotate(RotateAction::AROUND_Y_POSITIVE);
		}, "���right ������ҿ�");
	WindowEventHandler::SetEventCallBack(KEY_LEFT_PRESS, [&camera]() {camera.Rotate(RotateAction::AROUND_Y_NEGATIVE); }, "���left �������");




	struct Buffer {
		glm::mat4 world;
		glm::mat4 view;
		glm::mat4 proj;
	} buffer;
	buffer.world = glm::mat4(1.0);
	buffer.view = camera.GetView();
	buffer.proj = camera.GetProj();

	glm::mat4 transform;

	Camera camera2(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
	camera2.SetProjectType(false);
	camera2.SetParallelProjectParams(-1, -0.5, 2, 2);
	transform = camera2.GetProj() * camera2.GetView();
	FillBuffer(buffers["TransformParallel"], 0, sizeof(glm::mat4), (const char*)&transform);


	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	bufferBindInfos["Transform"].pipeId = 0;
	BindBuffer("Transform");

	bufferBindInfos["Transform"].pipeId = 1;
	bufferBindInfos["Transform"].binding = 3;
	BindBuffer("Transform");

	BindBuffer("TransformParallel");

	//��GBuffer
	BindTexture("GBuffer1");
	textureBindInfos["GBuffer1"].pipeId = 1;
	BindTexture("GBuffer1");
	textureBindInfos["GBuffer1"].pipeId = 2;
	BindTexture("GBuffer1");

	//��decal texture
	BindTexture("decalTexture");



	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;

		//ȷ��presentFence�ڴ���ʱ�Ѿ�����
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);


		CmdListWaitFinish(graphicCommandList);//��Ϊ�ǵ��̣߳����Եȴ�������ɺ��ٴ���
		WindowEventHandler::ProcessEvent();
		

		transform = camera.GetProj() * camera.GetView();
		FillBuffer(buffers["Transform"], 0, sizeof(glm::mat4), (const char*)&transform);

		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		CmdOpsClearWholeColorImage(graphicCommandList, textures["GBuffer1"].image, VkClearColorValue{ 0,0,0,1 });


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


void C20EfficientShadingExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
