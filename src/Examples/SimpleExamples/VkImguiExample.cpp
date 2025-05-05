#include "VkImguiExample.h"

void VkImguiExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/VkImguiExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/VkImguiExample.frag";
	InitDefaultGraphicSubpassInfo(shaderCodePath);


}

void VkImguiExample::InitResourceInfos()
{

	//LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geom);
	geoms.resize(1);
	auto& geom = geoms[0];
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	geom.InitAsScreenFillRect();
	

	//	
	TextureDataSource dataSource;
	dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/OIP.jpg";
	textureBindInfos["OIP"].textureDataSources.push_back(dataSource);
	textureBindInfos["OIP"].binding = 1;

	bufferBindInfos["Buffer"].size = 12;

}

void VkImguiExample::Loop()
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

void VkImguiExample::InitSyncObjectNumInfo()
{
	//numFences = 1;
	numSemaphores = 2;
}

void VkImguiExample::DrawImGui()
{
	// 你的UI内容
	ImGui::Begin("Hello Window");
	ImGui::Text("Hello, Vulkan ImGui!");
	ImGui::End();

}
