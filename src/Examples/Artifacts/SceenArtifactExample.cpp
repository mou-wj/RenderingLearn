#include "SceenArtifactExample.h"
#include "glm/mat4x4.hpp"

void SceenArtifactExample::InitSubPassInfo()
{


	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/SceenArtifactExample.vert";
	drawSceenCodePath.geometryShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/SceenArtifactExample.geom";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/SceenArtifactExample.frag";

	ShaderCodePaths frogBlendCodePath;
	frogBlendCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/SceenArtifactExampleFrogBlend.vert";
	frogBlendCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/SceenArtifactExampleFrogBlend.frag";
	
	renderPassInfos.resize(2);

	renderPassInfos[0].InitDefaultRenderPassInfo({ drawSceenCodePath,frogBlendCodePath }, windowWidth, windowHeight,1, { {1,true} });
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_NONE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//写入深度附件

	ShaderCodePaths lightDepthCodePath;
	lightDepthCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/SceenArtifactExampleLightDepth.vert";
	lightDepthCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/SceenArtifactExampleLightDepth.frag";

	ShaderCodePaths lightDepthCodeMapCopyPath;
	lightDepthCodeMapCopyPath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/SceenArtifactExampleLightDepthMapCopy.vert";
	lightDepthCodeMapCopyPath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/SceenArtifactExampleLightDepthMapCopy.frag";


	renderPassInfos[1].InitDefaultRenderPassInfo({ lightDepthCodePath,lightDepthCodeMapCopyPath }, windowWidth, windowHeight, 1, { {1,true} });
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	renderPassInfos[1].renderTargets.colorAttachments[0].clearValue = VkClearValue{1,0,0,1};
	renderPassInfos[1].renderTargets.colorAttachments[0].attachmentDesc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;//写入深度附件
	renderPassInfos[1].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_NONE;

	renderPassInfos[1].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthTestEnable = VK_FALSE;
	renderPassInfos[1].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	renderPassInfos[1].subpassInfo.subpassDescs[1].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_FALSE;//写入深度附件
}

void SceenArtifactExample::InitResourceInfos()
{

	geoms.resize(2);
	geoms[0].useIndexBuffers = false;
	LoadObj(std::string(PROJECT_DIR) + "/resources/obj/low_poly_nature_free.obj",geoms[0]);
	geoms[1].InitAsScreenFillRect();

	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	renderPassInfos[0].subpassDrawGeoInfos[1] = { 1 };
	renderPassInfos[1].subpassDrawGeoInfos[0] = { 0 };
	renderPassInfos[1].subpassDrawGeoInfos[1] = { 1 };

	TextureDataSource emptyDataSource;
	emptyDataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/low_poly_nature_free.png";
	textureBindInfos["baseColorTexture"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["baseColorTexture"].binding = 2;



	TextureDataSource emptyDepthDataSource;
	emptyDepthDataSource.width = windowWidth;
	emptyDepthDataSource.height = windowHeight;
	textureBindInfos["lightDepthTexture"].textureDataSources.push_back(emptyDepthDataSource);
	textureBindInfos["lightDepthTexture"].binding = 3;
	textureBindInfos["lightDepthTexture"].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	textureBindInfos["lightDepthTexture"].usage |= VK_IMAGE_USAGE_STORAGE_BIT;

	bufferBindInfos["TransformBuffer"].size = sizeof(glm::mat4) * 3;
	bufferBindInfos["TransformBuffer"].binding = 0;
	bufferBindInfos["TransformBuffer"].pipeId = 0;

	bufferBindInfos["Info"].size = sizeof(glm::vec4) * 2 + sizeof(glm::mat4) * 1 ;
	bufferBindInfos["Info"].binding = 1;
	bufferBindInfos["Info"].pipeId = 0;



}

void SceenArtifactExample::Loop()
{
	uint32_t i = 0;;
	glm::vec3 lightDir = glm::vec3(-30, -20, 0);

	GenerateLightDepthMap(lightDir);




	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	CaptureNum(3)


	Camera camera(glm::vec3(-15, -10,0),glm::vec3(-3,-3,0),glm::vec3(0,1,0));
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


	

	//绑定uniform buffer
	bufferBindInfos["TransformBuffer"].binding = 0;
	bufferBindInfos["TransformBuffer"].pipeId = 0;
	bufferBindInfos["TransformBuffer"].passId = 0;
	BindBuffer("TransformBuffer");


	BindBuffer("Info");

	//绑定纹理

	textureBindInfos["baseColorTexture"].passId = 0;
	textureBindInfos["baseColorTexture"].binding = 2;
	BindTexture("baseColorTexture");

	textureBindInfos["lightDepthTexture"].passId = 0;
	textureBindInfos["lightDepthTexture"].pipeId = 0;
	textureBindInfos["lightDepthTexture"].binding = 3;
	BindTexture("lightDepthTexture");

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
		FillBuffer(buffers["TransformBuffer"], 0, sizeof(Buffer), (const char*)&buffer);
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


void SceenArtifactExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}

void SceenArtifactExample::GenerateLightDepthMap(glm::vec3 lightDir)
{

	textureBindInfos["baseColorTexture"].passId = 1;
	textureBindInfos["baseColorTexture"].binding = 2;
	BindTexture("baseColorTexture");

	textureBindInfos["lightDepthTexture"].passId = 1;
	textureBindInfos["lightDepthTexture"].binding = 1;
	textureBindInfos["lightDepthTexture"].pipeId = 1;
	BindTexture("lightDepthTexture");

	FillBuffer(buffers["Info"], sizeof(glm::vec4), sizeof(glm::vec3), (const char*)&lightDir);


	ClearTexture("lightDepthTexture", {1,0,0,1});



	Camera camera(lightDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0),false);;
	camera.SetParallelProjectParams(0, 100, 60, 60);

	struct Buffer {
		glm::mat4 world;
		glm::mat4 view;
		glm::mat4 proj;
	} buffer;
	buffer.world = glm::mat4(1.0);
	buffer.view = camera.GetView();
	buffer.proj = camera.GetProj();

	glm::mat4 lightTransform = buffer.proj * buffer.view * buffer.world;
	glm::vec4 tmp = glm::vec4(-12.36641, -0.22178, -13.53522,1.0);
	tmp = lightTransform * tmp;
	tmp /= tmp.w;
	ShowVec(tmp);

	FillBuffer(buffers["Info"], sizeof(glm::vec4) * 2, sizeof(glm::mat4), (const char*)&lightTransform);

	glm::vec4 curP = camera.GetProj() * camera.GetView() * glm::vec4(1, 1, 1,1);
	ShowVec(curP);
	ShowMatColMajor(lightTransform);

	//执行depth path

	//buffer.view = Transform::GetEularRotateMatrix(0, 0, 0.2) * buffer.view;
	FillBuffer(buffers["TransformBuffer"], 0, sizeof(Buffer), (const char*)&buffer);

	bufferBindInfos["TransformBuffer"].binding = 0;
	bufferBindInfos["TransformBuffer"].passId = 1;
	BindBuffer("TransformBuffer");


	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = {  };
	submitSyncInfo.waitStages = {  };
	submitSyncInfo.sigSemaphores = {  };

	CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理
	auto& renderTargets = renderPassInfos[1].renderTargets;

	CmdListReset(graphicCommandList);
	CaptureBeginMacro
	CmdListRecordBegin(graphicCommandList);
	//CmdOpsImageMemoryBarrer(graphicCommandList, textures["lightDepthTexture"].image, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0);


	CmdOpsDrawGeom(graphicCommandList,1);
	//CmdOpsImageMemoryBarrer(graphicCommandList, textures["lightDepthTexture"].image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0);
	//CmdOpsImageMemoryBarrer(graphicCommandList, textures["lightDepthTexture"].image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_NONE, depthTextureOldLayout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);

	CmdListRecordEnd(graphicCommandList);
	CmdListSubmit(graphicCommandList, submitSyncInfo);
	CaptureEndMacro
	WaitIdle();
	

	textures["lightDepthTexture"].image.WriteToJpgFloat("test.jpg", 0, 0);

}
