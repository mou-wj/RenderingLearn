#pragma once
#include <vulkan/vulkan.h>
#include <map>
#include <string>
#include <vector>

// ÿ�� VkFormat ����ϸ��Ϣ
struct VkFormatInfo {
    std::string name;                 // ��ʽ����
    uint32_t componentCount;          // ������
    std::vector<float> componentSizes; // ÿ���������ֽ���
    uint32_t totalBytesPerPixel;      // ÿ�������ֽ���
};

extern std::map<VkFormat, VkFormatInfo> VkFormatToInfo;