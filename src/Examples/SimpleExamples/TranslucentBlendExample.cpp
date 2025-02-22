#include "TranslucentBlendExample.h"

void TranslucentBlendExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/TranslucentBlendExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/TranslucentBlendExample.frag";
	renderPassInfos.resize(1);
	renderPassInfos[0].InitDefaultRenderPassInfo(shaderCodePath, windowWidth, windowHeight);

	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.colorBlendState.logicOpEnable = VK_FALSE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.colorBlendState.logicOp = VK_LOGIC_OP_MAX_ENUM;

	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.colorBlendAttachmentStates[0].blendEnable = VK_TRUE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.colorBlendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.colorBlendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.colorBlendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.colorBlendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.colorBlendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.colorBlendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;
	renderPassInfos[0].subpassInfo.subpassDescs[0].subpassPipelineStates.colorBlendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;


}

void TranslucentBlendExample::InitResourceInfos()
{

	//LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geom);
	geoms.resize(2);
	auto& geom0 = geoms[0];
	auto& geom1 = geoms[1];
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0,1 };
	geom0.vertexAttrib.vertices = {
		-1,1,0.1,
		1,1,0.1,
		1,0,0.1,
		-1,0,0.1
	};
	geom0.vertexAttrib.colors = {
		1,0,0,
		1,0,0,
		1,0,0,
		1,0,0
	
	
	};
	geom0.useIndexBuffers = false;
	geom1.vertexAttrib.vertices = {
	0,1,0,
	1,1,0,
	1,-1,0,
	0,-1,0
	};
	geom1.vertexAttrib.colors = {
	0,0,1,
	0,0,1,
	0,0,1,
	0,0,1


	};
	geom1.useIndexBuffers = false;
	tinyobj::shape_t triangle;
	tinyobj::index_t index;
	index.vertex_index = 0;
	triangle.mesh.indices.push_back(index);
	index.vertex_index = 1;
	triangle.mesh.indices.push_back(index);
	index.vertex_index = 2;
	triangle.mesh.indices.push_back(index);
	triangle.mesh.num_face_vertices.push_back(3);
	index.vertex_index = 0;
	triangle.mesh.indices.push_back(index);
	index.vertex_index = 2;
	triangle.mesh.indices.push_back(index);
	index.vertex_index = 3;
	triangle.mesh.indices.push_back(index);
	triangle.mesh.num_face_vertices.push_back(3);
	geom0.shapes.push_back(triangle);
	geom1.shapes.push_back(triangle);


	bufferBindInfos["Buffer"].size = sizeof(glm::vec4);

}

void TranslucentBlendExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	CaptureNum(3);
	//std::vector<char> testImageData;

	//testImageData.resize(512 * 512 * 4);
	//for (uint32_t i = 0; i < 512; i++)
	//{
	//	for (uint32_t j = 0; j < 512; j++)
	//	{
	//		testImageData[(512 * i + j) * 4] = 255;
	//		testImageData[(512 * i + j) * 4 + 1] = 0;
	//		testImageData[(512 * i + j) * 4 + 2] = 0;
	//		testImageData[(512 * i + j) * 4 + 3] = 255;
	//	}


	//}

	//Texture testTexture = Create2DTexture(512, 512, testImageData.data());
	glm::vec4 color;
	
	FillBuffer(buffers["Buffer"], 0, 12, (const char*)&color);


	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	BindBuffer("Buffer");

	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		//确保presentFence在创建时已经触发
		auto nexIndex = GetNextPresentImageIndex(swapchainValidSemaphore);

		CmdListWaitFinish(graphicCommandList);
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

}

void TranslucentBlendExample::InitSyncObjectNumInfo()
{
	//numFences = 1;
	numSemaphores = 2;
}
