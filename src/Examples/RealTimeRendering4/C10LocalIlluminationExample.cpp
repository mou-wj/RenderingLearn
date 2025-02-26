#include "C10LocalIlluminationExample.h"
#include "glm/mat4x4.hpp"

void C10LocalIlluminationExample::InitSubPassInfo()
{

	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C10LocalIlluminationExample.vert";
	drawSceenCodePath.geometryShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C10LocalIlluminationExample.geom";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C10LocalIlluminationExample.frag";



	
	renderPassInfos.resize(1);
	renderPassInfos[0].InitDefaultRenderPassInfo(drawSceenCodePath,windowWidth,windowHeight);
	///renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_NONE;

}

void C10LocalIlluminationExample::InitResourceInfos()
{

	geoms.resize(2);
	geoms[0].useIndexBuffers = false;
	geoms[1].useIndexBuffers = true;
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/plane.obj",geoms[0]);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/sphere.obj", geoms[1]);
	
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 1 };


	TextureDataSource emptyDataSource;

	emptyDataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/DaySkyHDRI046A_1K-TONEMAPPED.jpg";
	textureBindInfos["EnvTexture"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["EnvTexture"].binding = 2;
	textureBindInfos["EnvTexture"].viewType = VK_IMAGE_VIEW_TYPE_2D;
	textureBindInfos["EnvTexture"].buildMipmap = true;


	bufferBindInfos["SimpleSceenExampleBuffer"].size = sizeof(glm::mat4) * 3;
	bufferBindInfos["SimpleSceenExampleBuffer"].binding = 0;
	bufferBindInfos["SimpleSceenExampleBuffer"].pipeId = 0;


	bufferBindInfos["Info"].size = sizeof(glm::vec4) * 3;
	bufferBindInfos["Info"].binding = 3;
	bufferBindInfos["Info"].pipeId = 0;

}

void C10LocalIlluminationExample::Loop()
{
	uint32_t i = 0;;
	uint32_t exampleType = 0;


	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");


	Camera camera(glm::vec3(0,-3,-3),glm::vec3(0,0,0),glm::vec3(0,1,0));
	//Camera camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
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
		std::cout << "����һ������: ��Χ[0,1]" << std::endl;
		std::cin >> tmp;
		if (exampleType > 2)
		{
			std::cout << "����Ƿ�" << std::endl;
			return;
		}
		if (exampleType == 1)
		{
			renderPassInfos[0].subpassDrawGeoInfos[0] = { 1 };
		}
		else {
			renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };

		}
		exampleType = tmp;
		; }, "���I ����һ���������л�ʵ����0��ʾһ���򵥾������Դ��blinn phong����; 1��ʾʹ����������ӳ��Ļ�����ͼ");




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
	BindBuffer("SimpleSceenExampleBuffer");
	BindBuffer("Info");
	BindTexture("EnvTexture");

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
		FillBuffer(buffers["SimpleSceenExampleBuffer"], 0, sizeof(Buffer), (const char*)&buffer);
		FillBuffer(buffers["Info"], sizeof(glm::vec4), sizeof(glm::vec3), (const char*)&camera.GetPos());
		FillBuffer(buffers["Info"], sizeof(glm::vec4) * 2, sizeof(uint32_t), (const char*)&exampleType);

		CmdListWaitFinish(graphicCommandList);//��Ϊ�ǵ��̣߳����Եȴ�������ɺ��ٴ���
		//ȷ��presentFence�ڴ���ʱ�Ѿ�����
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		
		CmdOpsDrawGeom(graphicCommandList,0);
		
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


void C10LocalIlluminationExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
