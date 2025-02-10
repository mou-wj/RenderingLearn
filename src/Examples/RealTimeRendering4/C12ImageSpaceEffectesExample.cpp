#include "C12ImageSpaceEffectesExample.h"
#include "glm/mat4x4.hpp"

void C12ImageSpaceEffectesExample::InitComputeInfo()
{
	//需要计算管线
	computeDesc.valid = true;
	computeDesc.computeShaderPaths = { std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C12ImageSpaceEffectesExample.comp" };


}

void C12ImageSpaceEffectesExample::InitSubPassInfo()
{

	ShaderCodePaths drawSceenCodePath;
	drawSceenCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C12ImageSpaceEffectesExample.vert";
	drawSceenCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C12ImageSpaceEffectesExample.frag";

	
	renderPassInfos.resize(1);
	renderPassInfos[0].InitDefaultRenderPassInfo(drawSceenCodePath,windowWidth,windowHeight);
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthTestEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthWriteEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	///renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_NONE;

}

void C12ImageSpaceEffectesExample::InitResourceInfos()
{

	geoms.resize(1);
	geoms[0].useIndexBuffers = false;
	geoms[0].InitAsScreenFillRect();
	
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };


	//一个纹理点光源阴影贴图
	TextureDataSource emptyDataSource;
	emptyDataSource.width = windowWidth;
	emptyDataSource.height = windowHeight;
	emptyDataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/DaySkyHDRI046A_1K-TONEMAPPED.jpg";;

	textureBindInfos["postProcessInput"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["postProcessInput"].compute = true;
	textureBindInfos["postProcessInput"].binding = 0;

	//
	textureBindInfos["postProcessResult"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["postProcessResult"].compute = true;
	textureBindInfos["postProcessResult"].binding = 1;
	textureBindInfos["postProcessResult"].usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	textureBindInfos["postProcessResultCache1"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["postProcessResultCache1"].compute = true;
	textureBindInfos["postProcessResultCache1"].binding = 1;
	textureBindInfos["postProcessResultCache1"].usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	textureBindInfos["postProcessResultCache2"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["postProcessResultCache2"].compute = true;
	textureBindInfos["postProcessResultCache2"].binding = 1;
	textureBindInfos["postProcessResultCache2"].usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	bufferBindInfos["Info"].size = sizeof(uint32_t);
	bufferBindInfos["Info"].compute = true;
	bufferBindInfos["Info"].binding = 2;

}

void C12ImageSpaceEffectesExample::Loop()
{
	uint32_t i = 0;;
	uint32_t exampleType = 0;
	CaptureNum(6)

	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");



	uint32_t funcType = 0;


	//绑定计算着色其的资源并执行
	BindBuffer("Info");

	


	//执行高斯模糊计算着色器
	GaussianFilterExample();
	//LightBlurExample();




	//绑定结果到图形管线并绘制
	textureBindInfos["postProcessResult"].compute = false;
	textureBindInfos["postProcessResult"].binding = 0;
	BindTexture("postProcessResult");


	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];

	

	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = {swapchainValidSemaphore };
	submitSyncInfo.waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();

		//确保presentFence在创建时已经触发
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理

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


void C12ImageSpaceEffectesExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}

void C12ImageSpaceEffectesExample::GaussianFilterExample()
{
	uint32_t funcType = 0;
	FillBuffer(buffers["Info"], 0, sizeof(uint32_t), (const char*)&funcType);

	BindTexture("postProcessInput");
	BindTexture("postProcessResult");

	uint32_t textureW = textureBindInfos["postProcessResult"].textureDataSources[0].width;
	uint32_t textureH = textureBindInfos["postProcessResult"].textureDataSources[0].height;


	SubmitSynchronizationInfo computeSubmitSyncInfo;
	computeSubmitSyncInfo.waitSemaphores = { };
	computeSubmitSyncInfo.waitStages = {  };
	computeSubmitSyncInfo.sigSemaphores = { };

	//运行计算着色器
	CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理
	CmdListReset(graphicCommandList);
	CaptureBeginMacro
	CmdListRecordBegin(graphicCommandList);
	CmdOpsImageMemoryBarrer(graphicCommandList, textures["postProcessResult"].image, VK_ACCESS_NONE, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	CmdOpsDispatch(graphicCommandList, 0, { textureW,textureH,1 });
	CmdOpsImageMemoryBarrer(graphicCommandList, textures["postProcessResult"].image, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	CmdListRecordEnd(graphicCommandList);

	CmdListSubmit(graphicCommandList, computeSubmitSyncInfo);
	CaptureEndMacro
	WaitIdle();


}

void C12ImageSpaceEffectesExample::LightBlurExample()
{
	uint32_t funcType = 1;
	FillBuffer(buffers["Info"], 0, sizeof(uint32_t), (const char*)&funcType);

	BindTexture("postProcessInput");
	BindTexture("postProcessResultCache1");
	
	uint32_t textureW = textureBindInfos["postProcessResultCache1"].textureDataSources[0].width;
	uint32_t textureH = textureBindInfos["postProcessResultCache1"].textureDataSources[0].height;


	SubmitSynchronizationInfo computeSubmitSyncInfo;
	computeSubmitSyncInfo.waitSemaphores = { };
	computeSubmitSyncInfo.waitStages = {  };
	computeSubmitSyncInfo.sigSemaphores = { };

	//运行计算着色器执行光源过滤
	CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理
	CmdListReset(graphicCommandList);
	CaptureBeginMacro
	CmdListRecordBegin(graphicCommandList);
	CmdOpsImageMemoryBarrer(graphicCommandList, textures["postProcessResultCache1"].image, VK_ACCESS_NONE, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	CmdOpsDispatch(graphicCommandList, 0, { textureW,textureH,1 });
	CmdOpsImageMemoryBarrer(graphicCommandList, textures["postProcessResultCache1"].image, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	CmdListRecordEnd(graphicCommandList);

	CmdListSubmit(graphicCommandList, computeSubmitSyncInfo);
	CaptureEndMacro
	WaitIdle();
	//将结果拷贝到另外一个输入


	//将输入改为上次过滤的输出并进行高斯模糊
	textureBindInfos["postProcessResultCache1"].binding = 0;
	//绑定到输入位
	BindTexture("postProcessResultCache1");
	
	//绑定高斯模糊的输出
	BindTexture("postProcessResultCache2");
	
	
	funcType = 0;
	FillBuffer(buffers["Info"], 0, sizeof(uint32_t), (const char*)&funcType);


	//运行计算着色器执行光源模糊
	CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理
	CmdListReset(graphicCommandList);
	CaptureBeginMacro
	CmdListRecordBegin(graphicCommandList);
	CmdOpsImageMemoryBarrer(graphicCommandList, textures["postProcessResultCache2"].image, VK_ACCESS_NONE, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	CmdOpsDispatch(graphicCommandList, 0, { textureW,textureH,1 });
	CmdOpsImageMemoryBarrer(graphicCommandList, textures["postProcessResultCache2"].image, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	CmdListRecordEnd(graphicCommandList);

	CmdListSubmit(graphicCommandList, computeSubmitSyncInfo);
	CaptureEndMacro
	WaitIdle();


	//将原图片绑定到输入0
	BindTexture("postProcessInput");
	//将模糊后的结果绑定到输入1
	textureBindInfos["postProcessResultCache2"].binding = 3;
	BindTexture("postProcessResultCache2");

	//绑定输出
	BindTexture("postProcessResult");

	funcType = 2;
	FillBuffer(buffers["Info"], 0, sizeof(uint32_t), (const char*)&funcType);

	//运行计算着色器执行光源模糊
	CmdListWaitFinish(graphicCommandList);//因为是单线程，所以等待命令完成后再处理
	CmdListReset(graphicCommandList);
	CaptureBeginMacro
	CmdListRecordBegin(graphicCommandList);
	CmdOpsImageMemoryBarrer(graphicCommandList, textures["postProcessResult"].image, VK_ACCESS_NONE, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	CmdOpsDispatch(graphicCommandList, 0, { textureW,textureH,1 });
	CmdOpsImageMemoryBarrer(graphicCommandList, textures["postProcessResult"].image, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	CmdListRecordEnd(graphicCommandList);

	CmdListSubmit(graphicCommandList, computeSubmitSyncInfo);
	CaptureEndMacro
	WaitIdle();

}
