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

void LoadUint8SRGBJpeg(const std::string& imageFile, const std::vector<PixelComponent>& wantPixelComponents, std::vector<uint8_t>& outImageData, uint32_t& outWidth, uint32_t& outHeight);;