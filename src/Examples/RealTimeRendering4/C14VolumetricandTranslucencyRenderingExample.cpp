#include "C14VolumetricandTranslucencyRenderingExample.h"
#include "glm/mat4x4.hpp"

#include <random>

void C14VolumetricandTranslucencyRenderingExample::InitSubPassInfo()
{

	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C14VolumetricandTranslucencyRenderingExample.vert";
	//drawSceenCodePath.geometryShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C14VolumetricandTranslucencyRenderingExample.geom";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C14VolumetricandTranslucencyRenderingExample.frag";

	ShaderCodePaths bgCodePath;
	bgCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C14VolumetricandTranslucencyRenderingExampleBackground.vert";
	//drawSceenCodePath.geometryShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C14VolumetricandTranslucencyRenderingExample.geom";
	bgCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C14VolumetricandTranslucencyRenderingExampleBackground.frag";

	
	renderPassInfos.resize(1);
	renderPassInfos[0].InitDefaultRenderPassInfo({drawSceenCodePath,bgCodePath}, windowWidth, windowHeight);
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	//添加一个subpass绘制背景
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_NONE;
	renderPassInfos[0].subpassInfo.subpassDescs[1].subpassPipelineStates.rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;



}

void C14VolumetricandTranslucencyRenderingExample::InitResourceInfos()
{

	geoms.resize(1);
	geoms[0].useIndexBuffers = false;
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geoms[0]);
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	renderPassInfos[0].subpassDrawGeoInfos[1] = { 0 };
	// 创建随机数生成器，使用随机设备作为种子
	std::random_device rd;
	std::mt19937 gen(rd());

	// 设置均匀分布，范围是 [0.0, 1.0)
	std::uniform_real_distribution<> dis(0.0, 0.2);

	std::uniform_real_distribution<> dis2(0.5, 2.0);

	float  tmp[16 * 16];
	TextureDataSource emptyDataSource;
	emptyDataSource.width = 16;
	emptyDataSource.height = 16; emptyDataSource.imagePixelDatas.resize(16*16*sizeof(float));
	for (uint32_t i = 0; i < 16; i++) {
		
		for (uint32_t j = 0; j < 16 * 16; j++)
		{
			// 生成一个随机数
			float random_value = dis(gen);
			tmp[j] = random_value;
		}


		std::memcpy(emptyDataSource.imagePixelDatas.data(), tmp, 16 * 16 * sizeof(float));
		textureBindInfos["SigmaT_Texture"].textureDataSources.push_back(emptyDataSource);
	}

	for (uint32_t i = 0; i < 16; i++) {

		for (uint32_t j = 0; j < 16 * 16; j++)
		{
			// 生成一个随机数
			float random_value = dis2(gen);
			tmp[j] = random_value;
		}


		std::memcpy(emptyDataSource.imagePixelDatas.data(), tmp, 16 * 16 * sizeof(float));
		textureBindInfos["InScatter_Texture"].textureDataSources.push_back(emptyDataSource);
	}







	textureBindInfos["SigmaT_Texture"].viewType = VK_IMAGE_VIEW_TYPE_3D;
	textureBindInfos["SigmaT_Texture"].format = VK_FORMAT_R32_SFLOAT;

	textureBindInfos["SigmaT_Texture"].binding = 1;

	textureBindInfos["InScatter_Texture"].viewType = VK_IMAGE_VIEW_TYPE_3D;
	textureBindInfos["InScatter_Texture"].format = VK_FORMAT_R32_SFLOAT;

	textureBindInfos["InScatter_Texture"].binding = 3;



	bufferBindInfos["SimpleSceenExampleBuffer"].size = sizeof(glm::mat4) * 3;
	bufferBindInfos["SimpleSceenExampleBuffer"].binding = 0;
	bufferBindInfos["SimpleSceenExampleBuffer"].pipeId = 0;
	bufferBindInfos["SimpleSceenExampleBuffer"].passId = 0;



	bufferBindInfos["Info"].size = sizeof(glm::vec4) * 3;
	bufferBindInfos["Info"].binding = 2;
	bufferBindInfos["Info"].pipeId = 0;
	bufferBindInfos["Info"].passId = 0;
}

void C14VolumetricandTranslucencyRenderingExample::Loop()
{
	uint32_t i = 0;;
	uint32_t exampleType = 0;


	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");


	Camera camera(glm::vec3(0,0,-3),glm::vec3(0,0,0),glm::vec3(0,1,0));
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




	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];


	SubmitSynchronizationInfo preComputePassSubmitSyncInfo;
	preComputePassSubmitSyncInfo.waitSemaphores = {  };
	preComputePassSubmitSyncInfo.waitStages = {  };
	preComputePassSubmitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	

	//绑定uniform buffer
	BindBuffer("SimpleSceenExampleBuffer");
	bufferBindInfos["SimpleSceenExampleBuffer"].pipeId = 1;
	BindBuffer("SimpleSceenExampleBuffer");

	BindBuffer("Info");
	bufferBindInfos["Info"].pipeId = 1;
	bufferBindInfos["Info"].binding = 1;
	BindBuffer("Info");

	BindTexture("SigmaT_Texture");
	BindTexture("InScatter_Texture");
	//BindBuffer("Info");
	
	glm::vec3 lightPos = glm::vec3(0, -3, 3);
	glm::vec3 lightColor = glm::vec3(1, 1, 1);

	FillBuffer(buffers["Info"], sizeof(glm::vec4), sizeof(glm::vec3), (const char*)&lightPos);
	FillBuffer(buffers["Info"], sizeof(glm::vec4) * 2, sizeof(glm::vec3), (const char*)&lightColor);
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
		FillBuffer(buffers["Info"], 0, sizeof(glm::vec3), (const char*)&camera.GetPos());
		CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理
		//确保presentFence在创建时已经触发
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


void C14VolumetricandTranslucencyRenderingExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
