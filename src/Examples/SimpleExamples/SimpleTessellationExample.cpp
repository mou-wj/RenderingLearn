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
	geom.vertexAttributesDatas = {
		-0.8,0.8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		 0.8,0.8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0.8,-0.8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		-0.8,0.7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0.7,-0.8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		-1,-1,   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	
	};
	geom.shapeIndices = { {0,1,2,3,4,5} };



}

void SimpleTessellationExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	CaptureNum(3)
	//记录command buffer

	auto drawFinished = semaphores[0];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = {  };
	submitSyncInfo.waitStages = { };
	submitSyncInfo.sigSemaphores = { drawFinished };

	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		//确保presentFence在创建时已经触发

		CaptureBeginMacro
		CmdListWaitFinish(graphicCommandList);
		CmdListReset(graphicCommandList);
		CmdListRecordBegin(graphicCommandList);
		CmdOpsDrawGeom(graphicCommandList);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		
		PresentPassResult(drawFinished, 0, 0);
		
		CaptureEndMacro


	}

}

void SimpleTessellationExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
