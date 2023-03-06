#include "Image.hpp"

#include "ErrorWarningAssert.hpp"
#include "StringUtils.hpp"
#define STB_IMAGE_IMPLEMENTATION // Exactly one .CPP (this Image.cpp) should #define this before #including stb_image.h
#include "ThirdParty/stb/stb_image.h"

const Image Image::WHITE_PIXEL = Image(IntVec2(2, 2), Rgba8::WHITE);

Image::Image(char const* imageFilePath)
{
	IntVec2 dimensions = IntVec2::ZERO;		// This will be filled in for us to indicate image width & height
	int bytesPerTexel = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load(1);
	unsigned char* texelData = stbi_load(imageFilePath, &dimensions.x, &dimensions.y, &bytesPerTexel, numComponentsRequested);

	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("Failed to load image \"%s\" - texel data is null", imageFilePath));
	GUARANTEE_OR_DIE(bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf("Failed to load image \"%s\" - unsupported BPP=%i (must be 3 or 4)", imageFilePath, bytesPerTexel));
	GUARANTEE_OR_DIE(dimensions.x > 0 && dimensions.y > 0, Stringf("Failed to load image \"%s\" - illegal texture dimensions (%i x %i)", imageFilePath, dimensions.x, dimensions.y));

	int imagePixels = dimensions.x * dimensions.y;
	m_imageFilePath = imageFilePath;
	m_dimensions = dimensions;
	m_rgbaTexels.reserve(imagePixels);

	if (bytesPerTexel == 3)
	{
		// 24-bit RGB
		for (int idx = 0; idx < imagePixels; idx++)
		{
			unsigned char r = texelData[idx * 3 + 0];
			unsigned char g = texelData[idx * 3 + 1];
			unsigned char b = texelData[idx * 3 + 2];
			unsigned char a = 255;
			m_rgbaTexels.emplace_back(r, g, b, a);
		}
	}
	else
	{
		// 32-bit RGBA
		for (int idx = 0; idx < imagePixels; idx++)
		{
			unsigned char r = texelData[idx * 4 + 0];
			unsigned char g = texelData[idx * 4 + 1];
			unsigned char b = texelData[idx * 4 + 2];
			unsigned char a = texelData[idx * 4 + 3];
			m_rgbaTexels.emplace_back(r, g, b, a);
		}
	}
	// Free the raw image texel data
	stbi_image_free(texelData);
}

Image::Image(const IntVec2& size, const Rgba8& color)
	: m_imageFilePath("")
	, m_dimensions(size)
{
	int dataSize = size.x * size.y;
	m_rgbaTexels.resize((size_t)dataSize);
	for (int i = 0; i < dataSize; i++)
		m_rgbaTexels[i] = color;
}

Image::Image()
{
}

