#include "SimpleMeshShaderExample.h"

void SimpleMeshShaderExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.taskShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SimpleMeshShaderExample.task";
	shaderCodePath.meshShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SimpleMeshShaderExample.mesh";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/SimpleMeshShaderExample.frag";
	InitDefaultGraphicSubpassInfo(shaderCodePath);
	enableMeshShaderEXT = true;
	//renderPassInfos[0].renderTargets.colorAttachment.attachmentDesc.format = VK_FORMAT_B8G8R8A8_SRGB;
}

void SimpleMeshShaderExample::InitResourceInfos()
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

	renderPassInfos[0].subpassDrawMeshGroupInfos[0] = std::array<uint32_t, 3>{1, 1, 1};//设置mesh subpass的绘制参数


}

void SimpleMeshShaderExample::Loop()
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
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);
		CaptureBeginMacro
		CmdListWaitFinish(graphicCommandList);
		CmdListReset(graphicCommandList);
		CmdListRecordBegin(graphicCommandList);
		CmdOpsDrawGeom(graphicCommandList);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachment.attachmentImage,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_NONE,VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdOpsCopyWholeImageToImage(graphicCommandList, renderTargets.colorAttachment.attachmentImage, swapchainImages[nexIndex]);
		CmdOpsImageMemoryBarrer(graphicCommandList, renderTargets.colorAttachment.attachmentImage, VK_ACCESS_TRANSFER_READ_BIT,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, swapchainImages[nexIndex], VK_ACCESS_TRANSFER_WRITE_BIT,VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro
		Present(nexIndex, { finishCopyTargetToSwapchain });
		



	}

}

void SimpleMeshShaderExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
