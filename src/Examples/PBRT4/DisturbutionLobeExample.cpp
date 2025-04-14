#include "DisturbutionLobeExample.h"
#include "glm/mat4x4.hpp"

void DisturbutionLobeExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SkyBoxExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SkyBoxExample.frag";
	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/PBRT4/DisturbutionLobeExample.vert";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/PBRT4/DisturbutionLobeExample.frag";

	renderPassInfos.resize(1);

	auto& subpassInfo = renderPassInfos[0].subpassInfo;
	auto& renderTargets = renderPassInfos[0].renderTargets;

	//InitDefaultGraphicSubpassInfo();
	//����subpass һ��������պУ�һ�����Ƴ���
	subpassInfo.subpassDescs.resize(2);
	//������ɫ��·��
	subpassInfo.subpassDescs[0].pipelinesShaderCodePaths = shaderCodePath;
	subpassInfo.subpassDescs[1].pipelinesShaderCodePaths = drawSceenCodePath;
	//��ʼ������״̬
	subpassInfo.subpassDescs[0].subpassPipelineStates.Init(windowWidth, windowWidth);
	subpassInfo.subpassDescs[1].subpassPipelineStates.Init(windowWidth, windowHeight);

	//�����޳�
	subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
	subpassInfo.subpassDescs[1].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;


	auto& subpassDesc1 = subpassInfo.subpassDescs[0];
	subpassDesc1.subpassDescription.flags = 0;
	subpassDesc1.subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc1.subpassDescription.inputAttachmentCount = 0;
	subpassDesc1.subpassDescription.pInputAttachments = nullptr;
	subpassDesc1.subpassDescription.colorAttachmentCount = 1;
	subpassDesc1.subpassDescription.pColorAttachments = &renderTargets.colorRefs[0];
	subpassDesc1.subpassDescription.pResolveAttachments = nullptr;
	subpassDesc1.subpassDescription.pDepthStencilAttachment = &renderTargets.depthRef;
	subpassDesc1.subpassDescription.preserveAttachmentCount = 0;
	subpassDesc1.subpassDescription.pPreserveAttachments = nullptr;

	auto& subpassDesc2 = subpassInfo.subpassDescs[1];
	subpassDesc2.subpassDescription.flags = 0;
	subpassDesc2.subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc2.subpassDescription.inputAttachmentCount = 0;
	subpassDesc2.subpassDescription.pInputAttachments = nullptr;
	subpassDesc2.subpassDescription.colorAttachmentCount = 1;
	subpassDesc2.subpassDescription.pColorAttachments = &renderTargets.colorRefs[0];
	subpassDesc2.subpassDescription.pResolveAttachments = nullptr;
	subpassDesc2.subpassDescription.pDepthStencilAttachment = &renderTargets.depthRef;
	subpassDesc2.subpassDescription.preserveAttachmentCount = 0;
	subpassDesc2.subpassDescription.pPreserveAttachments = nullptr;


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

void DisturbutionLobeExample::InitResourceInfos()
{

	geoms.resize(2);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geoms[0]);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/sphere.obj", geoms[1]);

	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	renderPassInfos[0].subpassDrawGeoInfos[1] = { 1 };

	std::vector<std::string> skyboxImages = {
	std::string(PROJECT_DIR) + "/resources/pic/testskybox/+x.jpg",//+x
	std::string(PROJECT_DIR) + "/resources/pic/testskybox/-x.jpg",//-x
	std::string(PROJECT_DIR) + "/resources/pic/testskybox/+y.jpg",//+y
	std::string(PROJECT_DIR) + "/resources/pic/testskybox/-y.jpg",//-y
	std::string(PROJECT_DIR) + "/resources/pic/testskybox/+z.jpg",//+z
	std::string(PROJECT_DIR) + "/resources/pic/testskybox/-z.jpg"//-z
	};
	textureBindInfos["skybox"] = TextureBindInfo(skyboxImages);
	textureBindInfos["skybox"].binding = 1;
	textureBindInfos["skybox"].viewType = VK_IMAGE_VIEW_TYPE_CUBE;

	bufferBindInfos["Buffer"].size = sizeof(glm::mat4) * 3;
	bufferBindInfos["Buffer"].binding = 0;

	bufferBindInfos["SimpleSceenExampleBuffer"].size = sizeof(glm::mat4) * 3 ;
	bufferBindInfos["SimpleSceenExampleBuffer"].binding = 0;
	bufferBindInfos["SimpleSceenExampleBuffer"].pipeId = 1;
	
	bufferBindInfos["type"].size = sizeof(uint32_t);
	bufferBindInfos["type"].binding = 1;
	bufferBindInfos["type"].pipeId = 1;
}

void DisturbutionLobeExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");


	Camera camera(glm::vec3(0,3,0),glm::vec3(0,0,0),glm::vec3(0,0,1));
	uint32_t viewType = 0;
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
	WindowEventHandler::SetEventCallBack(KEY_I_PRESS, [&viewType]() {
		std::cout << "����һ����������ΧΪ0��1��2" << std::endl;
		uint32_t tmp = 0;
		std::cin >> tmp;
		if (tmp >= 0 && tmp <= 2)
		{
			viewType = tmp;
		}
		
		}, "���I ����һ���������л���ʾʾ�������ͣ�0��ʾ��΢�������۵ķ�������ֲ������1��ʾΪ��΢�����brdf�����2���ֲ������������");




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



	FillBuffer(buffers["type"], 0, sizeof(uint32_t), (const char*)&viewType);

	FillBuffer(buffers["Buffer"], 0, sizeof(Buffer), (const char*)&buffer);


	auto drawFinished = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = {  };
	submitSyncInfo.waitStages = {  };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };


	//�������Լ�uniform buffer
	BindTexture("skybox");
	BindBuffer("Buffer");
	BindBuffer("SimpleSceenExampleBuffer");
	BindBuffer("type");


	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;

		//ȷ��presentFence�ڴ���ʱ�Ѿ�����


		//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
		buffer.view = camera.GetView();

		CmdListWaitFinish(graphicCommandList);//��Ϊ�ǵ��̣߳����Եȴ�������ɺ��ٴ���
		WindowEventHandler::ProcessEvent();
		FillBuffer(buffers["Buffer"], 0, sizeof(glm::mat4) * 3, (const char*)&buffer);
		FillBuffer(buffers["SimpleSceenExampleBuffer"], 0, sizeof(Buffer), (const char*)&buffer);
		FillBuffer(buffers["type"], 0, sizeof(uint32_t), (const char*)&viewType);

		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		
		CmdOpsDrawGeom(graphicCommandList);
		
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro
		PresentPassResult(drawFinished, 0, 0);



	}
	WaitIdle();
}


void DisturbutionLobeExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
