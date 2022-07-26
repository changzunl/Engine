#include "KeyButtonState.hpp"
#include "AnalogJoystick.hpp"

class InputSystem;

enum class XboxButtonID
{
	 XBOXBTN_UP,
	 XBOXBTN_DOWN,
	 XBOXBTN_LEFT,
	 XBOXBTN_RIGHT,
	 XBOXBTN_START,
	 XBOXBTN_BACK,
	 XBOXBTN_LTHUMB,
	 XBOXBTN_RTHUMB,
	 XBOXBTN_LSHOULDER,
	 XBOXBTN_RSHOULDER,
	 XBOXBTN_A,
	 XBOXBTN_B,
	 XBOXBTN_X,
	 XBOXBTN_Y,
	 NUM,
};

constexpr unsigned int NUM_XBOX_BUTTONS = (unsigned int) XboxButtonID::NUM;

class XboxController 
{
	friend class InputSystem;

public:
	XboxController();
	~XboxController();
	int GetControllerID() const                            { return m_controllerID; }
	bool IsConnected() const                               { return m_isConnected; }
	const AnalogJoystick& GetLeftJoystick() const          { return m_leftJoystick; }
	const AnalogJoystick& GetRightJoystick() const         { return m_rightJoystick; }
	float GetLeftTrigger() const                           { return m_leftTrigger; }
	float GetRightTrigger() const                          { return m_rightTrigger; }
	bool IsButtonDown(XboxButtonID button) const;
	bool WasButtonJustPressed(XboxButtonID button) const;
	bool WasButtonJustReleased(XboxButtonID button) const;

private:
	void Reset();
	void Update();
	void UpdateButton(XboxButtonID buttonID, const unsigned int& rawButtonStates);

private:
	int            m_controllerID                 = 0;
	bool           m_isConnected                  = false;
	KeyButtonState m_buttons[NUM_XBOX_BUTTONS]    = {};
	float          m_leftTrigger                  = 0.f;
	float          m_rightTrigger                 = 0.f;
	AnalogJoystick m_leftJoystick;
	AnalogJoystick m_rightJoystick;
};

