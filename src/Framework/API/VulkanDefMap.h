#pragma once
#include <vulkan/vulkan.h>
#include <map>
#include <string>
#include <vector>

// 每个 VkFormat 的详细信息
struct VkFormatInfo {
    std::string name;                 // 格式名称
    uint32_t componentCount;          // 分量数
    std::vector<float> componentSizes; // 每个分量的字节数
    uint32_t totalBytesPerPixel;      // 每像素总字节数
};

extern std::map<VkFormat, VkFormatInfo> VkFormatToInfo;