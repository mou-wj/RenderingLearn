#include "C26RealTimeRayTracingExample.h"

void C26RealTimeRayTracingExample::InitRaytrcingPipelineInfo()
{
	rayTracingPipelinsDesc.valid = true;
	RayTracingShaderCodePaths pipelineShaderPaths;
	pipelineShaderPaths.rayGenerateShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C26RealTimeRayTracingExample.rgen";
	pipelineShaderPaths.missShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C26RealTimeRayTracingExample.rmiss";
	pipelineShaderPaths.closeHitShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C26RealTimeRayTracingExample.rchit";
	pipelineShaderPaths.callableShaderPath = std::string(PROJECT_DIR) + "/src/Examples/RealTimeRendering4/C26RealTimeRayTracingExample.rcall";
	rayTracingPipelinsDesc.raytracingPipelineShaderPaths.push_back(pipelineShaderPaths);

}

void C26RealTimeRayTracingExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/DrawSimpleTriangle.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/DrawSimgleTriangle.frag";
	//InitDefaultGraphicSubpassInfo(shaderCodePath);


}

void C26RealTimeRayTracingExample::InitResourceInfos()
{

	//LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geom);
	geoms.resize(1);
	auto& geom = geoms[0];
	geom.InitAsScreenFillRect();
	geom.shapeIndices = { {0,1,2} };



	TextureDataSource emptyDataSource; 
	emptyDataSource.width = windowWidth;
	emptyDataSource.height = windowHeight;
	textureBindInfos["rayTracingOutputTexture"].textureDataSources.push_back(emptyDataSource);
	textureBindInfos["rayTracingOutputTexture"].binding = 1;
	textureBindInfos["rayTracingOutputTexture"].pipeId = 0;
	textureBindInfos["rayTracingOutputTexture"].rayTracing = true;
	textureBindInfos["rayTracingOutputTexture"].usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	
	
	accelerationStructureBindInfos["accelerationStructure"].pipeId = 0;

}

void C26RealTimeRayTracingExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	
	BindTexture("rayTracingOutputTexture");

	BindAccelerationStructure("accelerationStructure");

	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };
	CaptureNum(3);
	//auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		//确保presentFence在创建时已经触发

		
		CmdListWaitFinish(graphicCommandList);
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);
		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		//CmdOpsDrawGeom(graphicCommandList);
		CmdOpsTraceRays(graphicCommandList, 0, { windowWidth,windowHeight,1 });
		CmdOpsImageMemoryBarrer(graphicCommandList, textures["rayTracingOutputTexture"].image, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_NONE,VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsCopyWholeImageToImage(graphicCommandList, textures["rayTracingOutputTexture"].image, swapchainImages[nexIndex]);
		CmdOpsImageMemoryBarrer(graphicCommandList, textures["rayTracingOutputTexture"].image, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_TRANSFER_WRITE_BIT,VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdListRecordEnd(graphicCommandList);
		
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro
		Present(nexIndex, { finishCopyTargetToSwapchain });
		
		//CopyImageToImage(testTexture.image, swapchainImages[nexIndex], { swapchainImageValidSemaphore }, { presentValidSemaphore });
		//CopyImageToImage(renderTargets.colorAttachment.attachmentImage,testTexture.image, { drawSemaphore }, { presentValidSemaphore });
		//WaitIdle();
		//auto rgba = (const char*)testTexture.image.hostMapPointer;
		//auto r = rgba[0];
		//auto g = rgba[1];
		//auto b = rgba[2];
		//auto a = rgba[3];



	}

}

void C26RealTimeRayTracingExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
