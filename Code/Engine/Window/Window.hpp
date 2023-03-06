#include <map>

#include "WindowConfig.hpp"

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"

#include <vector>

class Window;
class InputSystem;

struct WindowInfo
{
	void* hWnd;
	Window* window;
};

class Window
{

public:
	// constructors
	Window(const WindowConfig& theConfig);
	~Window();

	// lifecycle
    void Startup();
    void Shutdown();

	void BeginFrame();
	void EndFrame();
    
	static void FrameBegin();
	static void FrameEnd();

	// utilities
	void*        GetHwnd() const                                   { return m_osWindowHandle; }
	InputSystem* GetInputSystem() const;
    IntVec2      GetClientDimensions() const;
	Vec2         GetNormalizedCursorPos() const;
	bool         IsWindowActive() const;
	bool         PullQuitRequested();
	bool         IsQuitRequested() const;
    void         SetFocus() const;
    void         SetMouseMode(bool hidden, bool clipped, bool relative);

	// setters
	void         SetWindowActive(bool active);
    void         SetNormalizedCursorPos(const Vec2& position) const;
    void         HandleQuit();

private:
	void CreateOSWindow();
    void UpdateCursor();

    static void RunMessagePump(void* hWnd);

public:
	static Window*    s_mainWindow;
	static Window*    s_activeWindow;
    static std::vector<WindowInfo> s_windows;
    WindowConfig      m_theConfig;

private:
	void*             m_osWindowHandle  = nullptr;
	IntVec2           m_clientDimension;
	bool              m_isActive        = false;
	bool              m_isQuitting      = false;

    bool              m_isMouseHidden = false; // or visible
    bool              m_isMouseClipped = false; // or free
    bool              m_isMouseRelative = false; // or absolute

	static bool       s_frameBegun;
};

