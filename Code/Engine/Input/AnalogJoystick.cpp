#include "AnalogJoystick.hpp"

#include "Engine/Math/MathUtils.hpp"

void AnalogJoystick::Reset()
{
	m_rawPosition.x = m_rawPosition.y = 0;
	m_correctedPosition.x = m_correctedPosition.y = 0;
	m_correctedPositionPolar.radius = 0;
	m_correctedPositionPolar.angleDegrees = 0;
}

void AnalogJoystick::UpdatePosition(float rawPositionX, float rawPositionY)
{
	m_rawPosition.x = RangeMap(rawPositionX, -32768.f, 32767.f, -1.f, 1.f);
	m_rawPosition.y = RangeMap(rawPositionY, -32768.f, 32767.f, -1.f, 1.f);
	CorrectWithDeadZone();
}

void AnalogJoystick::SetDeadZone(float innerDeadZoneFraction, float outerDeadZoneFraction)
{
	m_innerDeadZoneFraction = innerDeadZoneFraction;
	m_outerDeadZoneFraction = outerDeadZoneFraction;
	CorrectWithDeadZone();
}

void AnalogJoystick::CorrectWithDeadZone()
{
	PolarPosition2D polarPosition = PolarPosition2D(m_rawPosition);
	polarPosition.radius = RangeMapClamped(polarPosition.radius, m_innerDeadZoneFraction, m_outerDeadZoneFraction, 0.f, 1.f);
	if (polarPosition.radius == 0)
	{
		polarPosition.angleDegrees = 0;
	}
	m_correctedPosition = polarPosition.ConvertToVec2();
	m_correctedPositionPolar = polarPosition;
}

