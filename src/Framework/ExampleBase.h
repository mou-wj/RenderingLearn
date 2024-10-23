#pragma once
#include "API/VulkanAPI.h"
#include <map>
#include <string>
#include <spirv_glsl.hpp>
#include <spirv_cross.hpp>

class ExampleBase {
public:
	ExampleBase() = default;
	void Init();
	struct ShaderCodePaths {
		std::string vertexShaderPath = "";
		std::string geometryShaderPath = "";
		std::string fragmentShaderPath = "";
	};
	void ParseShaderFiles(std::vector<ShaderCodePaths>& shaderPaths);
	virtual void InitShaderStates() = 0;
	virtual void InitFrameBuffer() = 0;
	virtual void InitRenderPass() = 0;

private:
	void InitShaderStage();
	void CreateGraphicPipelines();


private:
	//shader parse
	void ReadGLSLShaderFile(const std::string& shaderPath,std::vector<char>& shaderCode);
	bool CompileGLSLToSPIRV(VkShaderStageFlagBits shaderStage, const std::vector<char>& srcCode, std::vector<uint32_t>& outSpirvCode);
	struct VertexAttributeInfo;
	void ParseVertexAttributes(const spirv_cross::ShaderResources& srcShaderResource, const spirv_cross::CompilerGLSL& compiler, std::vector<VertexAttributeInfo>& dstCacheAttributeInfo);
	struct ShaderResourceInfo;
	void ParseShaderResource(const spirv_cross::ShaderResources& srcShaderResource, const spirv_cross::CompilerGLSL& compiler, ShaderResourceInfo& dstCacheShaderResource);

private:
	
	

	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	uint32_t windowWidth = 512, windowHeight = 512;
	GLFWwindow* window = nullptr;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> swapchainImages;
	
	//
	struct Texture {
		VkImage image;
		VkImageView imageView;
		VkImageLayout imageCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	};
	std::map<std::string, Texture> textures;


	//shader info
	struct ShaderResourceInfo {
		std::string shaderFilePath = "";
		struct DescriptorBinding {
			std::string name = "";
			uint32_t binding;
			uint32_t setId;
			uint32_t numDescriptor;
			VkDescriptorType descriptorType;
		};
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
		
		std::map<VkShaderStageFlagBits, ShaderResourceInfo> shaderResourceInfos;

	};
	std::vector<GraphicsPiplineShaderInfo> pipelineShaderResourceInfos;


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
		GraphicsPipelineStates(uint32_t width,uint32_t height) {
			//不对shader stage进行初始化

			vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputState.flags  = 0;
			//vertex input的入口绑定信息根据shader来

			inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyState.pNext = nullptr;
			inputAssemblyState.flags = 0;
			inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssemblyState.primitiveRestartEnable = VK_FALSE;

			//默认不开启
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
			depthStencilState.front;//先不管stancil的操作
			depthStencilState.back;//先不管stancil的操作
			depthStencilState.minDepthBounds = 0.0f;
			depthStencilState.maxDepthBounds = 1.0f;

			//color blend 默认不开启
			colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendState.pNext = nullptr;
			colorBlendState.flags = 0;
			colorBlendState.logicOpEnable = VK_FALSE;
			colorBlendState.logicOp = VK_LOGIC_OP_AND;
			colorBlendState.attachmentCount = 0;
			colorBlendState.pAttachments = nullptr;
			colorBlendState.blendConstants[4];

			//dynamic state  默认不开启
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.pNext = nullptr;
			dynamicState.flags = 0;
			dynamicState.dynamicStateCount = 0;
			dynamicState.pDynamicStates = nullptr;

		}

	};

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkAttachmentDescription> attachments;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkFramebuffer frameBuffer = VK_NULL_HANDLE;

	//先不管compute pipeline
	std::vector<VkPipelineLayout> graphicsPipelineLayout;
	std::vector<GraphicsPipelineStates> graphicsPipelineStates;
	std::vector<VkPipeline> graphicsPipelines;





	//command
	VkCommandPool commandPool = VK_NULL_HANDLE;
	VkCommandBuffer renderCommandBuffer = VK_NULL_HANDLE,toolCommandBuffer = VK_NULL_HANDLE;
	






};