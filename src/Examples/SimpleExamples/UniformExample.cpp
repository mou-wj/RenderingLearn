#include "UniformExample.h"

void UniformExample::InitSubPassInfo()
{
	InitDefaultGraphicSubpassInfo();


}

void UniformExample::InitResourceInfos()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/UniformExample.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/src/Examples/SimpleExamples/UniformExample.frag";
	pipelinesShaderCodePaths = { shaderCodePath };
	//LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geom);
	geoms.resize(1);
	auto& geom = geoms[0];
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
	dataSource.picturePath = std::string(PROJECT_DIR) + "/resources/pic/OIP.jpg";
	textureInfos["OIP"].textureDataSources.push_back(dataSource);
	textureInfos["OIP"].binding = 1;

	uniformBufferInfos["Buffer"].size = 12;

}

void UniformExample::Loop()
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
		uint32_t enableTexture;
	} buffer;
	buffer.width = windowWidth;
	buffer.height = windowHeight;
	buffer.enableTexture = true;
	
	FillBuffer(uniformBuffers["Buffer"], 0, 12, (const char*)& buffer);

	uint32_t numCap = 4;
	while (!WindowEventHandler::WindowShouldClose())
	{
		i++;
		WindowEventHandler::ProcessEvent();
		//if (numCap != 0)
		//{
			CaptureBeginMacro
//		}
		//确保presentFence在创建时已经触发
		DrawGeom({}, { drawSemaphore });
			CaptureEndMacro;

		auto nexIndex = GetNextPresentImageIndex(swapchainImageValidSemaphore);
		CopyImageToImage(renderTargets.colorAttachment.attachmentImage, swapchainImages[nexIndex], { swapchainImageValidSemaphore,drawSemaphore }, { presentValidSemaphore });

		//CopyImageToImage(testTexture.image, swapchainImages[nexIndex], { swapchainImageValidSemaphore }, { presentValidSemaphore });
		//CopyImageToImage(renderTargets.colorAttachment.attachmentImage,testTexture.image, { drawSemaphore }, { presentValidSemaphore });
		//WaitIdle();
		//auto rgba = (const char*)testTexture.image.hostMapPointer;
		//auto r = rgba[0];
		//auto g = rgba[1];
		//auto b = rgba[2];
		//auto a = rgba[3];

		Present({ presentValidSemaphore }, { presentFinishSemaphore }, nexIndex);
		int a = 10;
		//if (numCap != 0)
		//{
			;
		//	numCap--;
		//}

	}

}
