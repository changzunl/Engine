#pragma once

#include "XmlUtils.hpp"
#include <string>
#include <vector>
#include <utility>

struct Rgba8;
struct IntVec2;
struct Vec2;
struct Vec3;

typedef std::pair<std::string, std::string> StringPair;
typedef std::vector<StringPair> StringPairList;

class NamedStrings
{
public:
	NamedStrings() {};
	~NamedStrings() {};

	void			PopulateFromXmlElementAttributes(XmlElement const& element);
	void			SetValue(const std::string& keyName, const std::string&    newValue);
	std::string		GetValue(const std::string& keyName, const std::string&    defaultValue) const;
	std::string		GetValue(const std::string& keyName, const char*           defaultValue) const;
	bool			GetValue(const std::string& keyName, bool                  defaultValue) const;
	int			    GetValue(const std::string& keyName, int                   defaultValue) const;
	float			GetValue(const std::string& keyName, float                 defaultValue) const;
	Rgba8			GetValue(const std::string& keyName, const Rgba8&          defaultValue) const;
	IntVec2         GetValue(const std::string& keyName, const IntVec2&        defaultValue) const;
	Vec2			GetValue(const std::string& keyName, const Vec2&           defaultValue) const;
	Vec3			GetValue(const std::string& keyName, const Vec3&           defaultValue) const;

private:
	void                 SetOrCreateEntry(const std::string& key, const std::string& value);
	const StringPair*    GetEntry(const std::string& key) const;

private:
	StringPairList	m_kvPairList;
};

