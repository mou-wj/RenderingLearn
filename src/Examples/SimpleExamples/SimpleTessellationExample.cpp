#include "SimpleTessellationExample.h"

void SimpleTessellationExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SimpleTessellationExample.vert";
	shaderCodePath.tessellationControlShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SimpleTessellationExample.tesc";
	shaderCodePath.tessellationEvaluationShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SimpleTessellationExample.tese";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SimpleTessellationExample.frag";
	InitDefaultGraphicSubpassInfo(shaderCodePath);
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.tessellationState.patchControlPoints = 3;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

}

void SimpleTessellationExample::InitResourceInfos()
{

	//LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geom);
	geoms.resize(1);
	auto& geom = geoms[0];
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	geom.vertexAttrib.vertices = {
		-0.8,0.8,0,
		0.8,0.8,0,
		0.8,-0.8,0,
		-0.8,0.7,0,
		0.7,-0.8,0,
		-1,-1,0
	};
	tinyobj::shape_t triangle;
	tinyobj::index_t index;
	index.vertex_index = 0;
	triangle.mesh.indices.push_back(index);
	index.vertex_index = 1;
	triangle.mesh.indices.push_back(index);
	index.vertex_index = 2;
	triangle.mesh.indices.push_back(index);
	triangle.mesh.num_face_vertices.push_back(3);
	index.vertex_index = 3;
	triangle.mesh.indices.push_back(index);
	index.vertex_index = 4;
	triangle.mesh.indices.push_back(index);
	index.vertex_index = 5;
	triangle.mesh.indices.push_back(index);
	triangle.mesh.num_face_vertices.push_back(3);
	geom.shapes.push_back(triangle);




}

void SimpleTessellationExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	CaptureNum(3)
	//记录command buffer

	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = {VK_PIPELINE_STAGE_TRANSFER_BIT};
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		//确保presentFence在创建时已经触发

		CaptureBeginMacro
		CmdListWaitFinish(graphicCommandList);
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);
		CmdListReset(graphicCommandList);
		CmdListRecordBegin(graphicCommandList);
		CmdOpsDrawGeom(graphicCommandList);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachments[0].attachmentImage,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_NONE,VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsCopyWholeImageToImage(graphicCommandList, renderTargets.colorAttachments[0].attachmentImage, swapchainImages[nexIndex]);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachments[0].attachmentImage, VK_ACCESS_TRANSFER_READ_BIT,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_TRANSFER_WRITE_BIT,VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		
		Present(nexIndex, { finishCopyTargetToSwapchain });
		
		CaptureEndMacro


	}

}

void SimpleTessellationExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
