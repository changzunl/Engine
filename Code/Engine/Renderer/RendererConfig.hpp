#pragma once

class Window;

struct RendererConfig
{
public:
	RendererConfig();
	~RendererConfig();

public:
	Window* m_window         = nullptr;
	int     m_initialVBOSize = 8192 * 6 * 4;
};

