#include "SamplePointsExample.h"

void SamplePointsExample::InitComputeInfo()
{
	//需要计算管线
	computeDesc.valid = true;
	computeDesc.computeShaderPaths = { std::string(PROJECT_DIR) + "/src/Examples/PBRT4/SamplePointsExample.comp" };


}

void SamplePointsExample::InitSubPassInfo()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/PBRT4/SamplePointsExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/PBRT4/SamplePointsExample.frag";
	InitDefaultGraphicSubpassInfo(shaderCodePath);


}

void SamplePointsExample::InitResourceInfos()
{

	//LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geom);
	geoms.resize(1);
	auto& geom = geoms[0];
	renderPassInfos[0].subpassDrawGeoInfos[0] = { 0 };
	geom.vertexAttrib.vertices = {
		-1,1,0,
		1,1,0,
		1,-1,0,
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
	index.vertex_index = 0;
	triangle.mesh.indices.push_back(index);
	index.vertex_index = 2;
	triangle.mesh.indices.push_back(index);
	index.vertex_index = 3;
	triangle.mesh.indices.push_back(index);
	triangle.mesh.num_face_vertices.push_back(3);
	geom.shapes.push_back(triangle);


	//	
	TextureDataSource dataSource;
	//dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/OIP.jpg";
	dataSource.width = 512;
	dataSource.height = 512;
	dataSource.imagePixelDatas.resize(512 * 512 * 4, 255);
	
	//textureBindInfos["outputImage"].textureDataSources.push_back(dataSource);
	//textureBindInfos["outputImage"].binding = 0;
	//textureBindInfos["outputImage"].usage = VK_IMAGE_USAGE_STORAGE_BIT;
	//textureBindInfos["outputImage"].compute = true;

	textureBindInfos["image"].textureDataSources.push_back(dataSource);
	textureBindInfos["image"].binding = 0;
	textureBindInfos["image"].usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	textureBindInfos["image"].compute = true;


	//textureBindInfos["testTexture"].textureDataSources.push_back(dataSource);
	//textureBindInfos["testTexture"].binding = 1;
	//textureBindInfos["testTexture"].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	bufferBindInfos["Buffer"].size = 8;
	bufferBindInfos["Option"].size = 36;
	bufferBindInfos["Option"].binding = 1;
	bufferBindInfos["Option"].compute = true;
}

void SamplePointsExample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
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
		uint32_t sampleType = 1, base1 = 2, base2 = 3, base3 = 5,sequenceType =0;
	} buffer;
	buffer.width = windowWidth;
	buffer.height = windowHeight;
	
	FillBuffer(buffers["Buffer"], 0, 8, (const char*)&buffer);
	FillBuffer(buffers["Option"], 0, 36, ((const char*)&buffer)+8);

	auto swapchainValidSemaphore = semaphores[0];
	auto finishCopyTargetToSwapchain = semaphores[1];
	SubmitSynchronizationInfo submitSyncInfo;
	submitSyncInfo.waitSemaphores = { swapchainValidSemaphore };
	submitSyncInfo.waitStages = { VK_PIPELINE_STAGE_TRANSFER_BIT };
	submitSyncInfo.sigSemaphores = { finishCopyTargetToSwapchain };

	BindBuffer("Buffer");
	BindBuffer("Option");
	//绑定到计算管线的描述符集中
	textureBindInfos["image"].binding = 0;
	textureBindInfos["image"].compute = true;
	BindTexture("image");
	//绑定到图形管线的描述符集中
	textureBindInfos["image"].binding = 1;
	textureBindInfos["image"].compute = false;
	BindTexture("image");

	WindowEventHandler::SetEventCallBack(EventType::KEY_I_PRESS, [&]() {
		buffer.sampleType = (buffer.sampleType+1) % 2;
		if (buffer.sampleType == 0)
		{
			std::cout << "当前为独立采样点:" << std::endl;
		}
		else {
			std::cout << "当前为分层采样点:" << std::endl;
		}
		FillBuffer(buffers["Option"], 0, 36, ((const char*)&buffer) + 8);
		}, "按I输入切换采样类型");
	WindowEventHandler::SetEventCallBack(EventType::KEY_J_PRESS, [&]() {
		uint32_t base1 = 2,base2 = 3,base3 = 5;
		std::cout << "请输入三个素数奇数:" << std::endl;
		std::cin >> base1 >> base2 >> base3;
		buffer.base1 = base1;
		buffer.base2 = base2;
		buffer.base3 = base3;
		FillBuffer(buffers["Option"], 0, 36, ((const char*)&buffer) + 8);

		}, "按J输入三个Hammersley序列的素数基数，分别用于生成随机Holton采样点的第1维，2维，3维分量");

	WindowEventHandler::SetEventCallBack(EventType::KEY_K_PRESS, [&]() {
		buffer.sequenceType = (buffer.sequenceType + 1) % 2;
		if (buffer.sequenceType == 0)
		{
			std::cout << "当前使用Hammersley 序列" << std::endl;
		}
		else {
			std::cout << "当前使用Sobol 序列" << std::endl;
		}
		FillBuffer(buffers["Option"], 0, 36, ((const char*)&buffer) + 8);
		}, "按K输入切换序列类型");

	WindowEventHandler::SetEventCallBack(EventType::KEY_C_PRESS, [&]() {
		CaptureNum(2)
		}, "按C捕获一帧");


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
		//利用计算管线生成一张采样点的结果图片
		//转换布局
		//CmdOpsImageMemoryBarrer(graphicCommandList, textures["image"].image, VK_ACCESS_NONE, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		CmdOpsDispatch(graphicCommandList);
		//CmdOpsImageMemoryBarrer(graphicCommandList, textures["image"].image, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		CmdOpsImageMemoryBarrer(graphicCommandList, textures["image"].image, VK_ACCESS_NONE, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);





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

}

void SamplePointsExample::InitSyncObjectNumInfo()
{
	//numFences = 1;
	numSemaphores = 2;
}
