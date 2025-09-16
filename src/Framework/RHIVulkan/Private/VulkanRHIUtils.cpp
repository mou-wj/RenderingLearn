#include "VulkanRHIUtils.h"

namespace RHIVulkan {

    VkFormat TransforFormatFrom(ERHIFormat format) {
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


    VkShaderStageFlagBits TransforShaderStageFrom(ERHIShaderType shaderType) {
        switch (shaderType) {
            case ERHIShaderType::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
            case ERHIShaderType::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
            case ERHIShaderType::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
            case ERHIShaderType::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
            case ERHIShaderType::TessControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            case ERHIShaderType::TessEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            case ERHIShaderType::Mesh: return VK_SHADER_STAGE_MESH_BIT_EXT; // Vulkan扩展
            case ERHIShaderType::Task: return VK_SHADER_STAGE_TASK_BIT_EXT; // Vulkan扩展
            case ERHIShaderType::RayGen: return VK_SHADER_STAGE_RAYGEN_BIT_KHR; // Ray Tracing
            case ERHIShaderType::ClosestHit: return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR; // Ray Tracing
            case ERHIShaderType::Miss: return VK_SHADER_STAGE_MISS_BIT_KHR; // Ray Tracing
            case ERHIShaderType::AnyHit: return VK_SHADER_STAGE_ANY_HIT_BIT_KHR; // Ray Tracing
            case ERHIShaderType::Intersection: return VK_SHADER_STAGE_INTERSECTION_BIT_KHR; // Ray Tracing
            case ERHIShaderType::Callable: return VK_SHADER_STAGE_CALLABLE_BIT_KHR; // Ray Tracing
            default:
                return static_cast<VkShaderStageFlagBits>(0); // 未知阶段
        }
    }   

}