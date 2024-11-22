#include "SkyBoxExample.h"
#include "glm/mat4x4.hpp"

void SkyBoxExample::InitSubPassInfo()
{
	InitDefaultGraphicSubpassInfo();


}

void SkyBoxExample::InitResourceInfos()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SkyBoxExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SkyBoxExample.frag";
	pipelinesShaderCodePaths = { shaderCodePath };
	geoms.resize(1);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geoms[0]);
	subpassDrawGeoInfos[0] = { 0 };

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


	//test
	std::vector<std::string> testskyboxImages = {
		std::string(PROJECT_DIR) + "/resources/pic/testskybox/+x.jpg",//+x
		std::string(PROJECT_DIR) + "/resources/pic/testskybox/-x.jpg",//-x
		std::string(PROJECT_DIR) + "/resources/pic/testskybox/+y.jpg",//+y
		std::string(PROJECT_DIR) + "/resources/pic/testskybox/-y.jpg",//-y
		std::string(PROJECT_DIR) + "/resources/pic/testskybox/+z.jpg",//+z
		std::string(PROJECT_DIR) + "/resources/pic/testskybox/-z.jpg"//-z
	};
	textureInfos["testskybox"] = TextureInfo(testskyboxImages);
	textureInfos["testskybox"].binding = 1;
	textureInfos["testskybox"].viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	


	uniformBufferInfos["Buffer"].size = sizeof(glm::mat4) * 3;
	uniformBufferInfos["Buffer"].binding = 0;
}

void SkyBoxExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	//std::vector<char> testImageData;

	//testImageData.resize(512 * 512 * 4);
	//for (uint32_t i = 0; i < 512; i++)
	//{
	//	for (uint32_t j = 0; j < 512; j++)
	//	{
	//		testImageData[(512 * i + j) * 4] = 255;
	//		testImageData[(512 * i + j) * 4 + 1] = 0;
	//		testImageData[(512 * i + j) * 4 + 2] = 0;
	//		testImageData[(512 * i + j) * 4 + 3] = 255;
	//	}


	//}
	Camera camera;

	WindowEventHandler::SetEventCallBack(KEY_W_PRESS, [&camera]() {camera.Move(MoveDirection::FORWARD); },"���w ���ǰ��");
	WindowEventHandler::SetEventCallBack(KEY_S_PRESS, [&camera]() {camera.Move(MoveDirection::BACK); }, "���s �������");
	WindowEventHandler::SetEventCallBack(KEY_A_PRESS, [&camera]() {camera.Move(MoveDirection::LEFT); }, "���a �������");
	WindowEventHandler::SetEventCallBack(KEY_D_PRESS, [&camera]() {camera.Move(MoveDirection::RIGHT); }, "���d �������");
	WindowEventHandler::SetEventCallBack(KEY_UP_PRESS, [&camera]() {
		//���Ͽ��൱�����е�������ת����z->y,��AROUND_X_NEGATIVE
		camera.Rotate(RotateAction::AROUND_X_NEGATIVE); },"���up ������Ͽ�");
	WindowEventHandler::SetEventCallBack(KEY_DOWN_PRESS, [&camera]() {camera.Rotate(RotateAction::AROUND_X_POSITIVE); }, "���down ������¿�");
	WindowEventHandler::SetEventCallBack(KEY_RIGHT_PRESS, [&camera]() {
		//���ҿ��൱�����е�������ת����x->z����AROUND_Y_POSITIVE
		camera.Rotate(RotateAction::AROUND_Y_POSITIVE);
		}, "���right ������ҿ�");
	WindowEventHandler::SetEventCallBack(KEY_LEFT_PRESS, [&camera]() {camera.Rotate(RotateAction::AROUND_Y_NEGATIVE); }, "���left �������");
	uint32_t curSkyBoxIndex = 0;
	WindowEventHandler::SetEventCallBack(KEY_I_PRESS, [&curSkyBoxIndex,this]() {
		if (curSkyBoxIndex == 0)
		{
			BindTexture("skybox");
		}
		else {
			BindTexture("testskybox");
		}
		curSkyBoxIndex = (++curSkyBoxIndex) % 2;
		}, "���i �л���պ�����");


	//Texture testTexture = Create2DTexture(512, 512, testImageData.data());
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
	BindUniformBuffer("Buffer");
	BindTexture("testskybox");

	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;


		//ȷ��presentFence�ڴ���ʱ�Ѿ�����
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
		buffer.view = camera.GetView();
		FillBuffer(uniformBuffers["Buffer"], 0, sizeof(Buffer), (const char*)&buffer);

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

}


void SkyBoxExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
