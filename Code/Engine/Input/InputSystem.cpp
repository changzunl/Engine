#include "Engine/Input/InputSystem.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"

#include "Engine/Math/AABB2.hpp"

#include "Engine/Window/Window.hpp"

#include <Windows.h>

const unsigned char KEYCODE_Q                 = 'Q';
const unsigned char KEYCODE_W                 = 'W';
const unsigned char KEYCODE_E                 = 'E';
const unsigned char KEYCODE_R                 = 'R';
const unsigned char KEYCODE_T                 = 'T';
const unsigned char KEYCODE_Y                 = 'Y';
const unsigned char KEYCODE_U                 = 'U';
const unsigned char KEYCODE_I                 = 'I';
const unsigned char KEYCODE_O                 = 'O';
const unsigned char KEYCODE_P                 = 'P';
const unsigned char KEYCODE_A                 = 'A';
const unsigned char KEYCODE_S                 = 'S';
const unsigned char KEYCODE_D                 = 'D';
const unsigned char KEYCODE_F                 = 'F';
const unsigned char KEYCODE_G                 = 'G';
const unsigned char KEYCODE_H                 = 'H';
const unsigned char KEYCODE_J                 = 'J';
const unsigned char KEYCODE_K                 = 'K';
const unsigned char KEYCODE_L                 = 'L';
const unsigned char KEYCODE_Z                 = 'Z';
const unsigned char KEYCODE_X                 = 'X';
const unsigned char KEYCODE_C                 = 'C';
const unsigned char KEYCODE_V                 = 'V';
const unsigned char KEYCODE_B                 = 'B';
const unsigned char KEYCODE_N                 = 'N';
const unsigned char KEYCODE_M                 = 'M';

const unsigned char KEYCODE_1                 = '1';
const unsigned char KEYCODE_2                 = '2';
const unsigned char KEYCODE_3                 = '3';
const unsigned char KEYCODE_4                 = '4';
const unsigned char KEYCODE_5                 = '5';
const unsigned char KEYCODE_6                 = '6';
const unsigned char KEYCODE_7                 = '7';
const unsigned char KEYCODE_8                 = '8';
const unsigned char KEYCODE_9                 = '9';
const unsigned char KEYCODE_0                 = '0';

const unsigned char KEYCODE_TAB               = '\t';
const unsigned char KEYCODE_COMMA             = VK_OEM_COMMA;
const unsigned char KEYCODE_PERIOD            = VK_OEM_PERIOD;

const unsigned char KEYCODE_SPACE             = VK_SPACE;
const unsigned char KEYCODE_ENTER             = VK_RETURN;
const unsigned char KEYCODE_ESC               = VK_ESCAPE;
const unsigned char KEYCODE_BACKSPACE         = VK_BACK;
const unsigned char KEYCODE_LSHIFT            = VK_SHIFT;
const unsigned char KEYCODE_LCTRL             = VK_CONTROL;
const unsigned char KEYCODE_LALT              = VK_LMENU;
const unsigned char KEYCODE_RSHIFT            = VK_SHIFT;
const unsigned char KEYCODE_RCTRL             = VK_CONTROL;
const unsigned char KEYCODE_RALT              = VK_RMENU;

const unsigned char KEYCODE_INSERT            = VK_INSERT;
const unsigned char KEYCODE_DELETE            = VK_DELETE;
const unsigned char KEYCODE_HOME              = VK_HOME;
const unsigned char KEYCODE_END               = VK_END;
const unsigned char KEYCODE_PAGEUP            = VK_PRIOR;
const unsigned char KEYCODE_PAGEDOWN          = VK_NEXT;

const unsigned char KEYCODE_F1                = VK_F1;
const unsigned char KEYCODE_F2                = VK_F2;
const unsigned char KEYCODE_F3                = VK_F3;
const unsigned char KEYCODE_F4                = VK_F4;
const unsigned char KEYCODE_F5                = VK_F5;
const unsigned char KEYCODE_F6                = VK_F6;
const unsigned char KEYCODE_F7                = VK_F7;
const unsigned char KEYCODE_F8                = VK_F8;
const unsigned char KEYCODE_F9                = VK_F9;
const unsigned char KEYCODE_F10               = VK_F10;
const unsigned char KEYCODE_F11               = VK_F11;
const unsigned char KEYCODE_F12               = VK_F12;

const unsigned char KEYCODE_UP                = VK_UP;
const unsigned char KEYCODE_DOWN              = VK_DOWN;
const unsigned char KEYCODE_LEFT              = VK_LEFT;
const unsigned char KEYCODE_RIGHT             = VK_RIGHT;
const unsigned char KEYCODE_LEFT_MOUSE        = VK_LBUTTON;
const unsigned char KEYCODE_RIGHT_MOUSE       = VK_RBUTTON;
const unsigned char KEYCODE_MIDDLE_MOUSE      = VK_MBUTTON;

const unsigned char KEYCODE_TILDE             = VK_OEM_3;

InputSystem::InputSystem(const InputSystemConfig& theConfig)
    : m_theConfig(theConfig)
{
    for (int index = 0; index < NUM_XBOX_CONTROLLERS; index++)
    {
        XboxController& xcontroller = m_xobxControllers[index];
        xcontroller.m_controllerID = index;
    }
}

InputSystem::~InputSystem()
{
}

void InputSystem::Startup()
{
}

void InputSystem::BeginFrame()
{
    UpdateXboxControllerState();
}

void InputSystem::EndFrame()
{
    m_quitButton.m_wasPressedLastFrame = m_quitButton.m_isPressed;
    m_quitButton.m_isPressed = false;
    for (int keyCode = 0; keyCode < NUM_KEYCODES; keyCode++)
    {
        m_keyStates[keyCode].m_wasPressedLastFrame = m_keyStates[keyCode].m_isPressed;
    }

	m_mouseWheel = 0;

    for (int index = 0; index < NUM_XBOX_CONTROLLERS; index++)
    {
        XboxController& xcontroller = m_xobxControllers[index];
        if (xcontroller.IsConnected())
        {
            for (int buttonIndex = 0; buttonIndex < NUM_XBOX_BUTTONS; buttonIndex++)
            {
                xcontroller.m_buttons[buttonIndex].m_wasPressedLastFrame = xcontroller.m_buttons[buttonIndex].m_isPressed;
            }
        }
    }
}

void InputSystem::Shutdown()
{
}

bool InputSystem::IsKeyDown(unsigned char keyCode) const
{
    return m_keyStates[keyCode].m_isPressed;
}

bool InputSystem::WasKeyJustPressed(unsigned char keyCode) const
{
    return m_keyStates[keyCode].m_isPressed && !m_keyStates[keyCode].m_wasPressedLastFrame;
}

bool InputSystem::WasKeyJustReleased(unsigned char keyCode) const
{
    return !m_keyStates[keyCode].m_isPressed && m_keyStates[keyCode].m_wasPressedLastFrame;
}

bool InputSystem::WasQuitJustRequested() const
{
    return m_quitButton.m_isPressed && !m_quitButton.m_wasPressedLastFrame;
}

bool InputSystem::HandleKeyPressed(unsigned char keyCode)
{
    std::string keyString = " ";
    keyString[0] = keyCode;
    EventArgs args;
    args.SetValue("key", keyString);
    if (!g_theEventSystem->FireEvent("Input:KeyPressed", args))
    {
        m_keyStates[keyCode].m_isPressed = true;
    }
    return true;
}

bool InputSystem::HandleKeyReleased(unsigned char keyCode)
{
    std::string keyString = " ";
    keyString[0] = keyCode;
    EventArgs args;
    args.SetValue("key", keyString);
    g_theEventSystem->FireEvent("Input:KeyReleased", args);

    m_keyStates[keyCode].m_isPressed = false;
    return true;
}

bool InputSystem::HandleCharInput(int charCode)
{
    std::string charString = " ";
    charString[0] = (char)charCode;
    EventArgs args;
    args.SetValue("char", charString);
    g_theEventSystem->FireEvent("Input:CharInput", args);
    return true;
}

bool InputSystem::HandleQuitRequested()
{
    m_quitButton.m_isPressed = true;
    return true;
}

const XboxController& InputSystem::GetXboxController(int controllerIndex) const
{
    return m_xobxControllers[controllerIndex];
}

const XboxController& InputSystem::GetFirstAvaliableXboxController() const
{
    for (int index = 0; index < NUM_XBOX_CONTROLLERS; index++)
    {
        if (m_xobxControllers[index].IsConnected()) return m_xobxControllers[index];
    }
    return m_xobxControllers[0];
}

Vec2 InputSystem::GetMouseClientPosition() const
{
    return m_mousePosition;
}

Vec2 InputSystem::GetMousePositionInWindow(const AABB2& viewport) const
{
    Vec2 position = GetMouseClientPosition();
    position.y = viewport.m_maxs.y - position.y;

    return position;
}

Vec2 InputSystem::GetMouseClientDelta() const
{
    return m_mouseDelta;
}

int InputSystem::GetMouseWheel() const
{
    return m_mouseWheel;
}

void InputSystem::SetMouseMode(bool hidden, bool clipped, bool relative)
{
    if (Window::s_activeWindow)
        Window::s_activeWindow->SetMouseMode(hidden, clipped, relative);
    else
        Window::s_mainWindow->SetMouseMode(hidden, clipped, relative);
}

void InputSystem::ReportMouseMode(bool hidden, bool clipped, bool relative)
{
    m_isStateModified = m_isMouseRelative != relative;
    m_isMouseHidden = hidden;
    m_isMouseClipped = clipped;
    m_isMouseRelative = relative;
}

void InputSystem::ReportMouseDelta(const Vec2& delta)
{
    if (m_isStateModified)
    {
        m_isStateModified = false;
        m_mouseDelta = Vec2();
    }
    else
	{
		m_mouseDelta = delta;
    }
}

void InputSystem::ReportMousePosition(const Vec2& position)
{
    m_mousePosition = position;
}

void InputSystem::ReportMouseWheel(int value)
{
    m_mouseWheel = value;
}

void InputSystem::UpdateXboxControllerState()
{
    for (int index = 0; index < NUM_XBOX_CONTROLLERS; index++)
    {
        m_xobxControllers[index].Update();
    }
}

