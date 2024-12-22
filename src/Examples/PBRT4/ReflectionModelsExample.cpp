#include "ReflectionModelsExample.h"
#include "glm/mat4x4.hpp"

void ReflectionModelsExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SkyBoxExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SkyBoxExample.frag";
	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/PBRT4/ReflectionModelsExample.vert";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/PBRT4/ReflectionModelsExample.frag";



	//InitDefaultGraphicSubpassInfo();
	//两个subpass 一个绘制天空盒，一个绘制场景
	subpassInfo.subpassDescs.resize(2);
	//设置着色器路径
	subpassInfo.subpassDescs[0].pipelinesShaderCodePaths = shaderCodePath;
	subpassInfo.subpassDescs[1].pipelinesShaderCodePaths = drawSceenCodePath;
	//初始化管线状态
	subpassInfo.subpassDescs[0].subpassPipelineStates.Init(windowWidth, windowWidth);
	subpassInfo.subpassDescs[1].subpassPipelineStates.Init(windowWidth, windowHeight);

	//开启剔除
	subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
	subpassInfo.subpassDescs[1].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;


	auto& subpassDesc1 = subpassInfo.subpassDescs[0];
	subpassDesc1.subpassDescription.flags = 0;
	subpassDesc1.subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc1.subpassDescription.inputAttachmentCount = 0;
	subpassDesc1.subpassDescription.pInputAttachments = nullptr;
	subpassDesc1.subpassDescription.colorAttachmentCount = 1;
	subpassDesc1.subpassDescription.pColorAttachments = &renderTargets.colorRef;
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
	subpassDesc2.subpassDescription.pColorAttachments = &renderTargets.colorRef;
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

void ReflectionModelsExample::InitResourceInfos()
{

	geoms.resize(3);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geoms[0]);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/plane.obj", geoms[1]);
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cylinder.obj", geoms[2]);

	subpassDrawGeoInfos[0] = { 0 };
	subpassDrawGeoInfos[1] = { 1 };

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
	textureBindInfos["skybox"] = TextureBindInfo(skyboxImages);
	textureBindInfos["skybox"].binding = 1;
	textureBindInfos["skybox"].viewType = VK_IMAGE_VIEW_TYPE_CUBE;




	bufferBindInfos["Buffer"].size = sizeof(glm::mat4) * 3;
	bufferBindInfos["Buffer"].binding = 0;

	bufferBindInfos["SceenInfo"].size = sizeof(glm::vec3);
	bufferBindInfos["SceenInfo"].binding = 2;
	bufferBindInfos["SceenInfo"].pipeId = 1;
	
}

void ReflectionModelsExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");


	Camera camera(glm::vec3(0,-1,-4),glm::vec3(0,0,0),glm::vec3(0,1,0));
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
	//ShowMat(buffer.view);
	//ShowMat(buffer.proj);
	//ShowVec(buffer.proj* buffer.view* glm::vec4(1, 1, 1, 1));

	FillBuffer(buffers["Buffer"], 0, sizeof(Buffer), (const char*)&buffer);
	FillBuffer(buffers["SceenInfo"], 0, sizeof(glm::vec3), (const char*)&(camera.GetPos()));

	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };


	//绑定纹理以及uniform buffer
	BindTexture("skybox");
	textureBindInfos["skybox"].pipeId = 1;
	BindTexture("skybox");


	BindBuffer("Buffer");
	bufferBindInfos["Buffer"].pipeId = 1;
	BindBuffer("Buffer");

	BindBuffer("SceenInfo");


	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;

		//确保presentFence在创建时已经触发
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
		buffer.view = camera.GetView();
		FillBuffer(buffers["Buffer"], 0, sizeof(Buffer), (const char*)&buffer);
		FillBuffer(buffers["SceenInfo"], 0, sizeof(glm::vec3), (const char*)&(camera.GetPos()));


		CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理
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


void ReflectionModelsExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
