#include "InputSystemConfig.hpp"
#include "XboxController.hpp"
#include "KeyButtonState.hpp"
#include "KeyCodes.hpp"

constexpr int NUM_KEYCODES = 256;
constexpr int NUM_XBOX_CONTROLLERS = 4;

struct AABB2;

class InputSystem
{
public:
    InputSystem(const InputSystemConfig& theConfig);
    ~InputSystem();

    void Startup();
    void BeginFrame();
    void EndFrame();
    void Shutdown();

    // key inputs
    bool IsKeyDown(unsigned char keyCode) const;
    bool WasKeyJustPressed(unsigned char keyCode) const;
    bool WasKeyJustReleased(unsigned char keyCode) const;
    bool WasQuitJustRequested() const;

    // input handlers
    bool HandleKeyPressed(unsigned char keyCode);
    bool HandleKeyReleased(unsigned char keyCode);
    bool HandleCharInput(int charCode); 
    bool HandleQuitRequested();

    // controllers
    const XboxController& GetXboxController(int controllerIndex) const;
    const XboxController& GetFirstAvaliableXboxController() const;

	// mouse
	Vec2 GetMouseClientPosition() const;
	Vec2 GetMousePositionInWindow(const AABB2& viewport) const;
	Vec2 GetMouseClientDelta() const;
    int GetMouseWheel() const;

    void SetMouseMode(bool hidden, bool clipped, bool relative);
    void ReportMouseMode(bool hidden, bool clipped, bool relative);
    void ReportMouseDelta(const Vec2& delta);
    void ReportMousePosition(const Vec2& position);
    void ReportMouseWheel(int value);

private:
    void UpdateXboxControllerState();

protected:
    InputSystemConfig m_theConfig;

    XboxController    m_xobxControllers[NUM_XBOX_CONTROLLERS] = {};
    KeyButtonState    m_quitButton;
    KeyButtonState    m_keyStates[NUM_KEYCODES]               = {};

public:
    bool              m_isMouseHidden                         = false; // or visible
    bool              m_isMouseClipped                        = false; // or free
    bool              m_isMouseRelative                       = false; // or absolute
    bool              m_isStateModified                       = false;

private:
    Vec2              m_mousePosition;
    Vec2              m_mouseDelta;
    int               m_mouseWheel                            = 0;
};

