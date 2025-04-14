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
	geom0.vertexAttributesDatas = {
	   -1,1,0.1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
		1,1,0.1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
		1,0,0.1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
	   -1,0,0.1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0
	
	};
	geom1.vertexAttributesDatas = {
		0,1,0.1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
		1,1,0.1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
	   1,-1,0.1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
	   0,-1,0.1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0

	};
	geom0.shapeIndices = { {0,1,2} };
	geom1.shapeIndices = { {0,2,3} };

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


	auto drawFinished = semaphores[0];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = {  };
	submitSyncInfo.waitStages = {  };
	submitSyncInfo.sigSemaphores = { drawFinished };

	BindBuffer("Buffer");

	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		//确保presentFence在创建时已经触发


		CmdListWaitFinish(graphicCommandList);
		CmdListReset(graphicCommandList);
		CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		CmdOpsDrawGeom(graphicCommandList);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro
		PresentPassResult(drawFinished, 0, 0);


	}

}

void TranslucentBlendExample::InitSyncObjectNumInfo()
{
	//numFences = 1;
	numSemaphores = 2;
}
