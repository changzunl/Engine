#include "XboxController.hpp"

#include <Windows.h>
#include <Xinput.h>
#pragma comment( lib, "xinput9_1_0" )

const unsigned int XINPUT_BUTTON_MASK[NUM_XBOX_BUTTONS] =
{
	XINPUT_GAMEPAD_DPAD_UP,
	XINPUT_GAMEPAD_DPAD_DOWN,
	XINPUT_GAMEPAD_DPAD_LEFT,
	XINPUT_GAMEPAD_DPAD_RIGHT,
	XINPUT_GAMEPAD_START,
	XINPUT_GAMEPAD_BACK,
    XINPUT_GAMEPAD_LEFT_THUMB,
    XINPUT_GAMEPAD_RIGHT_THUMB,
    XINPUT_GAMEPAD_LEFT_SHOULDER,
    XINPUT_GAMEPAD_RIGHT_SHOULDER,
    XINPUT_GAMEPAD_A,
    XINPUT_GAMEPAD_B,
    XINPUT_GAMEPAD_X,
    XINPUT_GAMEPAD_Y,
};

XboxController::XboxController()
{
}

XboxController::~XboxController()
{
}

void XboxController::Update()
{
	XINPUT_STATE xboxControllerState = {};
	DWORD errorStatus = XInputGetState(m_controllerID, &xboxControllerState);
	if (errorStatus == ERROR_SUCCESS)
	{
		m_isConnected = true;

		m_leftJoystick.UpdatePosition(xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY);
		m_rightJoystick.UpdatePosition(xboxControllerState.Gamepad.sThumbRX, xboxControllerState.Gamepad.sThumbRY);
		m_leftTrigger = static_cast<float>(xboxControllerState.Gamepad.bLeftTrigger) / 255.f;
		m_rightTrigger = static_cast<float>(xboxControllerState.Gamepad.bRightTrigger) / 255.f;
		for (int buttonIndex = 0; buttonIndex < NUM_XBOX_BUTTONS; buttonIndex++)
		{
			UpdateButton((XboxButtonID) buttonIndex, xboxControllerState.Gamepad.wButtons);
		}
	}
	else if (errorStatus == ERROR_DEVICE_NOT_CONNECTED)
	{
		Reset();
	}
	else
	{
		Reset();
	}

}

void XboxController::UpdateButton(XboxButtonID buttonID, const unsigned int& rawButtonStates)
{
    const unsigned int buttonIDInt = (unsigned int) buttonID;
    const unsigned int& mask = XINPUT_BUTTON_MASK[buttonIDInt];
    m_buttons[buttonIDInt].m_isPressed = (rawButtonStates & mask) == mask;
}

void XboxController::Reset()
{
    m_isConnected = false;
    m_leftJoystick.Reset();
    m_rightJoystick.Reset();
    m_leftTrigger = 0;
    m_rightTrigger = 0;
    for (int buttonIndex = 0; buttonIndex < NUM_XBOX_BUTTONS; buttonIndex++)
    {
        m_buttons[buttonIndex].m_wasPressedLastFrame = m_buttons[buttonIndex].m_isPressed = false;
    }
}

bool XboxController::IsButtonDown(XboxButtonID button) const
{
	return m_buttons[(int)button].m_isPressed;
}

bool XboxController::WasButtonJustPressed(XboxButtonID button) const
{
	return m_buttons[(int)button].m_isPressed && !m_buttons[(int)button].m_wasPressedLastFrame;
}

bool XboxController::WasButtonJustReleased(XboxButtonID button) const
{
	return !m_buttons[(int)button].m_isPressed && m_buttons[(int)button].m_wasPressedLastFrame;
}

