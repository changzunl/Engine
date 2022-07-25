#pragma once

#include "StringUtils.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"

struct Rgba8;
struct FloatRange;
struct Vec2;
struct Vec3;
struct EulerAngles;
struct IntVec2;

typedef tinyxml2::XMLDocument XmlDocument;
typedef tinyxml2::XMLElement XmlElement;
typedef tinyxml2::XMLAttribute XmlAttribute;

int                      ParseXmlAttribute(const XmlElement& element, const char* attributeName, int                    defaultValue);
char                     ParseXmlAttribute(const XmlElement& element, const char* attributeName, char                   defaultValue);
bool                     ParseXmlAttribute(const XmlElement& element, const char* attributeName, bool                   defaultValue);
float                    ParseXmlAttribute(const XmlElement& element, const char* attributeName, float                  defaultValue);
Rgba8                    ParseXmlAttribute(const XmlElement& element, const char* attributeName, const Rgba8&           defaultValue);
FloatRange               ParseXmlAttribute(const XmlElement& element, const char* attributeName, const FloatRange&      defaultValue);
Vec2                     ParseXmlAttribute(const XmlElement& element, const char* attributeName, const Vec2&            defaultValue);
Vec3                     ParseXmlAttribute(const XmlElement& element, const char* attributeName, const Vec3&            defaultValue);
EulerAngles              ParseXmlAttribute(const XmlElement& element, const char* attributeName, const EulerAngles&     defaultValue);
IntVec2                  ParseXmlAttribute(const XmlElement& element, const char* attributeName, const IntVec2&         defaultValue);
std::string              ParseXmlAttribute(const XmlElement& element, const char* attributeName, const std::string&     defaultValue);
std::string              ParseXmlAttribute(const XmlElement& element, const char* attributeName, const char*            defaultValue);
StringList               ParseXmlAttribute(const XmlElement& element, const char* attributeName, const StringList&      defaultValues);

