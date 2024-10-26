#include "DrawSimpleTriangleSample.h"

void DrawSimpleTriangleSample::InitSubPassInfo()
{
	InitDefaultGraphicSubpassInfo();


}

void DrawSimpleTriangleSample::InitResources()
{
	ShaderCodePaths shaderCodePath;
	shaderCodePath.vertexShaderPath = std::string(PROJECT_DIR) + "/resources/shader/glsl/DrawSimpleTriangle.vert";
	shaderCodePath.fragmentShaderPath = std::string(PROJECT_DIR) + "/resources/shader/glsl/DrawSimgleTriangle.frag";
	ParseShaderFiles({ shaderCodePath });



}
