#include "SimpleSceenExample.h"
#include "glm/mat4x4.hpp"

void SimpleSceenExample::InitSubPassInfo()
{
	//InitDefaultGraphicSubpassInfo();
	//����subpass һ��������պУ�һ�����Ƴ���
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
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/testcube.obj", geoms[1]);

	subpassDrawGeoInfos[0] = { 0 };
	subpassDrawGeoInfos[1] = { 1 };

	std::vector<std::string> skyboxImages = {
	std::string(PROJECT_DIR) + "/resources/pic/skybox/right.jpg",//+x
	std::string(PROJECT_DIR) + "/resources/pic/skybox/left.jpg",//-x
	//��Ϊ����ʹ�õ���glsl��texture�������������������ķ�ʽ�Ǹ���opengl��NDC������еģ�������opengl��NDC����+y����Ӧ�������ϵģ���vulkan��+y�����µģ�Ϊ������glsl��texture������������Է�ת+-y����Ӧ��ͼƬ�����Ƿ�ת+-yͼƬ����������Ȼ�������µߵ����⣬���Բ���ȡ��ת+-yͼƬ������������ͼƬ�ķ�ת��ȡ
	std::string(PROJECT_DIR) + "/resources/pic/skybox/bottom.jpg",//+y
	std::string(PROJECT_DIR) + "/resources/pic/skybox/top.jpg",//-y
	std::string(PROJECT_DIR) + "/resources/pic/skybox/front.jpg",//+z
	std::string(PROJECT_DIR) + "/resources/pic/skybox/back.jpg"//-z
	};
	textureInfos["skybox"] = TextureInfo(skyboxImages);
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


	Camera camera(glm::vec3(0,0,-12),glm::vec3(0,0,-11),glm::vec3(0,1,0));
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


	//�������Լ�uniform buffer
	BindTexture("skybox");
	BindUniformBuffer("Buffer");
	BindUniformBuffer("SimpleSceenExampleBuffer");


	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;

		//ȷ��presentFence�ڴ���ʱ�Ѿ�����
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
		buffer.view = camera.GetView();
		FillBuffer(uniformBuffers["Buffer"], 0, sizeof(Buffer), (const char*)&buffer);
		FillBuffer(uniformBuffers["SimpleSceenExampleBuffer"], 0, sizeof(Buffer), (const char*)&buffer);

		CmdListWaitFinish(graphicCommandList);//��Ϊ�ǵ��̣߳����Եȴ�������ɺ��ٴ���
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


void SimpleSceenExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
