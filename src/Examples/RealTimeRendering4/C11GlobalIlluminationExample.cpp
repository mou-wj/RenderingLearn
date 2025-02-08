#include "C11GlobalIlluminationExample.h"
#include "glm/mat4x4.hpp"

void C11GlobalIlluminationExample::InitSubPassInfo()
{

	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C11GlobalIlluminationExample.vert";
	drawSceenCodePath.geometryShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C11GlobalIlluminationExample.geom";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C11GlobalIlluminationExample.frag";


	ShaderCodePaths drawSceenDepthPassCodePath;
	drawSceenDepthPassCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C11GlobalIlluminationExample.vert";
	drawSceenDepthPassCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C11GlobalIlluminationExampleDepthPass.frag";


	
	renderPassInfos.resize(2);
	renderPassInfos[0].InitDefaultRenderPassInfo(drawSceenDepthPassCodePath,windowWidth,windowHeight);
	//修改depth pass的颜色附件格式
	renderPassInfos[0].renderTargets.colorAttachment.attachmentDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	//开启深度测试
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].renderTargets.colorAttachment.clearValue = VkClearValue{ 1.0,0.0,0.0,1.0 };

	renderPassInfos[1].InitDefaultRenderPassInfo(drawSceenCodePath, windowWidth, windowHeight);
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	///renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_NONE;

}

void C11GlobalIlluminationExample::InitResourceInfos()
{

	geoms.resize(1);
	geoms[0].useIndexBuffers = false;
	
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/monkey.obj",geoms[0]);
	
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	renderPassInfos[1].subpassDrawGeoInfos[0] = { 0 };

	//一个纹理点光源阴影贴图
	TextureDataSource emptyDataSource;
	emptyDataSource.width = windowWidth;
	emptyDataSource.height = windowHeight;
	emptyDataSource.picturePath = "";
	textureBindInfos["depthMap"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["depthMap"].binding = 2;
	textureBindInfos["depthMap"].viewType = VK_IMAGE_VIEW_TYPE_2D;
	textureBindInfos["depthMap"].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	textureBindInfos["depthMap"].passId = 1;

	bufferBindInfos["SimpleSceenExampleBuffer"].size = sizeof(glm::mat4) * 3;
	bufferBindInfos["SimpleSceenExampleBuffer"].binding = 0;

	bufferBindInfos["Info"].size = sizeof(glm::mat4) * 2 + sizeof(glm::vec3);
	bufferBindInfos["Info"].binding = 1;
	bufferBindInfos["Info"].passId = 1;
}

void C11GlobalIlluminationExample::Loop()
{
	uint32_t i = 0;;
	uint32_t exampleType = 0;


	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");


	Camera camera(glm::vec3(0,0,-3),glm::vec3(0,0,0),glm::vec3(0,1,0));
	//Camera camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
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

	WindowEventHandler::SetEventCallBack(KEY_I_PRESS, [&]() {
		uint32_t tmp = 0;
		std::cout << "输入一个整数: 范围[0,1]" << std::endl;
		std::cin >> tmp;
		if (exampleType > 2)
		{
			std::cout << "输入非法" << std::endl;
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
		; }, "点击I 输入一个整数来切换实例。0表示一个简单矩形面光源的blinn phong光照; 1表示使用球面坐标映射的环境贴图");




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
	auto depthPassFinish = semaphores[2];

	//绑定uniform buffer
	BindBuffer("SimpleSceenExampleBuffer");
	bufferBindInfos["SimpleSceenExampleBuffer"].passId = 1;
	BindBuffer("SimpleSceenExampleBuffer");
	
	BindBuffer("Info");



	BindTexture("depthMap");

	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { depthPassFinish, swapchainValidSemaphore };
	submitSyncInfo.waitStages = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };


	SubmitSynchronizationInfo depthPassSubmitSyncInfo;
	depthPassSubmitSyncInfo.waitSemaphores = {  };
	depthPassSubmitSyncInfo.waitStages = {  };
	depthPassSubmitSyncInfo.sigSemaphores = { depthPassFinish };

	auto& renderTargets = renderPassInfos[1].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();

		//运行一趟pass 得到当前的深度图
		CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理
		CmdListReset(graphicCommandList);

		CmdListRecordBegin(graphicCommandList);
		//进行depth pass
		CmdOpsDrawGeom(graphicCommandList);
		auto& colorTargetImage = renderPassInfos[0].renderTargets.colorAttachment.attachmentImage;

		auto colorOldLayout = colorTargetImage.currentLayout;
		auto precomputeTextureOldLayout = textures["depthMap"].image.currentLayout;

		CmdOpsImageMemoryBarrer(graphicCommandList, colorTargetImage, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);
		CmdOpsImageMemoryBarrer(graphicCommandList, textures["depthMap"].image, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);

		//拷贝深度图到深度纹理
		CmdOpsCopyImageToImage(graphicCommandList, colorTargetImage, 0, 0, textures["depthMap"].image, 0, 0);

		CmdOpsImageMemoryBarrer(graphicCommandList, colorTargetImage, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, colorOldLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0);
		CmdOpsImageMemoryBarrer(graphicCommandList, textures["depthMap"].image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE, precomputeTextureOldLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);

		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, depthPassSubmitSyncInfo);









		//确保presentFence在创建时已经触发
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
		buffer.view = camera.GetView();
		FillBuffer(buffers["SimpleSceenExampleBuffer"], 0, sizeof(Buffer), (const char*)&buffer);
		FillBuffer(buffers["Info"], 0, sizeof(glm::mat4) * 2, (const char*)&buffer.view);
		FillBuffer(buffers["Info"], sizeof(glm::mat4) * 2, sizeof(glm::vec3), (const char*)&camera.GetPos());
		CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理

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


void C11GlobalIlluminationExample::InitSyncObjectNumInfo()
{
	numSemaphores = 3;
}
