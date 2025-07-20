#include "HairArtifactExample.h"
#include "tiny_gltf.h"
#include "Framework/Utils/ModelFileTool.h"

void HairArtifactExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/HairArtifactExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/Artifacts/HairArtifactExample.frag";
	InitDefaultGraphicSubpassInfo(shaderCodePath);


}

void HairArtifactExample::InitResourceInfos()
{

	//LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geom);
	geoms.resize(1);
	auto& geom = geoms[0];
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	geom.InitAsScreenFillRect();

	auto hairModelSrc = std::string(PROJECT_DIR) + "/resources/glb/short_wavy_hair_with_bangs_with_bones.glb";
	RenderScene renderScene;
	LoadRenderScene(hairModelSrc, renderScene);

	//	
	TextureDataSource dataSource;
	dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/OIP.jpg";
	textureBindInfos["OIP"].textureDataSources.push_back(dataSource);
	textureBindInfos["OIP"].binding = 1;

	bufferBindInfos["Buffer"].size = 12;

}

void HairArtifactExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	CaptureNum(3);

	struct Buffer {
		float width, height;
		uint32_t enableTexture;
	} buffer;
	buffer.width = windowWidth;
	buffer.height = windowHeight;
	buffer.enableTexture = true;
	
	FillBuffer(buffers["Buffer"], 0, 12, (const char*)& buffer);


	auto drawFinished = semaphores[0];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = {  };
	submitSyncInfo.waitStages = {  };
	submitSyncInfo.sigSemaphores = { drawFinished };

	BindBuffer("Buffer");
	BindTexture("OIP");

	auto& renderTargets = renderPassInfos[0].renderTargets;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		//确保presentFence在创建时已经触发


		CmdListWaitFinish(graphicCommandList);
		CmdListReset(graphicCommandList);
		//CaptureBeginMacro
		CmdListRecordBegin(graphicCommandList);
		CmdOpsDrawGeom(graphicCommandList);
		CmdListRecordEnd(graphicCommandList);
		CmdListSubmit(graphicCommandList, submitSyncInfo);
		//CaptureEndMacro
		PresentPassResult(drawFinished,0, 0);


	}

}

void HairArtifactExample::InitSyncObjectNumInfo()
{
	//numFences = 1;
	numSemaphores = 2;
}

void HairArtifactExample::DrawImGui()
{
	// 你的UI内容
	ImGui::Begin("Hello Window");
	ImGui::Text("Hello, Vulkan ImGui!");
	ImGui::End();

}
