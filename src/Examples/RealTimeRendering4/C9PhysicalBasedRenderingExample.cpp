#include "C9PhysicalBasedRenderingExample.h"
#include "glm/mat4x4.hpp"

void C9PhysicalBasedRenderingExample::InitSubPassInfo()
{
	ShaderCodePaths preComputeCodePath;
	preComputeCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C9PhysicalBasedRenderingExamplePrecompute.vert";
	preComputeCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C9PhysicalBasedRenderingExamplePrecompute.frag";

	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C9PhysicalBasedRenderingExample.vert";
	drawSceenCodePath.geometryShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C9PhysicalBasedRenderingExample.geom";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C9PhysicalBasedRenderingExample.frag";



	
	renderPassInfos.resize(2);
	renderPassInfos[0].InitDefaultRenderPassInfo(preComputeCodePath, 32, 32);
	renderPassInfos[0].renderTargets.colorAttachment.attachmentDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	renderPassInfos[1].InitDefaultRenderPassInfo(drawSceenCodePath,windowWidth,windowHeight);


}

void C9PhysicalBasedRenderingExample::InitResourceInfos()
{

	geoms.resize(2);
	geoms[0].InitAsScreenFillRect();
	geoms[1].useIndexBuffers = false;
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/plane.obj",geoms[1]);

	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	renderPassInfos[1].subpassDrawGeoInfos[0] = { 1 };


	TextureDataSource emptyDataSource;
	emptyDataSource.width = 32;
	emptyDataSource.height = 32;
	emptyDataSource.picturePath = "";
	textureBindInfos["PreComputeMultiReflect"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["PreComputeMultiReflect"].binding = 1;
	textureBindInfos["PreComputeMultiReflect"].viewType = VK_IMAGE_VIEW_TYPE_2D;
	textureBindInfos["PreComputeMultiReflect"].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	textureBindInfos["PreComputeMultiReflect"].passId = 1;



	bufferBindInfos["SimpleSceenExampleBuffer"].size = sizeof(glm::mat4) * 3;
	bufferBindInfos["SimpleSceenExampleBuffer"].binding = 0;
	bufferBindInfos["SimpleSceenExampleBuffer"].pipeId = 0;
	bufferBindInfos["SimpleSceenExampleBuffer"].passId = 1;

	bufferBindInfos["Info"].size = sizeof(glm::vec4) * 3;
	bufferBindInfos["Info"].binding = 3;
	bufferBindInfos["Info"].pipeId = 0;
	bufferBindInfos["Info"].passId = 1;
}

void C9PhysicalBasedRenderingExample::Loop()
{
	uint32_t i = 0;;
	uint32_t exampleType = 0;


	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");


	Camera camera(glm::vec3(0,-1,-12),glm::vec3(0,0,0),glm::vec3(0,1,0));
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
		std::cout << "����һ������: ��Χ[0,5]" << std::endl;
		std::cin >> tmp;
		if (exampleType > 5)
		{
			std::cout << "����Ƿ�" << std::endl;
			return;
		}
		exampleType = tmp;
		; }, "���I ����һ���������л�����ģ�͡�0��ʾGGX�ľ��淴��ʾ��; 1��ʾ�⻬�α���ɢ��ģ��;2��ʾ�ֲڴα���ɢ���ʿ��ģ��;3��ʾ�ֲڴα���ɢ��OrenNayarģ��;4��ʾ���鲼��ģ��;5��ʾ΢���沼��ģ�� ");




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


	SubmitSynchronizationInfo preComputePassSubmitSyncInfo;
	preComputePassSubmitSyncInfo.waitSemaphores = {  };
	preComputePassSubmitSyncInfo.waitStages = {  };
	preComputePassSubmitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	//Ԥ����
	CmdListWaitFinish(graphicCommandList);//��Ϊ�ǵ��̣߳����Եȴ�������ɺ��ٴ���
	CaptureBeginMacro
	CmdListReset(graphicCommandList);

	CmdListRecordBegin(graphicCommandList);
	//����depth pass
	CmdOpsDrawGeom(graphicCommandList);
	auto& colorTargetImage = renderPassInfos[0].renderTargets.colorAttachment.attachmentImage;

	auto colorOldLayout = colorTargetImage.currentLayout;
	auto precomputeTextureOldLayout = textures["PreComputeMultiReflect"].image.currentLayout;

	CmdOpsImageMemoryBarrer(graphicCommandList, colorTargetImage, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);
	CmdOpsImageMemoryBarrer(graphicCommandList, textures["PreComputeMultiReflect"].image, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);

	//�������ͼ���������
	CmdOpsCopyImageToImage(graphicCommandList, colorTargetImage, 0, 0, textures["PreComputeMultiReflect"].image, 0, 0);

	CmdOpsImageMemoryBarrer(graphicCommandList, colorTargetImage, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, colorOldLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0);
	CmdOpsImageMemoryBarrer(graphicCommandList, textures["PreComputeMultiReflect"].image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE, precomputeTextureOldLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);

	CmdListRecordEnd(graphicCommandList);
	CmdListSubmit(graphicCommandList, preComputePassSubmitSyncInfo);

	CaptureEndMacro



	//��uniform buffer
	BindBuffer("SimpleSceenExampleBuffer");
	BindBuffer("Info");
	BindTexture("PreComputeMultiReflect");
	

	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	auto& renderTargets = renderPassInfos[1].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		//ȷ��presentFence�ڴ���ʱ�Ѿ�����
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
		buffer.view = camera.GetView();
		FillBuffer(buffers["SimpleSceenExampleBuffer"], 0, sizeof(Buffer), (const char*)&buffer);
		FillBuffer(buffers["Info"], sizeof(glm::vec4), sizeof(glm::vec3), (const char*)&camera.GetPos());
		FillBuffer(buffers["Info"], sizeof(glm::vec4) * 2, sizeof(uint32_t), (const char*)&exampleType);

		CmdListWaitFinish(graphicCommandList);//��Ϊ�ǵ��̣߳����Եȴ�������ɺ��ٴ���

		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		
		CmdOpsDrawGeom(graphicCommandList,1);
		
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


void C9PhysicalBasedRenderingExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
