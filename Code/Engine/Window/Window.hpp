#include "WindowConfig.hpp"

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"

class Window
{
public:
	// constructors
	Window(const WindowConfig& theConfig);
	~Window();

	// lifecycle
	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	// utilities
	void*        GetHwnd() const                                   { return m_osWindowHandle; }
	InputSystem* GetInputSystem() const;
    IntVec2      GetClientDimensions() const;
	Vec2         GetNormalizedCursorPos() const;
	bool         IsWindowActive() const;

	// setters
	void         SetWindowActive(bool active);
	void         SetNormalizedCursorPos(const Vec2& position) const;

private:
	void CreateOSWindow();
	void RunMessagePump();
	void UpdateCursor();

public:
	static Window*    s_mainWindow;
	WindowConfig      m_theConfig;

private:
	void*             m_osWindowHandle  = nullptr;
	IntVec2           m_clientDimension;
	bool              m_isActive        = false;
};

