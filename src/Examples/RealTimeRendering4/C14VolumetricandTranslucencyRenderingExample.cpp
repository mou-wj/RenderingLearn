#include "C14VolumetricandTranslucencyRenderingExample.h"
#include "glm/mat4x4.hpp"

void C14VolumetricandTranslucencyRenderingExample::InitSubPassInfo()
{

	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C14VolumetricandTranslucencyRenderingExample.vert";
	drawSceenCodePath.geometryShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C14VolumetricandTranslucencyRenderingExample.geom";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C14VolumetricandTranslucencyRenderingExample.frag";



	
	renderPassInfos.resize(1);
	renderPassInfos[0].InitDefaultRenderPassInfo(drawSceenCodePath,windowWidth,windowHeight);
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

}

void C14VolumetricandTranslucencyRenderingExample::InitResourceInfos()
{

	geoms.resize(1);
	geoms[0].useIndexBuffers = false;
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/plane.obj",geoms[0]);

	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };


	TextureDataSource emptyDataSource;
	emptyDataSource.width = 32;
	emptyDataSource.height = 32;
	emptyDataSource.picturePath = "";

	bufferBindInfos["SimpleSceenExampleBuffer"].size = sizeof(glm::mat4) * 3;
	bufferBindInfos["SimpleSceenExampleBuffer"].binding = 0;
	bufferBindInfos["SimpleSceenExampleBuffer"].pipeId = 0;
	bufferBindInfos["SimpleSceenExampleBuffer"].passId = 0;

	//bufferBindInfos["Info"].size = sizeof(glm::vec4) * 3;
	//bufferBindInfos["Info"].binding = 3;
	//bufferBindInfos["Info"].pipeId = 0;
	//bufferBindInfos["Info"].passId = 1;
}

void C14VolumetricandTranslucencyRenderingExample::Loop()
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

	

	//��uniform buffer
	BindBuffer("SimpleSceenExampleBuffer");
	//BindBuffer("Info");
	

	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		//ȷ��presentFence�ڴ���ʱ�Ѿ�����
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
		buffer.view = camera.GetView();
		FillBuffer(buffers["SimpleSceenExampleBuffer"], 0, sizeof(Buffer), (const char*)&buffer);

		CmdListWaitFinish(graphicCommandList);//��Ϊ�ǵ��̣߳����Եȴ�������ɺ��ٴ���

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


void C14VolumetricandTranslucencyRenderingExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
