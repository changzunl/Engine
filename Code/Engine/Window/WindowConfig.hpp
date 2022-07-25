#pragma once

#include <string>

class InputSystem;

struct WindowConfig
{
public:
	WindowConfig();
	~WindowConfig();

public:
	std::string     m_windowTitle     = "";
	float           m_clientAspect    = 2.0f;
	InputSystem*    m_inputSystem     = nullptr;
};

