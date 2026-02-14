#include "VulkanRHIUtils.h"
#include "Log.h"
namespace RHIVulkan {

    VkFormat TransformFormatFrom(ERHIFormat format) {
        switch (format) {
        case ERHIFormat::R8_UNorm: return VK_FORMAT_R8_UNORM;
        case ERHIFormat::R8G8B8A8_UNorm: return VK_FORMAT_R8G8B8A8_UNORM;
        case ERHIFormat::R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
        case ERHIFormat::B8G8R8A8_UNorm: return VK_FORMAT_B8G8R8A8_UNORM;
        case ERHIFormat::B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
        case ERHIFormat::R16G16_Float: return VK_FORMAT_R16G16_SFLOAT;
        case ERHIFormat::R16G16B16A16_Float: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case ERHIFormat::R32_Float: return VK_FORMAT_R32_SFLOAT;
        case ERHIFormat::R32G32_Float: return VK_FORMAT_R32G32_SFLOAT;
        case ERHIFormat::R32G32B32A32_Float: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case ERHIFormat::D24_UNorm_S8_UInt: return VK_FORMAT_D24_UNORM_S8_UINT;
        case ERHIFormat::D32_Float: return VK_FORMAT_D32_SFLOAT;
        default:
            return VK_FORMAT_UNDEFINED; // 未知格式
        }
    }


    VkShaderStageFlagBits TransformShaderStageFrom(ERHIShaderFrequency shaderType) {
        switch (shaderType) {
            case ERHIShaderFrequency::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
            case ERHIShaderFrequency::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
            case ERHIShaderFrequency::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
            case ERHIShaderFrequency::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
            case ERHIShaderFrequency::TessControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            case ERHIShaderFrequency::TessEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            case ERHIShaderFrequency::Mesh: return VK_SHADER_STAGE_MESH_BIT_EXT; // Vulkan扩展
            case ERHIShaderFrequency::Task: return VK_SHADER_STAGE_TASK_BIT_EXT; // Vulkan扩展
            case ERHIShaderFrequency::RayGen: return VK_SHADER_STAGE_RAYGEN_BIT_KHR; // Ray Tracing
            case ERHIShaderFrequency::ClosestHit: return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR; // Ray Tracing
            case ERHIShaderFrequency::Miss: return VK_SHADER_STAGE_MISS_BIT_KHR; // Ray Tracing
            case ERHIShaderFrequency::AnyHit: return VK_SHADER_STAGE_ANY_HIT_BIT_KHR; // Ray Tracing
            case ERHIShaderFrequency::Intersection: return VK_SHADER_STAGE_INTERSECTION_BIT_KHR; // Ray Tracing
            case ERHIShaderFrequency::Callable: return VK_SHADER_STAGE_CALLABLE_BIT_KHR; // Ray Tracing
            default:
                return static_cast<VkShaderStageFlagBits>(0); // 未知阶段
        }
    }



    VkImageUsageFlags TransformTextureUsageFlagsFrom(ERHITextureCreateFlags Flags)
    {
        VkImageUsageFlags Usage = 0;

        if (EnumHasAnyFlags(Flags, ERHITextureCreateFlags::ShaderResource))
        {
            // sampled image / combined image sampler
            Usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        if (EnumHasAnyFlags(Flags, ERHITextureCreateFlags::RenderTarget))
        {
            // color attachment
            Usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }

        if (EnumHasAnyFlags(Flags, ERHITextureCreateFlags::UAV))
        {
            // storage image
            Usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        }

        if (EnumHasAnyFlags(Flags, ERHITextureCreateFlags::DepthStencil))
        {
            Usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        if (EnumHasAnyFlags(Flags, ERHITextureCreateFlags::CopySrc))
        {
            Usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        if (EnumHasAnyFlags(Flags, ERHITextureCreateFlags::CopyDest))
        {
            Usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        if (EnumHasAnyFlags(Flags, ERHITextureCreateFlags::Presentable))
        {
            // swapchain images *must* have this
            Usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }

        return Usage;
    }
    VkSampleCountFlagBits TransformSampleCountFrom(uint32_t sampleCount)
    {
        switch (sampleCount)
        {
        case 1:  return VK_SAMPLE_COUNT_1_BIT;
        case 2:  return VK_SAMPLE_COUNT_2_BIT;
        case 4:  return VK_SAMPLE_COUNT_4_BIT;
        case 8:  return VK_SAMPLE_COUNT_8_BIT;
        case 16: return VK_SAMPLE_COUNT_16_BIT;
        case 32: return VK_SAMPLE_COUNT_32_BIT;
        case 64: return VK_SAMPLE_COUNT_64_BIT;
        default:
            LOG_ERROR("Invalid sample count: {}", sampleCount);
        }
        return VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
    }

    VkImageType TransformImageTypeFrom(ERHITextureType type)
    {
        switch (type)
        {
        case ERHITextureType::Texture1D:
            return VK_IMAGE_TYPE_1D;

        case ERHITextureType::Texture2D:
        case ERHITextureType::Texture2DArray:
        case ERHITextureType::TextureCube:
        case ERHITextureType::TextureCubeArray:
            // Cube / Array 本质都是 2D image
            return VK_IMAGE_TYPE_2D;

        case ERHITextureType::Texture3D:
            return VK_IMAGE_TYPE_3D;

        case ERHITextureType::Unknown:
        default:
            break;
        }
        LOG_ERROR("Invalid texture type: {}", static_cast<int>(type));
        return VK_IMAGE_TYPE_2D;
    }

    VkImageViewType TransformViewTypeFrom(ERHITextureType type) {
        switch (type) {
        case ERHITextureType::Texture1D:
            return VK_IMAGE_VIEW_TYPE_1D;
        case ERHITextureType::Texture2D:
            return VK_IMAGE_VIEW_TYPE_2D;
        case ERHITextureType::Texture3D:
            return VK_IMAGE_VIEW_TYPE_3D;
        case ERHITextureType::TextureCube:
            return VK_IMAGE_VIEW_TYPE_CUBE;
        case ERHITextureType::Texture2DArray:
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case ERHITextureType::TextureCubeArray:
            return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        default:
            break;
        }
        LOG_ERROR("Invalid texture type: {}", static_cast<int>(type));
        return VK_IMAGE_VIEW_TYPE_2D; // 默认类型
    }

    VkImageAspectFlags GetImageAspectFlags(VkFormat format)
    {
        switch (format)
        {
            // ---- Depth only ----
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;

            // ---- Stencil only ----
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;

            // ---- Depth + Stencil ----
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

            // ---- Everything else treated as color ----
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }
}