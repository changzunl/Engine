#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <string>
#include <vector>
#include "ThirdParty/stb/stb_image.h"

class Image
{
public:
	static const Image WHITE_PIXEL;

public:
    Image();
    Image(char const* imageFilePath);
    Image(const IntVec2& size, const Rgba8& color);

    const std::string&    GetImageFilePath() const                                            { return m_imageFilePath; }
    const IntVec2&        GetDimensions() const 										      { return m_dimensions; }
    const void*           GetRawData() const 				                                  { return m_rgbaTexels.data(); }
    size_t                GetRawDataSize() const 				                              { return m_rgbaTexels.size() * sizeof(Rgba8); }

    const Rgba8&          GetTexelColor(const IntVec2& texelCoords) const 				      { return m_rgbaTexels[texelCoords.x + texelCoords.y * m_dimensions.x]; }
    void			      SetTexelColor(const IntVec2& texelCoords, const Rgba8& newColor)    { m_rgbaTexels[texelCoords.x + texelCoords.y * m_dimensions.x] = newColor; }

private:
	std::string			  m_imageFilePath;
	IntVec2			      m_dimensions;
	std::vector<Rgba8>	  m_rgbaTexels;
};

