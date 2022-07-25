#include "NamedStrings.hpp"

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

#include <stdlib.h>
#include <string.h>

void NamedStrings::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	const tinyxml2::XMLAttribute* attr = element.FirstAttribute();
	while (attr)
	{
		SetValue(attr->Name(), attr->Value());
		attr = attr->Next();
	}
}

void NamedStrings::SetValue(const std::string& keyName, const std::string& newValue)
{
	SetOrCreateEntry(keyName, newValue);
}

std::string NamedStrings::GetValue(const std::string& keyName, const std::string& defaultValue) const
{
	const StringPair* pairPointer = GetEntry(keyName);
	if (!pairPointer) return defaultValue;
	return pairPointer->second;
}

std::string NamedStrings::GetValue(const std::string& keyName, const char* defaultValue) const
{
	const StringPair* pairPointer = GetEntry(keyName);
	if (!pairPointer) return defaultValue;
	return pairPointer->second;
}

bool NamedStrings::GetValue(const std::string& keyName, bool defaultValue) const
{
	const StringPair* pairPointer = GetEntry(keyName);
	if (!pairPointer) return defaultValue;
	return _stricmp(pairPointer->second.c_str(), "true") == 0;
}

int NamedStrings::GetValue(const std::string& keyName, int defaultValue) const
{
	const StringPair* pairPointer = GetEntry(keyName);
	if (!pairPointer) return defaultValue;
	return atoi(pairPointer->second.c_str());
}

float NamedStrings::GetValue(const std::string& keyName, float defaultValue) const
{
	const StringPair* pairPointer = GetEntry(keyName);
	if (!pairPointer) return defaultValue;
	return (float) atof(pairPointer->second.c_str());
}

Rgba8 NamedStrings::GetValue(const std::string& keyName, const Rgba8& defaultValue) const
{
	const StringPair* pairPointer = GetEntry(keyName);
	if (!pairPointer) return defaultValue;
	Rgba8 value;
	value.SetFromText(pairPointer->second.c_str());
	return value;
}

IntVec2 NamedStrings::GetValue(const std::string& keyName, const IntVec2& defaultValue) const
{
	const StringPair* pairPointer = GetEntry(keyName);
	if (!pairPointer) return defaultValue;
	IntVec2 value;
	value.SetFromText(pairPointer->second.c_str());
	return value;
}

Vec2 NamedStrings::GetValue(const std::string& keyName, const Vec2& defaultValue) const
{
	const StringPair* pairPointer = GetEntry(keyName);
	if (!pairPointer) return defaultValue;
	Vec2 value;
	value.SetFromText(pairPointer->second.c_str());
	return value;
}

Vec3 NamedStrings::GetValue(const std::string& keyName, const Vec3& defaultValue) const
{
	const StringPair* pairPointer = GetEntry(keyName);
	if (!pairPointer) return defaultValue;
	Vec3 value;
	value.SetFromText(pairPointer->second.c_str());
	return value;
}

void NamedStrings::SetOrCreateEntry(const std::string& key, const std::string& value)
{
	for (StringPair& entry : m_kvPairList)
	{
		if (entry.first == key)
		{
			entry.second = value;
			return;
		}
	}
	m_kvPairList.emplace_back(key, value);
}

const StringPair* NamedStrings::GetEntry(const std::string& key) const
{
	for (const StringPair& entry : m_kvPairList)
	{
		if (entry.first == key) return &entry;
	}
	return nullptr;
}
