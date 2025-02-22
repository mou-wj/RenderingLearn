#pragma once
#include "API/VulkanAPI.h"
#include "API/VulkanDefMap.h"
#include "Utils/tiny_obj_loader.h"
#include "Utils/RenderDocTool.h"
#include "Utils/WindowEventHandler.h"
#include "Common/Transform.h"
#include "Common/GlmShowTool.hpp"
#include "Utils/ImageFileTool.h"
#include "Utils/Camera.h"
#include <map>
#include <set>
#include <string>
#include <array>
#include "spirv_glsl.hpp"
#include "spirv_cross.hpp"
#include <algorithm>

#include <regex>

struct DescriptorBinding {
	std::string name = "";
	uint32_t binding;
	uint32_t setId;
	uint32_t numDescriptor;
	uint32_t inputAttachmentIndex;
	VkDescriptorType descriptorType;
};

//shader info
struct ShaderResourceInfo {
	std::string shaderFilePath = "";

	std::map<uint32_t, std::vector<DescriptorBinding>> descriptorSetBindings;
	std::string entryName = "";
	std::vector<uint32_t> spirvCode;
};
struct ShaderInputAttributeInfo {
	uint32_t location;
	uint32_t offset;
	std::string name;
	VkFormat format;
};
using VertexAttributeInfo = ShaderInputAttributeInfo;
struct GraphicsPiplineShaderInfo {
	std::vector<VertexAttributeInfo> inputAttributesInfo;
	uint32_t vertexIputStride;
	std::map<VkShaderStageFlagBits, ShaderResourceInfo> shaderResourceInfos;

};


struct Buffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDeviceSize size = 0;
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
	VkMemoryPropertyFlags memoryPropties = VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM;
	void* hostMapPointer = nullptr;
};

struct Image {
	VkImage image;
	VkImageView imageView;
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	uint64_t totalMemorySize = 0;
	VkDeviceMemory memory;
	VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	uint32_t numLayer = 1, numMip = 1;
	VkImageTiling tiling = VK_IMAGE_TILING_LINEAR;
	VkExtent3D extent;
	VkSampleCountFlagBits sample = VK_SAMPLE_COUNT_1_BIT;
	VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	std::map<uint32_t, std::map<uint32_t, VkSubresourceLayout>> layerMipLayouts;
	void* hostMapPointer = nullptr;
	VkFormatInfo GetFormatInfo() {
		return VkFormatToInfo[format];
	
	}

	uint32_t GetFormatNumBits(VkFormat format) {
		switch (format)
		{
		case VK_FORMAT_R8G8B8A8_SRGB:;
		case VK_FORMAT_B8G8R8A8_SRGB:;
		case VK_FORMAT_D32_SFLOAT:
			return 32;
		case VK_FORMAT_MAX_ENUM:

			break;
		default:
			break;
		}
		assert(0);
		return 0;

	}
	void CopyToOther(Image& dstImage, uint32_t layer, uint32_t mip)
	{
		//检查格式和长宽是否匹配
		auto dstSubLayout = dstImage.layerMipLayouts[layer][mip];
		auto dstTexelByteSize = VkFormatToInfo[dstImage.format].totalBytesPerPixel;
		auto dstSubW = dstSubLayout.rowPitch / dstTexelByteSize;
		auto dstSubH = dstSubLayout.size / dstSubLayout.rowPitch;

		auto srcSubLayout = layerMipLayouts[layer][mip];
		auto srcTexelByteSize = VkFormatToInfo[format].totalBytesPerPixel;
		auto srcSubW = srcSubLayout.rowPitch / srcTexelByteSize;
		auto srcSubH = srcSubLayout.size / srcSubLayout.rowPitch;
		if ((srcTexelByteSize == dstTexelByteSize)
			&& (srcSubW == dstSubW)
			&& (srcSubH == dstSubH)
			&& (image != dstImage.image)
			)
		{
			std::memcpy((char*)dstImage.hostMapPointer + dstSubLayout.offset, (char*)hostMapPointer + srcSubLayout.offset, dstSubW * dstSubH * dstTexelByteSize);


		}
	
	
	
	
	}


	//手动调用翻转所有的layer的纵坐标数据
	void FlipY() {
		for (uint32_t i = 0; i < numLayer; i++)
		{
			for (uint32_t j = 0; j < numMip; j++)
			{
				FlipY(i, j);

			}
		}
	

	
	}
	void FlipY(uint32_t layer, uint32_t mip)
	{
		auto layout = layerMipLayouts[layer][mip];
		auto texelByteSize = VkFormatToInfo[format].totalBytesPerPixel;
		if (memory)
		{
			auto layoutWidth = layout.rowPitch / texelByteSize;
			auto layoutHeight = layout.size / layout.rowPitch;
			std::vector<char> tmpBuffer(texelByteSize);
			size_t offset1 = 0, offset2 = 0;
			for (uint32_t col = 0; col < layoutWidth; col++)
			{
				for (uint32_t raw = 0; raw < layoutHeight/2; raw++)
				{
					offset1 = raw * layout.rowPitch + col * texelByteSize;
					offset2 = (layoutHeight - 1 - raw) * layout.rowPitch + col * texelByteSize;
					
					
					std::memcpy(tmpBuffer.data(), (char*)hostMapPointer + layout.offset + offset1, texelByteSize);
					std::memcpy((char*)hostMapPointer + layout.offset + offset1, (char*)hostMapPointer + layout.offset + offset2, texelByteSize);
					std::memcpy((char*)hostMapPointer + layout.offset + offset2, tmpBuffer.data(), texelByteSize);

				}


			}




		}


	
	
	
	}
	bool WholeCopyCompatible(const Image& other) {
		return  numLayer == other.numLayer && numMip == other.numMip && aspect == other.aspect &&
			extent.width == other.extent.width && extent.height == other.extent.height && extent.depth == other.extent.depth && ChechFormatSizeCompatible(other.format);
	}
	bool WholeBlitCompatible(const Image& other)
	{
		return  numLayer == other.numLayer && numMip == other.numMip && aspect == other.aspect &&
			ChechFormatSizeCompatible(other.format);
	}
	bool ChechFormatSizeCompatible(VkFormat otherFormat)
	{
		return GetFormatNumBits(format) == GetFormatNumBits(otherFormat);

	}
	VkExtent3D GetMipLevelExtent(uint32_t mipLevel)
	{
		VkExtent3D size;
		size.width = extent.width / pow(2, mipLevel) >= 1 ? extent.width / pow(2, mipLevel) : 1;
		size.height = extent.height / pow(2, mipLevel) >= 1 ? extent.height / pow(2, mipLevel) : 1;
		size.depth = extent.depth / pow(2, mipLevel) >= 1 ? extent.depth / pow(2, mipLevel) : 1;
		return size;
	}
	void WriteToJpg(const std::string& outJpgFile,uint32_t layer,uint32_t mip) {
		uint32_t numChannels = VkFormatToInfo[format].componentCount;
		uint32_t numBytesPerChannel = VkFormatToInfo[format].componentSizes[0];//假设所有分量的字节数都相同
		if (numChannels == 0)
		{
			Log("未知格式",0);
		}
		uint32_t texelByteWidth = numBytesPerChannel* numChannels;
		auto layout = layerMipLayouts[layer][mip];
		uint32_t w = layout.rowPitch / texelByteWidth;
		uint32_t h = layout.size / layout.rowPitch;

		WriteJpeg(outJpgFile, (const char*)(hostMapPointer )+layout.offset, w, h,numChannels);
	}
	void WriteToJpgFloat(const std::string& outJpgFile, uint32_t layer, uint32_t mip) {
		uint32_t numChannels = VkFormatToInfo[format].componentCount;
		uint32_t numBytesPerChannel = VkFormatToInfo[format].componentSizes[0];//假设所有分量的字节数都相同
		if (numChannels == 0)
		{
			Log("未知格式", 0);
		}
		uint32_t texelByteWidth = numBytesPerChannel * numChannels;
		auto layout = layerMipLayouts[layer][mip];
		uint32_t w = layout.rowPitch / texelByteWidth;
		uint32_t h = layout.size / layout.rowPitch;

		WriteJpeg(outJpgFile, (const float*)((char*)(hostMapPointer)+layout.offset), w, h,numChannels);
	}

};

struct Barrier {
	VkImageMemoryBarrier imageMemoryBarrier;
	VkBufferMemoryBarrier bufferMemoryBarrier;
	Barrier() {
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.pNext = nullptr;
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = 0;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = nullptr;
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.layerCount = 0;
		imageMemoryBarrier.subresourceRange.levelCount = 0;


		bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferMemoryBarrier.pNext = nullptr;
		bufferMemoryBarrier.srcAccessMask;
		bufferMemoryBarrier.dstAccessMask;
		bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferMemoryBarrier.buffer = VK_NULL_HANDLE;
		bufferMemoryBarrier.offset = 0;
		bufferMemoryBarrier.size = 0;

	}
	const VkImageMemoryBarrier& ImageBarrier(Image& image,VkAccessFlags srcAccess, VkAccessFlags dstAccess,VkImageLayout dstImageLayout,int layer = -1/*-1表示所有，否则表示指定的所有layer*/,int mip = -1/*-1表示所有，否则表示指定的所有miplevel*/)
	{
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.pNext = nullptr;
		imageMemoryBarrier.srcAccessMask = srcAccess;
		imageMemoryBarrier.dstAccessMask = dstAccess;
		imageMemoryBarrier.oldLayout = image.currentLayout;
		imageMemoryBarrier.newLayout = dstImageLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = image.image;
		imageMemoryBarrier.subresourceRange.aspectMask = image.aspect;
		if (layer == -1)
		{
			imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
			imageMemoryBarrier.subresourceRange.layerCount = image.numLayer;
		}
		else {
			imageMemoryBarrier.subresourceRange.baseArrayLayer = layer;
			imageMemoryBarrier.subresourceRange.layerCount = 1;
		}
		if (mip == -1)
		{
			imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
			imageMemoryBarrier.subresourceRange.levelCount = image.numMip;
		}
		else {
			imageMemoryBarrier.subresourceRange.baseMipLevel = mip;
			imageMemoryBarrier.subresourceRange.levelCount = 1;
		}
		return imageMemoryBarrier;
	}


};

struct GraphicsPipelineStates {
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
	VkPipelineTessellationStateCreateInfo tessellationState;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizationState;
	VkSampleMask sampleMask = 0xFFFFFFFF;
	VkPipelineMultisampleStateCreateInfo multisampleState;
	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
	VkPipelineColorBlendStateCreateInfo colorBlendState;
	VkPipelineDynamicStateCreateInfo dynamicState;

	VkViewport viewport;
	VkRect2D scissor;
	GraphicsPipelineStates() = default;
	void Init(uint32_t width, uint32_t height,uint32_t numColorAttachments = 1) {
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
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

		multisampleState.pSampleMask = &sampleMask;//这里如果参数是一个临时变量的地址，那么在渲染管线的多重采样阶段可能会应为错误的信息导致光栅化的片段被剔除
		multisampleState.alphaToCoverageEnable = VK_FALSE;
		multisampleState.alphaToOneEnable = VK_FALSE;

		//depth stencil
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.pNext = nullptr;
		depthStencilState.flags = 0;
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
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
		colorBlendAttachmentStates.resize(numColorAttachments);
		colorBlendState.attachmentCount = colorBlendAttachmentStates.size();
		for (uint32_t i = 0; i < colorBlendAttachmentStates.size(); i++)
		{
			colorBlendAttachmentStates[i].blendEnable = VK_FALSE;
			colorBlendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachmentStates[i].colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachmentStates[i].alphaBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachmentStates[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		}
		
		colorBlendState.pAttachments = colorBlendAttachmentStates.data();
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
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};
struct GraphicPipelineInfos {
	GraphicsPiplineShaderInfo pipelineShaderResourceInfo;
	GraphicsPipelineStates pipelineStates;

	VkDescriptorPool descriptorPool;
	std::map<uint32_t, DescriptorSetInfo> descriptorSetInfos;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	//input attachment 绑定信息，表示某个set中某个binding上对应的附件索引
	std::map<uint32_t, std::map<uint32_t, uint32_t>> setBindingAttachmentIds;


};

struct ComputeDesc {
	bool valid = false;//为true则会创建compute pipeline
	//std::string computeShaderPath = "";
	std::vector<std::string> computeShaderPaths;
};

struct ComputePipelineInfos {
	ShaderResourceInfo computeShaderResourceInfo;
	VkPipelineShaderStageCreateInfo computeShaderStage{};
	VkDescriptorPool descriptorPool;
	std::map<uint32_t, DescriptorSetInfo> descriptorSetInfos;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	ComputePipelineInfos() {
		computeShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStage.pNext = nullptr;
		computeShaderStage.flags = 0;
		computeShaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStage.module = VK_NULL_HANDLE;
		computeShaderStage.pName = nullptr;
		computeShaderStage.pSpecializationInfo = nullptr;//不指定特殊常数
	}
};


struct TextureDataSource {
	std::string picturePath = "";
	std::vector<char> imagePixelDatas;//VK_FORMAT_R8G8B8A8_SRGB
	uint32_t width = 0, height = 0;


};
struct TextureBindInfo {
	static const VkFormat defaultTextureFormat = VK_FORMAT_R8G8B8A8_UNORM;
	static const VkFormatFeatureFlags textureFormatFeatures = (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT);



	std::vector<TextureDataSource> textureDataSources;
	VkFormat format = defaultTextureFormat;//指定要创建的纹理的format
	uint32_t formatComponentByteSize = 1;//指定format每个颜色分量的字节大小，如果指定出错会导致填充image数据出问题
	VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;//天空盒的数据需要翻转Y
	uint32_t passId = 0,pipeId = 0, setId = 0, binding = 0, elementId = 0;
	bool compute = false;//是否用于compute pipeline的标志
	VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	bool buildMipmap = false;
	TextureBindInfo() = default;
	TextureBindInfo(const std::vector<std::string>& imagePaths) {
		textureDataSources.resize(imagePaths.size());
		for (uint32_t i = 0; i < imagePaths.size(); i++)
		{
			textureDataSources[i].picturePath = imagePaths[i];
		}
	}
};


struct Texture {
	Image image;
	VkSampler sampler;

};
struct Geometry
{
	std::string geoPath = "";
	tinyobj::attrib_t vertexAttrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	bool useIndexBuffers = true;

	//顶点缓冲索引缓冲
	Buffer vertexBuffer;
	std::vector<Buffer> indexBuffers;//һ��indexbuffer ����ģ���е�һ������
	uint32_t numVertex = 0;
	std::vector<uint32_t> numIndexPerZone;


	//只使用顶点缓冲
	std::vector<Buffer> shapeVertexBuffers;

	bool dynamicFlag = false;//如果为true，则使用shapeDynamicVertexBuffers进行绘制，该buffers不会在初始化的时候进行填充数据
	//
	std::vector<Buffer> shapeDynamicVertexBuffers;
	std::vector<uint32_t> dynamicNumVertexPerZone;//指明shapeDynamicVertexBuffers中要绘制顶点的数量

	struct AABB {
		float minX = 0, maxX = 0;
		float minY = 0, maxY = 0;
		float minZ = 0, maxZ = 0;

		static bool CheckAABBsIntersect(Geometry::AABB& aabb1, Geometry::AABB& aabb2) {
			return (aabb1.minX <= aabb2.maxX && aabb1.maxX >= aabb2.minX &&
				aabb1.minY <= aabb2.maxY && aabb1.maxY >= aabb2.minY &&
				aabb1.minZ <= aabb2.maxZ && aabb1.maxZ >= aabb2.minZ);
		
		}
		// 判断AABB a是否完全包含在AABB b中
		static bool IsContained(Geometry::AABB& a, Geometry::AABB& b) {
			return (a.minX >= b.minX && a.maxX <= b.maxX &&
				a.minY >= b.minY && a.maxY <= b.maxY &&
				a.minZ >= b.minZ && a.maxZ <= b.maxZ);
		}


	};

	AABB AABBs;//简单的边界包围盒
	glm::vec3 AABBcenter;

	std::vector<AABB> shadeAABBs;


	bool CloserThanOther(Geometry& other, glm::vec3 viewPos) {
		float dist = glm::distance(viewPos, glm::vec3(AABBcenter[0], AABBcenter[1], AABBcenter[2]));
		float distOther = glm::distance(viewPos, glm::vec3(other.AABBcenter[0], other.AABBcenter[1], other.AABBcenter[2]));
		return dist < distOther;
	
	}

	void InitAsScreenFillRect() {
		vertexAttrib.vertices = {
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
		shapes.push_back(triangle);

	}


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
	VkClearValue clearValue;
};
struct RenderTargets {
	uint32_t width = 512, height = 512;


	static const VkFormat colorFormat = VK_FORMAT_R8G8B8A8_SRGB;//ֻ默认的颜色附件格式，必须要支持
	static const VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;//ֻ默认的颜色附件格式，必须要支持
	static const VkFormatFeatureFlags colorAttachmentFormatFeatures = (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
	static const VkFormatFeatureFlags depthAttachmentFormatFeatures = (VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);



	std::vector<Attachment> colorAttachments = { Attachment()};
	std::vector<VkAttachmentReference> colorRefs = { VkAttachmentReference{.attachment = 0,.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } };

	bool enaleInputAttachment = false;
	std::vector<VkAttachmentReference> inputAttachmentRefs = { VkAttachmentReference{.attachment = 0,.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL } };

	Attachment depthAttachment;//render pass��1�Ÿ���
	VkAttachmentReference	depthRef{ .attachment = 1,.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };


	RenderTargets() {
		//colorAttachments.resize(1);
		InitRenderTarget(1, width, height);
	}

	void InitRenderTarget(uint32_t numColorAttachments,uint32_t rtWidth , uint32_t rtHeight) {
		ASSERT(numColorAttachments >= 1);
		colorAttachments.resize(numColorAttachments);
		colorRefs.resize(numColorAttachments);
		inputAttachmentRefs.resize(numColorAttachments);
		for (uint32_t i = 0; i < colorAttachments.size(); i++)
		{
			colorAttachments[i].attachmentDesc.format = colorFormat;//默认颜色附件格式，可以在创建附件前修改
			colorAttachments[i].attachmentImage.extent = VkExtent3D{ .width = rtWidth,.height = rtHeight,.depth = 1 };//默认颜色附件尺寸，可以在创建附件前修改
			colorAttachments[i].clearValue = VkClearValue{ 0,0,0,1 };//默认颜色附件清除值，可以在创建附件前修改
			inputAttachmentRefs[i].attachment = i;
			inputAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			colorRefs[i].attachment = i;
			colorRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		depthAttachment.attachmentDesc.format = depthFormat;//默认深度附件格式，可以在创建附件前修改
		depthAttachment.clearValue = VkClearValue{ 1.0,0 };//默认颜色附件清除值，可以在创建附件前修改
		depthAttachment.attachmentImage.extent = VkExtent3D{ .width = rtWidth,.height = rtHeight,.depth = 1 };//默认深度附件尺寸，可以在创建附件前修改
		depthRef.attachment = numColorAttachments;

	
	}
};
struct ShaderCodePaths {

	std::string taskShaderPath = "";
	std::string meshShaderPath = "";
	std::string vertexShaderPath = "";
	std::string tessellationControlShaderPath = "";
	std::string tessellationEvaluationShaderPath = "";
	std::string geometryShaderPath = "";
	std::string fragmentShaderPath = "";
};

struct SubpassDesc {
	bool enableInputAttachment = false;
	VkSubpassDescription subpassDescription;
	GraphicsPipelineStates subpassPipelineStates;
	ShaderCodePaths pipelinesShaderCodePaths;


};
struct SubpassInfo {
	std::vector<SubpassDesc> subpassDescs;
	std::vector<VkSubpassDependency> subpassDepends;
};

struct BufferBindInfo {
	uint32_t size;
	uint32_t passId = 0,pipeId = 0, setId = 0, binding = 0, elementId = 0;
	bool compute = false;//是否用于compute pipeline的标志
};

struct CommandList {
	VkCommandBuffer commandBuffer  = VK_NULL_HANDLE;
	VkFence commandFinishFence = VK_NULL_HANDLE;
	VkCommandBufferUsageFlags commandBufferUsage = 0;
};

struct SubmitSynchronizationInfo {
	std::vector<VkSemaphore> waitSemaphores;//指明要等待的信号量
	std::vector<VkPipelineStageFlags> waitStages;//指明在哪个阶段进行等待信号量
	std::vector<VkSemaphore> sigSemaphores;//指明要触发的信号量

};


struct RenderPassInfo {

	SubpassInfo subpassInfo;//子pass信息
	std::vector<GraphicPipelineInfos> graphcisPipelineInfos;//每个子pass对应的pipeline信息
	std::map<uint32_t, std::vector<uint32_t>> subpassDrawGeoInfos;//每个subpass要绘制的几何体
	bool truncateNextSubpassDraw = false;//表明是否要截断某个subpass之后的subpass的绘制
	uint32_t truncatedNextSubpassIndex = 1;//要截断的subpass id,即从某个id的subpass开始以及后续subpass不再进行绘制
	std::map<uint32_t, std::array<uint32_t,3>> subpassDrawMeshGroupInfos;//每个mesh subpas的绘制参数
	std::map<uint32_t, bool> isMeshSubpass;//该subpass是否含mesh shader

	RenderTargets renderTargets;//每个pass的渲染结果对象
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkFramebuffer frameBuffer = VK_NULL_HANDLE;

	//在这里定义一些快速设置render pass信息的接口

	//默认设置，一个默认subpass
	void InitDefaultRenderPassInfo(ShaderCodePaths drawSceenCodePath/*一个subpass需要的着色器文件路径*/,uint32_t viewportWidth/*视口宽度*/, uint32_t viewportHeight/*视口高度*/,uint32_t numColorAttachments = 1) {
	
	

		//InitDefaultGraphicSubpassInfo();
		subpassInfo.subpassDescs.resize(1);

		//设置着色器路径
		subpassInfo.subpassDescs[0].pipelinesShaderCodePaths = drawSceenCodePath;
		//初始化管线状态
		subpassInfo.subpassDescs[0].subpassPipelineStates.Init(viewportWidth, viewportHeight, numColorAttachments);

		renderTargets.InitRenderTarget(numColorAttachments, viewportWidth, viewportHeight);
		//开启剔除
		subpassInfo.subpassDescs[0].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;


		auto& subpassDesc1 = subpassInfo.subpassDescs[0];
		subpassDesc1.subpassDescription.flags = 0;
		subpassDesc1.subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc1.subpassDescription.inputAttachmentCount = 0;
		subpassDesc1.subpassDescription.pInputAttachments = nullptr;
		subpassDesc1.subpassDescription.colorAttachmentCount = renderTargets.colorAttachments.size();
		subpassDesc1.subpassDescription.pColorAttachments = renderTargets.colorRefs.data();
		subpassDesc1.subpassDescription.pResolveAttachments = nullptr;
		subpassDesc1.subpassDescription.pDepthStencilAttachment = &renderTargets.depthRef;
		subpassDesc1.subpassDescription.preserveAttachmentCount = 0;
		subpassDesc1.subpassDescription.pPreserveAttachments = nullptr;


		subpassInfo.subpassDepends.resize(1);
		auto& subpassDepend1 = subpassInfo.subpassDepends[0];
		subpassDepend1.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDepend1.dstSubpass = 0;
		subpassDepend1.dependencyFlags = 0;
		subpassDepend1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDepend1.srcAccessMask = 0;
		subpassDepend1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		subpassDepend1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	
	
	}

	void InitDefaultRenderPassInfo(const std::vector<ShaderCodePaths>& renderPassSubpassShaderPaths, uint32_t viewportWidth/*视口宽度*/, uint32_t viewportHeight/*视口高度*/, uint32_t numColorAttachments = 1, std::map<uint32_t, bool> subpassInputAttachmentEnables = {}) {
		
		//初始化render target的信息
		renderTargets.InitRenderTarget(numColorAttachments, viewportWidth, viewportHeight);

		subpassInfo.subpassDescs.resize(renderPassSubpassShaderPaths.size());
		//初始化subpass描述
		for (uint32_t subpassId = 0; subpassId < renderPassSubpassShaderPaths.size(); subpassId++)
		{
			subpassInfo.subpassDescs[subpassId].pipelinesShaderCodePaths = renderPassSubpassShaderPaths[subpassId];
			//初始化管线状态
			subpassInfo.subpassDescs[subpassId].subpassPipelineStates.Init(viewportWidth, viewportHeight, numColorAttachments);

			//开启剔除
			subpassInfo.subpassDescs[subpassId].subpassPipelineStates.rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;


			auto& subpassDesc = subpassInfo.subpassDescs[subpassId];
			subpassDesc.subpassDescription.flags = 0;
			subpassDesc.subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDesc.subpassDescription.inputAttachmentCount = 0;
			subpassDesc.subpassDescription.pInputAttachments = nullptr;
			if (subpassInputAttachmentEnables[subpassId])
			{
				renderTargets.enaleInputAttachment |= subpassInputAttachmentEnables[subpassId];
				subpassDesc.subpassDescription.inputAttachmentCount = renderTargets.inputAttachmentRefs.size();
				subpassDesc.subpassDescription.pInputAttachments = renderTargets.inputAttachmentRefs.data();
			}
			subpassDesc.subpassDescription.colorAttachmentCount = renderTargets.colorAttachments.size();
			subpassDesc.subpassDescription.pColorAttachments = renderTargets.colorRefs.data();
			subpassDesc.subpassDescription.pResolveAttachments = nullptr;
			subpassDesc.subpassDescription.pDepthStencilAttachment = &renderTargets.depthRef;
			subpassDesc.subpassDescription.preserveAttachmentCount = 0;
			subpassDesc.subpassDescription.pPreserveAttachments = nullptr;




		}

		//初始化subpass依赖
		subpassInfo.subpassDepends.resize(renderPassSubpassShaderPaths.size());
		for (uint32_t subpassId = 0; subpassId < renderPassSubpassShaderPaths.size(); subpassId++)
		{
			auto& subpassDepend = subpassInfo.subpassDepends[subpassId];
			if (subpassId == 0)
			{
				subpassDepend.srcSubpass = VK_SUBPASS_EXTERNAL;
				subpassDepend.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpassDepend.srcAccessMask = 0;
				subpassDepend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				subpassDepend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			}
			else {
				subpassDepend.srcSubpass = subpassId - 1;

				subpassDepend.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				subpassDepend.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				subpassDepend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				subpassDepend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			}

			subpassDepend.dstSubpass = subpassId;
			subpassDepend.dependencyFlags = 0;




			}
		}
	
	
	


};



class ExampleBase {
	//



public:
	ExampleBase() = default;
	~ExampleBase();
	static void Run(ExampleBase* example);

protected:

	virtual void InitResourceInfos() = 0;//��ʼ����Ҫ����Դ
	virtual void Loop() = 0;//��Ⱦѭ��


protected:
	virtual void InitComputeInfo() {}
	virtual void InitSubPassInfo() = 0;
	virtual void InitSyncObjectNumInfo() = 0;
	void InitDefaultGraphicSubpassInfo(ShaderCodePaths subpassShaderCodePaths);
	//
protected:
	void LoadObj(const std::string& objFilePath, Geometry& geo);
	//resource
	void FillImageFromDataSource(Image& image, TextureBindInfo& textureBindInfo);

	void FillBuffer(Buffer buffer, VkDeviceSize offset, VkDeviceSize size, const char* data);
protected:
	//runtime
	void CmdListReset(CommandList& cmdList);
	void CmdListRecordBegin(CommandList& cmdList);
	void CmdListRecordEnd(CommandList& cmdList);
	void CmdOpsImageMemoryBarrer(CommandList& cmdList, Image& image, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkImageLayout dstImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,int layer = -1,int mip = -1);
	//compute
	void CmdOpsDispatch(CommandList& cmdList, uint32_t computePassIndex = 0, std::array<uint32_t, 3> groupSize = {1,1,1});
	

	//transfer
	void CmdOpsCopyWholeImageToImage(CommandList& cmdList, Image& srcImage, Image& dstImage);
	void CmdOpsCopyImageToImage(CommandList& cmdList, Image& srcImage,uint32_t srcLayer,uint32_t srcMip, Image& dstImage, uint32_t dstLayer, uint32_t dstMip);
	void CmdOpsBlitWholeImageToImage(CommandList& cmdList, Image& srcImage, Image& dstImage);
	//graphic
	void CmdOpsDrawGeom(CommandList& cmdList,uint32_t renderPassIndex = 0);
	//execute
	void CmdListSubmit(CommandList& cmdList, SubmitSynchronizationInfo& info);
	void CmdListWaitFinish(CommandList& cmdList);
	VkResult Present(uint32_t imageIndex, const std::vector<VkSemaphore>& waitSemaphores);
	uint32_t GetNextPresentImageIndex(VkSemaphore sigValidSemaphore);

protected:
	//根据和相机的距离来排序要绘制的geo，小序
	void SortGeosFollowCloseDistance(glm::vec3 cameraPos);

	//runtime
	std::vector<Geometry> geoms;
	CommandList graphicCommandList;
	//SubpassInfo subpassInfo;
	ComputeDesc computeDesc;
	//std::map<uint32_t, std::vector<uint32_t>> subpassDrawGeoInfos;
	//uint32_t numFences = 1;
	uint32_t numSemaphores = 1;
	void WaitIdle();

	void WaitAllFence(const std::vector<VkFence>& fences);
	void ResetAllFence(const std::vector<VkFence>& fences);


	std::vector<VkSemaphore> semaphores;//根据不同的场景动态创建
	//std::vector<VkFence> fences;//根据不同场景创建

	std::map<std::string, TextureBindInfo> textureBindInfos;
	std::map<std::string, BufferBindInfo> bufferBindInfos;

protected:
	//这里的数据不能被派生类创建和析构
	//render pass ֻ
	//RenderTargets renderTargets;
	bool enableMeshShaderEXT = false;//是否开启mesh shader拓展
	std::vector<RenderPassInfo> renderPassInfos;
	std::vector<Image> swapchainImages;
	uint32_t windowWidth = 512, windowHeight = 512;
	std::map<std::string, Texture> textures;
	std::map<std::string, Buffer> buffers;
	void BindTexture(const std::string& textureName);
	void BindBuffer(const std::string& bufferName);


	void ResizeBuffer(Buffer& buffer,VkDeviceSize newByteSize);
private:

	void Init();
	//graphic

	virtual void InitContex();
	virtual void InitAttanchmentDesc(RenderPassInfo& renderPassInfo);
	void InitRenderPass(RenderPassInfo& renderPassInfo);
	
	
	virtual void InitRenderPasses();
	virtual void InitFrameBuffer(RenderPassInfo& renderPassInfo);
	void InitSyncObject();
	virtual void InitGraphicPipelines(RenderPassInfo& renderPassInfo);
	virtual void InitRecources();
	virtual void InitQueryPool();
	virtual void InitCommandList();
	void InitGeometryResources(Geometry& geo);
	void InitTextureResources();
	void InitUniformBufferResources();
	//compute
	void InitCompute();
	void InitComputePipeline(ComputePipelineInfos& computePipelineInfos);

private:
	virtual void Clear();
	virtual void ClearContex();
	virtual void ClearAttanchment(RenderPassInfo& renderPassInfo);
	virtual void ClearRenderPass(RenderPassInfo& renderPassInfo);
	void ClearRenderPasses();
	virtual void ClearFrameBuffer(RenderPassInfo& renderPassInfo);
	virtual void ClearGraphicPipelines(RenderPassInfo& renderPassInfo);
	virtual void ClearSyncObject();
	virtual void ClearRecources();
	virtual void ClearQueryPool();
	virtual void ClearComputePipeline();
	



private:
	//utils
	int32_t GetSuitableQueueFamilyIndex(VkPhysicalDevice physicalDevice,VkQueueFlags wantQueueFlags,bool needSupportPresent,uint32_t wantNumQueue);
	void PickValidPhysicalDevice();
	void CheckCandidateTextureFormatSupport();
	int32_t GetMemoryTypeIndex(uint32_t  wantMemoryTypeBits,VkMemoryPropertyFlags wantMemoryFlags);
	Image CreateImage(VkImageType imageType, VkImageViewType viewType, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t numMip, uint32_t numLayer, VkImageUsageFlags usage, VkImageAspectFlags aspect, VkMemoryPropertyFlags memoryProperies, VkComponentMapping viewMapping = VkComponentMapping{}, VkImageTiling tiling = VK_IMAGE_TILING_LINEAR, VkSampleCountFlagBits sample = VK_SAMPLE_COUNT_1_BIT, VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED);
	
	void FillImage(Image& image, VkDeviceSize offset, VkDeviceSize size, const char* data);
	void DestroyImage(Image& image);

	Texture CreateTexture(TextureBindInfo& textureBindInfo);

	
	Buffer CreateVertexBuffer(const char* buf, VkDeviceSize size);
	Buffer CreateIndexBuffer(const char* buf, VkDeviceSize size);
	Buffer CreateShaderAccessBuffer(const char* buf, VkDeviceSize size);
	
	void DestroyBuffer(Buffer& buffer);

	void TransferWholeImageLayout(Image& image, VkImageLayout dstImageLayout);
	void GenerateMipmap(Image& image);


	//descriptor
	VkDescriptorType GetDescriptorType(DescriptorSetInfo& descriptorSetInfo,uint32_t binding);
	void BindTexture(const Texture& texture, VkDescriptorSet set, uint32_t binding, uint32_t elemenId ,VkDescriptorType descriptorType);
	void BindBuffer(const Buffer& uniformBuffer, VkDescriptorSet set, uint32_t binding, uint32_t elemenId, VkDescriptorType descriptorType);

	void BindInputAttachment(RenderPassInfo& renderPassInfo);


	void CreateSemaphores(uint32_t numSemaphores);
	//void CreateFences(uint32_t numFences);

	//info set tools
	void SetSupassDescription(VkSubpassDescription &subpassDesc,
		VkSubpassDescriptionFlags       flags,
		VkPipelineBindPoint             pipelineBindPoint,
		const std::vector<VkAttachmentReference>& inputAttachmentRefs,//在创建subpass的时候不能被销毁
		const std::vector<VkAttachmentReference>& colorAttachmentRefs,//在创建subpass的时候不能被销毁
		const VkAttachmentReference* resolveAttachments,//在创建subpass的时候不能被销毁
		const VkAttachmentReference* depthStencilAttachment,//在创建subpass的时候不能被销毁
		const std::vector<uint32_t>& preserveAttachments//在创建subpass的时候不能被销毁
	);


	//check
	bool CheckLinearFormatFeatureSupport(VkPhysicalDevice curPhysicalDevive, VkFormat format,VkFormatFeatureFlags features);
	bool CheckOptimalFormatFeatureSupport(VkPhysicalDevice curPhysicalDevive, VkFormat format, VkFormatFeatureFlags features);
	void PrintSupportedFormatFeatures(VkFormatFeatureFlags features);
	void FindFormat(VkPhysicalDevice curPhysicalDevive, VkFormatFeatureFlags features);

	bool CheckExtensionSupport(VkPhysicalDevice curPhysicalDevive, const std::vector<const char*> entensions);

private:
	
	//shader parse
	void ReadGLSLShaderFile(const std::string& shaderPath, std::vector<char>& shaderCode);
	bool CompileGLSLToSPIRV(VkShaderStageFlagBits shaderStage, const std::vector<char>& srcCode, std::vector<uint32_t>& outSpirvCode);
	void TransferGLSLFileToSPIRVFileAndRead(const std::string& srcGLSLFile, std::vector<uint32_t>& outSpirvCode);
	void ParseSPIRVShaderInputAttribute(const std::vector<uint32_t>& spirvCode, std::vector<ShaderInputAttributeInfo>& dstCacheShaderInputAttributeInfo);
	void ParseSPIRVShaderResourceInfo(const std::vector<uint32_t>& spirvCode, ShaderResourceInfo& dstCacheShaderResource);
	void ParseShaderFiles(RenderPassInfo& renderPassInfo);



	//utils
	Buffer CreateBuffer(VkBufferUsageFlags usage,const char* buf, VkDeviceSize size,VkMemoryPropertyFlags memoryPropties);

	



private:

	bool initFlag = false;
	//struct DeviceRequirement
	//{
	//	VkQueueFlags queueType = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
	//	uint32_t numQueue = 3;
	//	const bool needPresent = true;


	//};



	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugUtilMessager = VK_NULL_HANDLE;
	

	VkPhysicalDeviceProperties physicalDeviceProps;
	VkPhysicalDeviceFeatures2 physicalDeviceFeatures2;
	VkPhysicalDeviceMeshShaderFeaturesEXT physicalDeviceMeshShaderFeaturesEXT;
	VkPhysicalDeviceMaintenance4Features maintenance4Feature;

	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	GLFWwindow* window = nullptr;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkColorSpaceKHR colorSpace;
	//const VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;//所有地方的颜色格式都为B G R A ,在该格式情况下，片段着色器输出会对调R和B分量，即片段着色器输出（1，0，0，1），实际写入到附件中的是（0，0，1，1），对swapchain中的image，存储在其中的pixel的值为（1，0，0，1）则会显示蓝色,着色器中采样获取的格式为（R,G,B,A）,要想正常的在该格式下进行显示，在片段着色其中的颜色输出需要按照正常的R，G，B，A格式，采样得到的颜色也是按照R，G，B，A格式
	const VkFormat swapchainImageFormat = RenderTargets::colorFormat;//默认交换链图像的格式 

	const std::vector<VkFormat> candidatedTextureFormats = {
		VK_FORMAT_R8G8B8A8_UNORM,//当前使用stb读取的时srgb格式的数据，而该格式再着色器中不会进行gama解码，且颜色附件为srgb的，所以输出的时候会自动编码，所以为保证结果正确，在着色器中需要手动解码
		VK_FORMAT_R8G8B8A8_SNORM,
		VK_FORMAT_R8G8B8A8_USCALED,
		VK_FORMAT_R8G8B8A8_SSCALED,
		VK_FORMAT_R8G8B8A8_UINT,
		VK_FORMAT_R8G8B8A8_SINT,
		VK_FORMAT_R32G32B32A32_SFLOAT,
		VK_FORMAT_R32_SFLOAT
	};

	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;






	const std::array<VertexAttributeType, 6> vertexAttributes = {
		VAT_Position_float32,//location 0
		VAT_Normal_float32,//location 1
		VAT_Color_float32,//location 2
		VAT_TextureCoordinates_float32,//location 3
		VAT_AUX1_float32,//location 4
		VAT_AUX2_float32//location 5
	};//



	const uint32_t vertexAttributeInputStride = 3 * vertexAttributes.size() * sizeof(float);

	//先不考虑compute pipeline
	//std::vector<GraphicPipelineInfos> graphcisPipelineInfos;
	//ComputePipelineInfos computePipelineInfos;
	std::vector<ComputePipelineInfos> computePipelinesInfos;

	VkQueue graphicQueue = VK_NULL_HANDLE;//该队列可以graphic，可以transfer，可以present
	

	uint32_t queueFamilyIndex = 0;


	Barrier barrier;

	//command
	VkCommandPool commandPool = VK_NULL_HANDLE;

	CommandList oneSubmitCommandList;

	//VkCommandBuffer renderCommandBuffer = VK_NULL_HANDLE, oneSubmitCommandBuffer = VK_NULL_HANDLE;
	//VkFence renderCommandBufferFence = VK_NULL_HANDLE, oneSubmitCommmandBufferFence = VK_NULL_HANDLE;


	VkQueryPool queryPool = VK_NULL_HANDLE;
	

};