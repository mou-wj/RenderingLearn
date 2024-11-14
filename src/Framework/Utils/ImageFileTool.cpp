#include "ImageFileTool.h"
#include "stb_image.h"
#include "stb_image_write.h"

void LoadCharSRGBJpeg(const std::string& imageFile, const std::vector<PixelComponent>& wantPixelComponents, std::vector<char>& outImageData, uint32_t& outWidth, uint32_t& outHeight)
{
	int x = 0, y = 0, numChannel = 0;
	auto imageData = stbi_load(imageFile.c_str(), &x, &y, &numChannel,4);
	outWidth = x;
	outHeight = y;
	uint32_t wantNumComps = wantPixelComponents.size();
	outImageData.resize(outWidth * outHeight * wantNumComps);

	for (uint32_t row = 0; row < y; row++)
	{
		for (uint32_t col = 0; col < x; col++)
		{
			for (uint32_t i = 0; i < wantPixelComponents.size(); i++)
			{
				outImageData[(row * outWidth + col) * wantNumComps + i] = imageData[(row * outWidth + col) * 4 + wantPixelComponents[i]];
			}
		}
	}

	stbi_image_free(imageData);
}
