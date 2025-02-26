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
	//renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].renderTargets.colorAttachments[0].attachmentDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	renderPassInfos[0].renderTargets.colorAttachments[0].attachmentImage.numLayer = 1;
	renderPassInfos[0].renderTargets.colorAttachments[0].clearValue = VkClearValue{ 1.0,100.0,10100.0,1.0 };
	
	renderPassInfos[1].InitDefaultRenderPassInfo(drawSceenCodePath, windowWidth, windowHeight);
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[1].renderTargets.colorAttachments[0].clearValue = VkClearValue{ 0.2,0.0,0.0,1.0 };
	
}

void C7ShadowsExample::InitResourceInfos()
{

	geoms.resize(1);
	geoms[0].useIndexBuffers = false;
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/simple_sceen.obj",geoms[0]);

	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	renderPassInfos[1].subpassDrawGeoInfos[0] = { 0 };


	bufferBindInfos["SimpleSceenExampleBuffer"].size = sizeof(glm::mat4) * 3;
	bufferBindInfos["SimpleSceenExampleBuffer"].binding = 0;
	bufferBindInfos["SimpleSceenExampleBuffer"].pipeId = 0;

	bufferBindInfos["SceenInfo"].size = sizeof(glm::vec4) * 2 + sizeof(glm::mat4) * 6 + sizeof(uint32_t);
	bufferBindInfos["SceenInfo"].binding = 2;
	bufferBindInfos["SceenInfo"].pipeId = 0;
	bufferBindInfos["SceenInfo"].passId = 1;

	//һ��������Դ��Ӱ��ͼ
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
	textureBindInfos["pointShadowMap"].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	textureBindInfos["pointShadowMap"].passId = 1;


	textureBindInfos["pointShadowArrayMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["pointShadowArrayMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["pointShadowArrayMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["pointShadowArrayMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["pointShadowArrayMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["pointShadowArrayMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["pointShadowArrayMap"].binding = 3;
	textureBindInfos["pointShadowArrayMap"].viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	textureBindInfos["pointShadowArrayMap"].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	textureBindInfos["pointShadowArrayMap"].passId = 1;
}

void C7ShadowsExample::Loop()
{
	uint32_t i = 0;;
	uint32_t exampleType = 0;


	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");


	Camera camera(glm::vec3(0,0,0),glm::vec3(0,0,1),glm::vec3(0,1,0));
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


	WindowEventHandler::SetEventCallBack(KEY_I_PRESS, [&]() {
		uint32_t tmp = 0;
		std::cout << "����һ������: ��Χ[0,3]" << std::endl;
		std::cin >> tmp;
		if (exampleType > 3)
		{
			std::cout << "����Ƿ�" << std::endl;
			return;
		}
		exampleType = tmp;
		FillBuffer(buffers["SceenInfo"], 32 + sizeof(glm::mat4) * 6, sizeof(uint32_t), (const char*)&exampleType);

		; }, "���I ����һ���������л�ʹ�õ���Ӱ��ͼ�㷨��0:��ʾֱ��ͨ����Ӱ��ͼ; 1:PCF �ٷֱ��˲�; 2: PCSS -PCF�Ľ��汾�����ݾ�����Ϣ�����˲��˴�С; 3:CVM ������Ӱ��ͼ ");



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


	//��uniform buffer
	BindBuffer("SimpleSceenExampleBuffer");
	BindBuffer("SceenInfo");

	//������
	BindTexture("pointShadowMap");
	BindTexture("pointShadowArrayMap");

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

	glm::vec3 lightPos = glm::vec3(-3,-6,0);
	//glm::vec3 lightPos = glm::vec3(0, 0, 0);
	//glm::vec3 lightPos = glm::vec3(0, 0, 0);
	CaptureNum(9);
	for (uint32_t i = 0; i < 6; i++)
	{
		//��ȡ��Ӱ��ͼ
		camera.SetCamera2(lightPos, cubeViewTarget[i], cubeViewDown[i]);
		buffer.view = camera.GetView();
		FillBuffer(buffers["SimpleSceenExampleBuffer"], 0, sizeof(Buffer), (const char*)&buffer);
		glm::mat4 tmpVP = buffer.proj * buffer.view;
		uint32_t offset = 32 + sizeof(glm::mat4) * i;
		FillBuffer(buffers["SceenInfo"], offset, sizeof(glm::mat4), (const char*)&tmpVP);

		CmdListWaitFinish(graphicCommandList);//��Ϊ�ǵ��̣߳����Եȴ�������ɺ��ٴ���
		CaptureBeginMacro
		CmdListReset(graphicCommandList);

		CmdListRecordBegin(graphicCommandList);
		//����depth pass
		CmdOpsDrawGeom(graphicCommandList);
		auto& colorTargetImage = renderPassInfos[0].renderTargets.colorAttachments[0].attachmentImage;

		auto colorOldLayout = colorTargetImage.currentLayout;
		auto depthTextureOldLayout = textures["pointShadowMap"].image.currentLayout;

		CmdOpsImageMemoryBarrer(graphicCommandList, colorTargetImage, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,0,0);
		CmdOpsImageMemoryBarrer(graphicCommandList, textures["pointShadowMap"].image, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, i, 0);
		
		//�������ͼ���������
		CmdOpsCopyImageToImage(graphicCommandList, colorTargetImage, 0, 0, textures["pointShadowMap"].image, i, 0);

		CmdOpsImageMemoryBarrer(graphicCommandList, colorTargetImage, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, colorOldLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0);
		CmdOpsImageMemoryBarrer(graphicCommandList, textures["pointShadowMap"].image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE, depthTextureOldLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,i,0);
	
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro

	}
	textures["pointShadowMap"].image.CopyToOther(textures["pointShadowArrayMap"].image, 0, 0);
	textures["pointShadowMap"].image.CopyToOther(textures["pointShadowArrayMap"].image, 1, 0);
	textures["pointShadowMap"].image.CopyToOther(textures["pointShadowArrayMap"].image, 2, 0);
	textures["pointShadowMap"].image.CopyToOther(textures["pointShadowArrayMap"].image, 3, 0);
	textures["pointShadowMap"].image.CopyToOther(textures["pointShadowArrayMap"].image, 4, 0);
	textures["pointShadowMap"].image.CopyToOther(textures["pointShadowArrayMap"].image, 5, 0);
	//textures["pointShadowMap"].image.WriteToJpgFloat("depthMap0.jpg", 0, 0);
	//textures["pointShadowMap"].image.WriteToJpgFloat("depthMap1.jpg", 1, 0);
	//textures["pointShadowMap"].image.WriteToJpgFloat("depthMap2.jpg", 2, 0);
	//textures["pointShadowMap"].image.WriteToJpgFloat("depthMap3.jpg", 3, 0);
	//textures["pointShadowMap"].image.WriteToJpgFloat("depthMap4.jpg", 4, 0);
	//textures["pointShadowMap"].image.WriteToJpgFloat("depthMap5.jpg", 5, 0);
	////��תY
	textures["pointShadowMap"].image.FlipY();
	//textures["pointShadowMap"].image.WriteToJpgFloat("depthMap0Flip.jpg", 0, 0);
	//textures["pointShadowMap"].image.WriteToJpgFloat("depthMap1Flip.jpg", 1, 0);
	//textures["pointShadowMap"].image.WriteToJpgFloat("depthMap2Flip.jpg", 2, 0);
	//textures["pointShadowMap"].image.WriteToJpgFloat("depthMap3Flip.jpg", 3, 0);
	//textures["pointShadowMap"].image.WriteToJpgFloat("depthMap4Flip.jpg", 4, 0);
	//textures["pointShadowMap"].image.WriteToJpgFloat("depthMap5Flip.jpg", 5, 0);


	//������Ӱ
	auto& renderTargets = renderPassInfos[1].renderTargets;

	camera.SetCamera(glm::vec3(0,0,-6), glm::vec3(0,0,0), glm::vec3(0,1,0));
	//camera.SetCamera2(lightPos, cubeViewTarget[0], cubeViewDown[0]);
	bufferBindInfos["SimpleSceenExampleBuffer"].passId = 1;
	BindBuffer("SimpleSceenExampleBuffer");

	FillBuffer(buffers["SceenInfo"], 0, sizeof(glm::vec3), (const char*)&lightPos);
	FillBuffer(buffers["SceenInfo"], 16, sizeof(glm::vec3), (const char*)&camera.GetPos());
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;


		//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
		buffer.view = camera.GetView();
		FillBuffer(buffers["SimpleSceenExampleBuffer"], 0, sizeof(Buffer), (const char*)&buffer);

		CmdListWaitFinish(graphicCommandList);//��Ϊ�ǵ��̣߳����Եȴ�������ɺ��ٴ���
		//ȷ��presentFence�ڴ���ʱ�Ѿ�����
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);


		WindowEventHandler::ProcessEvent();
		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		
		CmdOpsDrawGeom(graphicCommandList,1);
		
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


void C7ShadowsExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
