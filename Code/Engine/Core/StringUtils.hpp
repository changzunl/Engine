#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

typedef std::vector<std::string> StringList;

//-----------------------------------------------------------------------------------------------
const std::string       Stringf( char const* format, ... );
const std::string       Stringf( int maxLength, char const* format, ... );
StringList              SplitStringOnDelimiter(const std::string& originalString, char delimiterToSplitOn);
std::string             TrimString(const std::string& inputString);



