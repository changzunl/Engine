#include "Engine/Core/NamedProperties.hpp"

char tolower(char in) {
    if (in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
}

std::string tolower(std::string str)
{
    for (auto& c : str)
        c = tolower(c);

    return str;
}
