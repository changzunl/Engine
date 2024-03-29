#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

StringList SplitStringOnDelimiter(const std::string& inputString, char delimiter)
{
	StringList list;
	size_t start = 0;
	size_t cursor = 0;
	while (true)
	{
		unsigned char ch = inputString[cursor];
		if (ch == 0 || ch == delimiter)
		{
			list.push_back(inputString.substr(start, cursor - start));
			start = cursor + 1;
		}
		if (ch == 0) break;

		++cursor;
	}
	return list;
}

std::string TrimString(const std::string& inputString)
{
	size_t firstNonSpace = inputString.find_first_not_of(' ');
	if (firstNonSpace == std::string::npos) return "";

	size_t lastNonSpace = inputString.find_last_not_of(' ');
	return inputString.substr(firstNonSpace,  lastNonSpace + 1 - firstNonSpace);
}

StringList ParseStringOnSpace(const std::string& originalString)
{
	StringList list = SplitStringOnDelimiter(originalString, ' ');
	if (list.empty())
		return list;

	StringList parsed;

	for (auto& str : list)
	{
		if (str.empty())
			continue;

		if (parsed.empty())
        {
            parsed.push_back(str);
		}
		else if (str.back() == '"')
		{
			while (parsed.size() > 1 && std::find(parsed.back().begin(), parsed.back().end(), '"') == parsed.back().end()) // while cannot find '"' in prev line
			{
				*(parsed.end() - 2) += " " + parsed.back();
				parsed.pop_back();
			}
			parsed.back() += " " + str;
		}
		else
		{
			parsed.push_back(str);
		}
	}

	return parsed;
}

StringList ParseArgumentOnEquals(const std::string& originalString)
{
	StringList result = { "", "" };

	for (size_t i = 0; i < originalString.size(); i++)
	{
		if (originalString[i] == '=')
		{
			result[0] = originalString.substr(0, i);
			result[1] = originalString.substr(i + 1, originalString.size() - i - 1);

			if (result[1].size() >= 2 && result[1].front() == '\"' && result[1].back() == '\"')
			{
                result[1].pop_back();
                result[1].erase(result[1].begin());
			}

			return result;
		}
	}

	return result;
}

