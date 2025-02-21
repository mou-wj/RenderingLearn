#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/Include/ResourceLimits.h"
#include "glslang/Public/resource_limits_c.h"
#include "Framework/Utils/RenderDocTool.h"

#include <shaderc/shaderc.h>
#include <fstream>
#include <iostream>
#include <spirv_glsl.hpp>
#include <spirv_cross.hpp>

#include "tiny_gltf.h"
#include "stb_image_write.h"


void LoadGLBTest() {

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	const std::string& glbFilePath = std::string(PROJECT_DIR) + "/resources/GLB/metallic_barrel_with_lod.glb";
	std::string err;
	bool res = loader.LoadBinaryFromFile(&model, &err, nullptr, glbFilePath);
	int a = 10;
	model.accessors;

	// 假设访问第一个网格的第一个图元
	const tinygltf::Mesh& mesh = model.meshes[0];
	const tinygltf::Primitive& primitive = mesh.primitives[0];

	// 查找位置属性
	auto it = primitive.attributes.find("POSITION");
	if (it == primitive.attributes.end()) {
		std::cerr << "POSITION attribute not found!" << std::endl;
		return;
	}

	// 获取位置属性的访问器
	int positionAccessorIndex = it->second;
	const tinygltf::Accessor& positionAccessor = model.accessors[positionAccessorIndex];

	// 获取缓冲区视图和缓冲区
	const tinygltf::BufferView& bufferView = model.bufferViews[positionAccessor.bufferView];
	const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

	// 提取顶点数据
	const float* positions = reinterpret_cast<const float*>(
		&buffer.data[bufferView.byteOffset + positionAccessor.byteOffset]
		);

	size_t vertexCount = positionAccessor.count; // 顶点数量
	size_t componentCount = tinygltf::GetNumComponentsInType(positionAccessor.type); // 每个顶点的组件数量

	std::cout << "Vertex Positions:" << std::endl;
	for (size_t i = 0; i < vertexCount; ++i) {
		float x = positions[i * componentCount + 0];
		float y = positions[i * componentCount + 1];
		float z = positions[i * componentCount + 2];
		std::cout << "  Vertex " << i << ": (" << x << ", " << y << ", " << z << ")" << std::endl;
	}

	//for (size_t i = 0; i < model.textures.size(); ++i) {
	//	const auto& texture = model.textures[i];
	//	const auto& image = model.images[texture.source];
	//	auto name = model.bufferViews[image.bufferView].name;
	//	std::string texture_filename = name + ".jpg";

	//	stbi_write_jpg(texture_filename.c_str(), image.width, image.height, image.component, reinterpret_cast<const char*>(image.image.data()), 100);
	//
	//}



}


void GLSL2SPIRV()
{
	std::string vulkanIncludeDir(VULKAN_INCLUDE_DIRS);
	uint32_t pos = vulkanIncludeDir.find_last_of("/");
	std::string vulkanInstallDir = vulkanIncludeDir.substr(0, pos);
	std::string glslcDir = vulkanInstallDir + "/Bin/glslc.exe";
	std::string tmpSpvDir = "tmp.spv";
	std::string generateCmd = glslcDir + " " + std::string(std::string(PROJECT_DIR) + "/resources/shader/glsl/DrawSimpleTriangle.vert") + " -o " + tmpSpvDir;
	int ret = system(generateCmd.c_str());
	if (ret != 0)
	{
		//THROW_ERROR;
	}

	std::ifstream spvfile(tmpSpvDir, std::ios::ate | std::ios::binary);


	if (!spvfile.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	std::vector<char> spvShaderCode;
	size_t spvfileSize = (size_t)spvfile.tellg();
	spvShaderCode.resize(spvfileSize);

	spvfile.seekg(0);
	spvfile.read(spvShaderCode.data(), spvfileSize);
	spvfile.close();

	std::vector<uint32_t> spirvCode32(
		reinterpret_cast<uint32_t*>(spvShaderCode.data()),
		reinterpret_cast<uint32_t*>(spvShaderCode.data() + spvShaderCode.size()));
	//spirv_cross::Compiler shaderCompiler(spirvCode32);
	spirv_cross::CompilerGLSL shaderCompiler(spirvCode32);


	std::string suffix = "";
	
	std::ifstream file(std::string(PROJECT_DIR) + "/resources/shader/glsl/DrawSimpleTriangle.vert", std::ios::ate | std::ios::binary);


	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	std::vector<char> shaderCode;
	size_t fileSize = (size_t)file.tellg();
	shaderCode.resize(fileSize);

	file.seekg(0);
	file.read(shaderCode.data(), fileSize);
	shaderCode.push_back('\0');
	file.close();

	std::string shaderCodeStr(shaderCode.begin(), shaderCode.end());


	std::string vertexCode = "#version 450\n"\
		"void main(){\n"\
		"gl_Position = vec4(1.0,1.0,1.0,1.0);\n"\
		"}"
		;

	EShLanguage shaderType = EShLangVertex;

	// ��ʼ�� glslang
	glslang::InitializeProcess();

	// ���� GLSL ����Ϊ SPIR-V ����
	glslang::TShader shader(shaderType); // ���磬���������һ��������ɫ��
	const char* source = shaderCodeStr.data();
	shader.setStrings(&source, 1);
	std::cout << "shader compile " << std::endl << std::string(shaderCode.begin(), shaderCode.end()) << std::endl << std::endl;;
	if (!shader.parse(reinterpret_cast<const TBuiltInResource*>(glslang_default_resource()), 110, false, EShMessages::EShMsgDefault)) {
		std::cerr << "Failed to parse GLSL shader!" << std::endl;
		std::cerr << shader.getInfoLog() << std::endl;
		std::cerr << shader.getInfoDebugLog() << std::endl;
		glslang::FinalizeProcess();
		return ;
	}

	glslang::TProgram program;
	program.addShader(&shader);

	if (!program.link(EShMsgDefault)) {
		std::cerr << "Failed to link GLSL shader!" << std::endl;
		std::cerr << program.getInfoLog() << std::endl;
		std::cerr << program.getInfoDebugLog() << std::endl;
		glslang::FinalizeProcess();

		return ;
	}
	std::vector<uint32_t> outSpirvCode;
	glslang::GlslangToSpv(*program.getIntermediate(shaderType), outSpirvCode);



	// ���� glslang
	glslang::FinalizeProcess();

}

void RenderDocCapTest() {
	CaptureOutPathSetMacro(std::string(PROJECT_DIR) + "/capture.rdc")
	CaptureBeginMacro
	if (IsRenderDocCapturing)
	{
		std::cout << "sss";
	}
	CaptureEndMacro


}
void TransferGLSLToSpirv(const std::string& srcGLSLFile,const std::string& outSpirvFile){

    std::string vulkanIncludeDir(VULKAN_INCLUDE_DIRS);
    uint32_t pos = vulkanIncludeDir.find_last_of("/");
    std::string vulkanInstallDir = vulkanIncludeDir.substr(0, pos);
    std::string glslcDir = vulkanInstallDir + "/Bin/glslc.exe";
    std::string generateCmd = glslcDir + " " + srcGLSLFile + " -o " + outSpirvFile;
    int ret = system(generateCmd.c_str());
    if (ret != 0)
    {
        //LogFunc(0);
    }

}
