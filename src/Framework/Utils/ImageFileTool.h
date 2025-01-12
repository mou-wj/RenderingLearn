#pragma once
#include <string>
#include <vector>
enum PixelComponent:uint32_t
{
	R = 0,
	G = 1,
	B = 2,
	A = 3
};

void LoadCharUnsignedCharJpeg(const std::string& imageFile, const std::vector<PixelComponent>& wantPixelComponents, std::vector<char>& outImageData, uint32_t& outWidth, uint32_t& outHeight,bool flip = false);
void WriteJpeg(const std::string& outImagePath, const char* imageData, uint32_t width, uint32_t height, uint32_t numComponent = 4);
void WriteJpeg(const std::string& outImagePath, const float* imageData, uint32_t width, uint32_t height, uint32_t numComponent = 4);
