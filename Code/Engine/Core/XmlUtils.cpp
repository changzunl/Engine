#include "XmlUtils.hpp"

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

int ParseXmlAttribute(const XmlElement& element, const char* attributeName, int defaultValue)
{
	return element.IntAttribute(attributeName, defaultValue);
}

char ParseXmlAttribute(const XmlElement& element, const char* attributeName, char defaultValue)
{
	return (char) element.IntAttribute(attributeName, defaultValue);
}

bool ParseXmlAttribute(const XmlElement& element, const char* attributeName, bool defaultValue)
{
	return element.BoolAttribute(attributeName, defaultValue);
}

float ParseXmlAttribute(const XmlElement& element, const char* attributeName, float defaultValue)
{
	return element.FloatAttribute(attributeName, defaultValue);
}

Rgba8 ParseXmlAttribute(const XmlElement& element, const char* attributeName, const Rgba8& defaultValue)
{
	Rgba8 value = defaultValue;
	const char* attrValue = element.Attribute(attributeName, nullptr);
	if (attrValue)
	{
		value.SetFromText(attrValue);
	}
	return value;
}

FloatRange ParseXmlAttribute(const XmlElement& element, const char* attributeName, const FloatRange& defaultValue)
{
	FloatRange value = defaultValue;
	const char* attrValue = element.Attribute(attributeName, nullptr);
	if (attrValue)
	{
		value.SetFromText(attrValue);
	}
	return value;
}

Vec2 ParseXmlAttribute(const XmlElement& element, const char* attributeName, const Vec2& defaultValue)
{
	Vec2 value = defaultValue;
	const char* attrValue = element.Attribute(attributeName, nullptr);
	if (attrValue)
	{
		value.SetFromText(attrValue);
	}
	return value;
}

Vec3 ParseXmlAttribute(const XmlElement& element, const char* attributeName, const Vec3& defaultValue)
{
	Vec3 value = defaultValue;
	const char* attrValue = element.Attribute(attributeName, nullptr);
	if (attrValue)
	{
		value.SetFromText(attrValue);
	}
	return value;
}

EulerAngles ParseXmlAttribute(const XmlElement& element, const char* attributeName, const EulerAngles& defaultValue)
{
	EulerAngles value = defaultValue;
	const char* attrValue = element.Attribute(attributeName, nullptr);
	if (attrValue)
	{
		value.SetFromText(attrValue);
	}
	return value;
}

IntVec2 ParseXmlAttribute(const XmlElement& element, const char* attributeName, const IntVec2& defaultValue)
{
	IntVec2 value = defaultValue;
	const char* attrValue = element.Attribute(attributeName, nullptr);
	if (attrValue)
	{
		value.SetFromText(attrValue);
	}
	return value;
}

std::string ParseXmlAttribute(const XmlElement& element, const char* attributeName, const std::string& defaultValue)
{
	return ParseXmlAttribute(element, attributeName, defaultValue.c_str());
}

std::string ParseXmlAttribute(const XmlElement& element, const char* attributeName, const char* defaultValue)
{
	const char* attrValue = element.Attribute(attributeName, nullptr);
	return attrValue ? attrValue : defaultValue;
}

StringList ParseXmlAttribute(const XmlElement& element, const char* attributeName, const StringList& defaultValues)
{
	const char* attrValue = element.Attribute(attributeName, nullptr);
	return attrValue ? SplitStringOnDelimiter(attrValue, ',') : defaultValues;
}

