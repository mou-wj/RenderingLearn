#include "C17CurvesAndCurvedSurfacesExample.h"

void C17CurvesAndCurvedSurfacesExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	//shaderCodePath.taskShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C17CurvesAndCurvedSurfacesExample.task";
	shaderCodePath.meshShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C17CurvesAndCurvedSurfacesExampleCurves.mesh";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C17CurvesAndCurvedSurfacesExample.frag";
	renderPassInfos.resize(2);
	renderPassInfos[0].InitDefaultRenderPassInfo({shaderCodePath}, windowWidth, windowHeight);
	//InitDefaultGraphicSubpassInfo(shaderCodePath);
	enableMeshShaderEXT = true;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	
	shaderCodePath.meshShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C17CurvesAndCurvedSurfacesExampleSurfaces.mesh";
	renderPassInfos[1].InitDefaultRenderPassInfo({shaderCodePath}, windowWidth, windowHeight);
	
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_NONE;

}

void C17CurvesAndCurvedSurfacesExample::InitResourceInfos()
{

	bufferBindInfos["ControlPointsInfo"].size = sizeof(glm::vec4) * 8;

	bufferBindInfos["SurfaceControlPointsInfo"].size = sizeof(glm::vec4) * 4 * 4;
	bufferBindInfos["SurfaceControlPointsInfo"].passId = 1;
	


	bufferBindInfos["Transform"].size = sizeof(glm::mat4);
	bufferBindInfos["Transform"].passId = 1;
	bufferBindInfos["Transform"].binding = 1;

	renderPassInfos[0].subpassDrawMeshGroupInfos[0] = std::array<uint32_t, 3>{5, 1, 1};//设置mesh subpass的绘制参数
	renderPassInfos[1].subpassDrawMeshGroupInfos[0] = std::array<uint32_t, 3>{5, 5, 1};//设置mesh subpass的绘制参数

}

void C17CurvesAndCurvedSurfacesExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	CaptureNum(3)
		//记录command buffer
	glm::vec4 controlPoints[4];
	controlPoints[0] = glm::vec4(-0.7,0,0,1);
	controlPoints[1] = glm::vec4(0,-1,0,1);
	controlPoints[2] = glm::vec4(0.4, -0.5, 0,1);
	controlPoints[3] = glm::vec4(0.9, 0.5, 0, 1);
	glm::vec4 controlPointsTanV[4];
	controlPointsTanV[0] = glm::vec4(0, -1, 0, 1);
	controlPointsTanV[1] = glm::vec4(1, 0, 0, 1);
	controlPointsTanV[2] = glm::vec4(1, 1, 0, 1);
	controlPointsTanV[3] = glm::vec4(0, 1, 0, 1);


	FillBuffer(buffers["ControlPointsInfo"], 0, sizeof(glm::vec4) * 4, (const char*)controlPoints);
	FillBuffer(buffers["ControlPointsInfo"], sizeof(glm::vec4) * 4, sizeof(glm::vec4) * 4, (const char*)controlPointsTanV);
	BindBuffer("ControlPointsInfo");


	//Bezier曲面控制点
	glm::vec4 bezierSurfaceControlPoints[4][4];
	for (uint32_t i = 0; i < 4; i++)
	{
		for (uint32_t j = 0; j < 4; j++)
		{
			bezierSurfaceControlPoints[i][j] = glm::vec4(float(i) - 2, 0, float(j) + 2, 1.0);
		}
	}
	bezierSurfaceControlPoints[2][1].y = -4;

	Camera camera(glm::vec3(0, -4, -3), glm::vec3(0, 0, 3), glm::vec3(0, 1, 0));

	glm::mat4 mvpTransform = camera.GetProj() * camera.GetView();

	ShowMatColMajor(mvpTransform);

	FillBuffer(buffers["SurfaceControlPointsInfo"], 0, sizeof(glm::vec4) * 4 * 4, (const char*)bezierSurfaceControlPoints);
	FillBuffer(buffers["Transform"], 0, sizeof(glm::mat4), (const char*)&mvpTransform);

	BindBuffer("SurfaceControlPointsInfo");
	BindBuffer("Transform");


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



	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	uint32_t passId = 1;

	auto& renderTargets = renderPassInfos[passId].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		mvpTransform = camera.GetProj() * camera.GetView();
		FillBuffer(buffers["Transform"], 0, sizeof(glm::mat4), (const char*)&mvpTransform);

		CaptureBeginMacro
		CmdListWaitFinish(graphicCommandList);
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);
		CmdListReset(graphicCommandList);
		CmdListRecordBegin(graphicCommandList);
		CmdOpsDrawGeom(graphicCommandList, passId);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachments[0].attachmentImage,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_NONE,VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsCopyWholeImageToImage(graphicCommandList, renderTargets.colorAttachments[0].attachmentImage, swapchainImages[nexIndex]);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachments[0].attachmentImage, VK_ACCESS_TRANSFER_READ_BIT,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_TRANSFER_WRITE_BIT,VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro
		Present(nexIndex, { finishCopyTargetToSwapchain });
		



	}

}

void C17CurvesAndCurvedSurfacesExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
