#pragma once
#include "API/VulkanAPI.h"
#include <map>
#include <string>
#include <array>
#include <spirv_glsl.hpp>
#include <spirv_cross.hpp>
struct DescriptorBinding {
	std::string name = "";
	uint32_t binding;
	uint32_t setId;
	uint32_t numDescriptor;
	VkDescriptorType descriptorType;
};

//shader info
struct ShaderResourceInfo {
	std::string shaderFilePath = "";

	std::map<uint32_t, std::vector<DescriptorBinding>> descriptorSetBindings;
	std::string entryName = "";
	std::vector<uint32_t> spirvCode;
};
struct VertexAttributeInfo {
	uint32_t location;
	uint32_t offset;
	std::string name;
	VkFormat format;
};
struct GraphicsPiplineShaderInfo {
	std::vector<VertexAttributeInfo> inputAttributesInfo;
	uint32_t vertexIputStride;
	std::map<VkShaderStageFlagBits, ShaderResourceInfo> shaderResourceInfos;

};


struct GraphicsPipelineStates {
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
	VkPipelineTessellationStateCreateInfo tessellationState;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizationState;
	VkPipelineMultisampleStateCreateInfo multisampleState;
	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	VkPipelineColorBlendStateCreateInfo colorBlendState;
	VkPipelineDynamicStateCreateInfo dynamicState;

	VkViewport viewport;
	VkRect2D scissor;
	GraphicsPipelineStates() = default;
	void Init(uint32_t width, uint32_t height) {
		//����shader stage���г�ʼ��

		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.flags = 0;
		//vertex input����ڰ���Ϣ����shader��

		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.pNext = nullptr;
		inputAssemblyState.flags = 0;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		//Ĭ�ϲ�����
		tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tessellationState.pNext = nullptr;
		tessellationState.flags = 0;
		tessellationState.patchControlPoints = 0;


		//viewport state
		viewport = VkViewport{ .x = 0,.y = 0,.width = (float)width,.height = (float)height,.minDepth = 0,.maxDepth = 1 };
		scissor = VkRect2D{ .offset = VkOffset2D{.x = 0,.y = 0},.extent = VkExtent2D{.width = width,.height = height} };
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;
		viewportState.flags = 0;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;


		//rasterization state
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.pNext = nullptr;
		rasterizationState.flags = 0;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_TRUE;
		rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationState.depthBiasEnable = VK_FALSE;
		rasterizationState.depthBiasConstantFactor = 0.0f;
		rasterizationState.depthBiasClamp = 0.0f;
		rasterizationState.depthBiasSlopeFactor = 0.0f;
		rasterizationState.lineWidth = 1.0f;

		//multisample
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.pNext = nullptr;
		multisampleState.flags = 0;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.minSampleShading = 1.0f;
		VkSampleMask sampleMask = 0x00000000;
		multisampleState.pSampleMask = &sampleMask;
		multisampleState.alphaToCoverageEnable = VK_FALSE;
		multisampleState.alphaToOneEnable = VK_FALSE;

		//depth stencil
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.pNext = nullptr;
		depthStencilState.flags = 0;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_TRUE;
		depthStencilState.stencilTestEnable = VK_FALSE;
		depthStencilState.front;//�Ȳ���stancil�Ĳ���
		depthStencilState.back;//�Ȳ���stancil�Ĳ���
		depthStencilState.minDepthBounds = 0.0f;
		depthStencilState.maxDepthBounds = 1.0f;

		//color blend Ĭ�ϲ�����
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.pNext = nullptr;
		colorBlendState.flags = 0;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.logicOp = VK_LOGIC_OP_AND;
		colorBlendState.attachmentCount = 0;
		colorBlendState.pAttachments = nullptr;
		colorBlendState.blendConstants[4];

		//dynamic state  Ĭ�ϲ�����
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = nullptr;
		dynamicState.flags = 0;
		dynamicState.dynamicStateCount = 0;
		dynamicState.pDynamicStates = nullptr;

	}

};
struct DescriptorSetInfo {
	VkDescriptorSetLayout setLayout;
	VkDescriptorSet descriptorSet;
};
struct GraphicPipelineInfos {
	GraphicsPiplineShaderInfo pipelineShaderResourceInfo;
	GraphicsPipelineStates pipelineStates;

	VkDescriptorPool descriptorPool;
	std::map<uint32_t, DescriptorSetInfo> descriptorSetInfos;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;


};
struct Buffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	void* hostMapPointer = nullptr;
};

struct Image {
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory memory;
	VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	uint32_t numLayer = 0, numMip = 1;
	VkImageTiling tiling = VK_IMAGE_TILING_LINEAR;
	VkExtent3D extent;
	VkSampleCountFlagBits sample = VK_SAMPLE_COUNT_1_BIT;
	VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
};

struct Texture {
	Image image;
	VkSampler sampler;

};
struct Geometry
{
	Buffer vertexBuffer;
	std::vector<Buffer> indexBuffers;//һ��indexbuffer ����ģ���е�һ������
	 


};

enum VertexAttributeType {
	VAT_Position_float32,//x,y,z float
	VAT_TextureCoordinates_float32,//u,v,w
	VAT_Normal_float32,//nx,ny,nz
	VAT_Color_float32,//r,g,b
	VAT_AUX1_float32,//��������1,x,y,z
	VAT_AUX2_float32//��������2,x,y,z
};

struct Attachment {
	VkAttachmentDescription attachmentDesc;
	Image attachmentImage;
};
struct RenderTargets {
	Attachment colorAttachment;//render pass��0�Ÿ���
	Attachment depthAttachment;//render pass��1�Ÿ���
	const VkAttachmentReference colorRef{ .attachment = 0,.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, depthRef{ .attachment = 1,.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
};

struct SubpassInfo {
	std::vector<VkSubpassDescription> subpassDescs;
	std::vector<VkSubpassDependency> subpassDepends;

		

};

class ExampleBase {
	//



public:
	ExampleBase() = default;
	static void Run(ExampleBase* example);

protected:
	void Init();
	virtual void InitResources() = 0;//��ʼ����Ҫ����Դ
	virtual void Loop() = 0;//��Ⱦѭ��
	struct ShaderCodePaths {
		std::string vertexShaderPath = "";
		std::string geometryShaderPath = "";
		std::string fragmentShaderPath = "";
	};
	void ParseShaderFiles(const std::vector<ShaderCodePaths>& shaderPaths);

protected:
	void Draw();
	void Present();


protected:
	//virtual void InitTextures() = 0;
	virtual void InitSubPassInfo() = 0;
	virtual void InitContex();
	virtual void InitAttanchmentDesc();
	virtual void InitRenderPass();
	virtual void InitFrameBuffer();
	void InitDefaultGraphicSubpassInfo();
	virtual void InitGraphicPipelines();

protected:
	//utils
	int32_t GetPhysicalDeviceSurportGraphicsQueueFamilyIndex(VkPhysicalDevice physicalDevice);
	void PickValidPhysicalDevice();
	int32_t GetMemoryTypeIndex();
	Image CreateImage(VkImageType imageType, VkImageViewType viewType, VkFormat format, uint32_t width, uint32_t height,uint32_t depth, uint32_t numMip, uint32_t numLayer, VkImageUsageFlags usage, VkImageAspectFlags aspect, VkImageTiling tiling = VK_IMAGE_TILING_LINEAR, VkSampleCountFlagBits sample = VK_SAMPLE_COUNT_1_BIT,VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED);
	
	
	Texture Load2DTexture(const std::string& texFilePath);
	Texture LoadCubeTexture(const std::array<std::string, 6>& faceTexFilePaths/*+x,-x,+y,-y,+z,-z*/);
	Geometry LoadObj(const std::string& objFilePath);
	
	Buffer CreateVertexBuffer(const char* buf, VkDeviceSize size);
	Buffer CreateIndexBuffer(const char* buf, VkDeviceSize size);
	void FillBuffer(Buffer buffer, VkDeviceSize offset, VkDeviceSize size, const char* data);

	void TransferWholeImageLayout(Image& image, VkImageLayout dstImageLayout);
	void TransferImageLayout(VkImage image, VkImageLayout srcImageLayout, VkImageLayout dstImageLayout, VkImageSubresourceRange subRange);

private:



private:
	
	//shader parse
	void ReadGLSLShaderFile(const std::string& shaderPath, std::vector<char>& shaderCode);
	bool CompileGLSLToSPIRV(VkShaderStageFlagBits shaderStage, const std::vector<char>& srcCode, std::vector<uint32_t>& outSpirvCode);
	void ParseVertexAttributes(const spirv_cross::ShaderResources& srcShaderResource, const spirv_cross::CompilerGLSL& compiler, std::vector<VertexAttributeInfo>& dstCacheAttributeInfo);
	void ParseShaderResource(const spirv_cross::ShaderResources& srcShaderResource, const spirv_cross::CompilerGLSL& compiler, ShaderResourceInfo& dstCacheShaderResource);
	void TransferGLSLFileToSPIRVFile(const std::string& srcGLSLFile, const std::string& dstSPIRVFile);
	void ReadSPIRVFile(const std::string& spirvFile,std::vector<uint32_t>& outSpirvCode);

	//create descriptor set layout

	//create pipeline layout

	//create graphics pipeline


	//utils

private:

	bool initFlag = false;

	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugUtilMessager = VK_NULL_HANDLE;
	
	VkPhysicalDeviceProperties physicalDeviceProps;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	uint32_t windowWidth = 512, windowHeight = 512;
	GLFWwindow* window = nullptr;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkColorSpaceKHR colorSpace;
	const VkFormat colorFormat = VK_FORMAT_R8G8B8A8_SRGB;//��srgb����ĸ�ʽ
	const VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;//ֻ�����ֵ�ĸ�ʽ
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	std::vector<Image> swapchainImages;
	std::vector<VkSemaphore> swapchainImageValidSemaphores;



	std::map<std::string, Texture> textures;

	const std::array<VertexAttributeType, 6> vertexAttributes = {
		VAT_Position_float32,//location 0
		VAT_Normal_float32,//location 1
		VAT_Color_float32,//location 2
		VAT_TextureCoordinates_float32,//location 3
		VAT_AUX1_float32,//location 4
		VAT_AUX2_float32//location 5
	};//


	int32_t usedMemoryTypeIndex;

	VkSemaphore transferOperationFinish;
	const uint32_t vertexAttributeInputStride = 3 * vertexAttributes.size() * sizeof(float);

	//先不考虑compute pipeline
	std::vector<GraphicPipelineInfos> graphcisPipelineInfos;

	//render pass ֻ
	RenderTargets renderTargets;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	SubpassInfo subpassInfo;
	VkFramebuffer frameBuffer = VK_NULL_HANDLE;

	Geometry geom;



	VkQueue graphicQueue = VK_NULL_HANDLE,presentQueue = VK_NULL_HANDLE,transferQueue = VK_NULL_HANDLE;
	VkFence graphicFence = VK_NULL_HANDLE,presentFence = VK_NULL_HANDLE, transferFence = VK_NULL_HANDLE;
	uint32_t queueFamilyIndex = 0;




	//command
	VkCommandPool commandPool = VK_NULL_HANDLE;
	VkCommandBuffer renderCommandBuffer = VK_NULL_HANDLE, toolCommandBuffer = VK_NULL_HANDLE;





};