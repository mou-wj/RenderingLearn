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
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//д����ȸ���

	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//д����ȸ���
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
		////��Ϊ����ʹ�õ���glsl��texture�������������������ķ�ʽ�Ǹ���opengl��NDC������еģ�������opengl��NDC����+y����Ӧ�������ϵģ���vulkan��+y�����µģ�Ϊ������glsl��texture������������Է�ת+-y����Ӧ��ͼƬ�����Ƿ�ת+-yͼƬ����������Ȼ�������µߵ����⣬���Բ���ȡ��ת+-yͼƬ������������ͼƬ�ķ�ת��ȡ
		//std::string(PROJECT_DIR) + "/resources/pic/skybox/bottom.jpg",//+y
		//std::string(PROJECT_DIR) + "/resources/pic/skybox/top.jpg",//-y
		//std::string(PROJECT_DIR) + "/resources/pic/skybox/front.jpg",//+z
		//std::string(PROJECT_DIR) + "/resources/pic/skybox/back.jpg"//-z
		std::string(PROJECT_DIR) + "/resources/pic/skybox1/right.png",//+x
		std::string(PROJECT_DIR) + "/resources/pic/skybox1/left.png",//-x
		//��Ϊ����ʹ�õ���glsl��texture�������������������ķ�ʽ�Ǹ���opengl��NDC������еģ�������opengl��NDC����+y����Ӧ�������ϵģ���vulkan��+y�����µģ�Ϊ������glsl��texture������������Է�ת+-y����Ӧ��ͼƬ�����Ƿ�ת+-yͼƬ����������Ȼ�������µߵ����⣬���Բ���ȡ��ת+-yͼƬ������������ͼƬ�ķ�ת��ȡ
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




	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];


	

	//��uniform buffer
	BindBuffer("TransformBuffer");
	bufferBindInfos["TransformBuffer"].pipeId = 1;
	BindBuffer("TransformBuffer");

	BindBuffer("Info");

	//������
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

		CmdListWaitFinish(graphicCommandList);//��Ϊ�ǵ��̣߳����Եȴ�������ɺ��ٴ���
		//ȷ��presentFence�ڴ���ʱ�Ѿ�����
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
