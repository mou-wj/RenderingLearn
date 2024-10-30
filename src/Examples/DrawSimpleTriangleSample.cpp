#include "DrawSimpleTriangleSample.h"

void DrawSimpleTriangleSample::InitSubPassInfo()
{
	InitDefaultGraphicSubpassInfo();


}

void DrawSimpleTriangleSample::InitResourceInfos()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/resources/shader/glsl/DrawSimpleTriangle.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/resources/shader/glsl/DrawSimgleTriangle.frag";
	ParseShaderFiles({ shaderCodePath });
	//LoadObj(std::string(PROJECT_DIR) + "/resources/obj/cube.obj",geom);
	geoms.resize(1);
	auto& geom = geoms[0];
	geom.vertexAttrib.vertices = {
		-1,1,0,
		1,1,0,
		0,0,0
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
	geom.shapes.push_back(triangle);




}

void DrawSimpleTriangleSample::Loop()
{
	uint32_t i = 0;;
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/test.rdc");
	while (1)
	{
		i++;
		//确保presentFence在创建时已经触发
		DrawGeom({}, { drawSemaphore });
		auto nexIndex = GetNextPresentImageIndex(swapchainImageValidSemaphore);
		CopyImageToImage(renderTargets.colorAttachment.attachmentImage, swapchainImages[nexIndex], { swapchainImageValidSemaphore,drawSemaphore }, { presentValidSemaphore });
		Present({ presentValidSemaphore }, {presentFinishSemaphore}, nexIndex);

	}

}
