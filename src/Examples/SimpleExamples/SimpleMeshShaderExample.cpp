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
	geom.vertexAttributesDatas = {
		-0.8,0.8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0.8,0.8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0.8,-0.8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		-0.8,0.7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0.7,-0.8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0


	};
	geom.shapeIndices = { {0,1,2,3,4,5} };

	renderPassInfos[0].subpassDrawMeshGroupInfos[0] = std::array<uint32_t, 3>{1, 1, 1};//设置mesh subpass的绘制参数


}

void SimpleMeshShaderExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	CaptureNum(3)
	//记录command buffer

	auto drawFinished = semaphores[0];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = {  };
	submitSyncInfo.waitStages = {};
	submitSyncInfo.sigSemaphores = { drawFinished };

	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();

		CaptureBeginMacro
		CmdListWaitFinish(graphicCommandList);
		//确保presentFence在创建时已经触发
		CmdListReset(graphicCommandList);
		CmdListRecordBegin(graphicCommandList);
		CmdOpsDrawGeom(graphicCommandList);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		CaptureEndMacro
		PresentPassResult(drawFinished, 0, 0);
		



	}

}

void SimpleMeshShaderExample::InitSyncObjectNumInfo()
{
	numSemaphores = 2;
}
