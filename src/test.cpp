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

#include "stb_image_write.h"



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

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

int VulkanInitTest() {


	// 初始化GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW!" << std::endl;
		return -1;
	}

	// 设置GLFW不使用OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);

	// 初始化Vulkan
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Example";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// 获取GLFW所需的扩展
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> instanceExtensions;
	instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	instanceExtensions.push_back(glfwExtensions[0]);
	instanceExtensions.push_back(glfwExtensions[1]);

	std::vector<const char*> instanceLayers{ "VK_LAYER_KHRONOS_validation" };

	createInfo.enabledExtensionCount = 3;
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();
	createInfo.enabledLayerCount = 1;
	createInfo.ppEnabledLayerNames = instanceLayers.data();
	// 创建Vulkan实例
	VkInstance instance;
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		std::cerr << "Failed to create Vulkan instance!" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	// 列举可用的Vulkan扩展
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "Available Vulkan extensions:" << std::endl;
	for (const auto& extension : extensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}


	// 枚举物理设备
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		std::cerr << "No Vulkan-supported GPUs found!" << std::endl;
		vkDestroyInstance(instance, nullptr);
		glfwTerminate();
		return -1;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// 选择第一个物理设备并列举扩展
	VkPhysicalDevice physicalDevice = devices[1];
	extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
	extensions.resize(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

	std::cout << "Device extensions:" << std::endl;
	for (const auto& extension : extensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = 0;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR physicalDeviceRayTracingPipelineFeaturesKHR{};
	VkPhysicalDeviceVulkanMemoryModelFeatures physicalDeviceVulkanMemoryModelFeatures{};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR physicalDeviceAccelerationStructureFeaturesKHR{};
	
	VkPhysicalDeviceFeatures2 physicalDeviceFeatures2;
	physicalDeviceFeatures2.sType= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	physicalDeviceFeatures2.pNext = &physicalDeviceRayTracingPipelineFeaturesKHR;
	physicalDeviceRayTracingPipelineFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	physicalDeviceRayTracingPipelineFeaturesKHR.pNext = &physicalDeviceVulkanMemoryModelFeatures;


	physicalDeviceVulkanMemoryModelFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES;
	physicalDeviceVulkanMemoryModelFeatures.pNext = &physicalDeviceAccelerationStructureFeaturesKHR;




	physicalDeviceAccelerationStructureFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	physicalDeviceAccelerationStructureFeaturesKHR.pNext = VK_NULL_HANDLE;



	vkGetPhysicalDeviceFeatures2(physicalDevice, &physicalDeviceFeatures2);
	
	std::vector<const char*> deviceExtensions;
	deviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
	deviceExtensions.push_back(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
	deviceExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
	deviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
	deviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size(); // 无扩展
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.pNext = &physicalDeviceFeatures2;
	const VkBaseInStructure* base = reinterpret_cast<const VkBaseInStructure*>(deviceCreateInfo.pNext);
	VkDevice device;
	auto res = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
	if (res != VK_SUCCESS) {
		std::cerr << "Failed to create logical device!" << std::endl;
		vkDestroyInstance(instance, nullptr);
		glfwTerminate();
		return -1;
	}
	vkDestroyDevice(device, nullptr);


	// 清理
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;





}