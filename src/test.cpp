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
