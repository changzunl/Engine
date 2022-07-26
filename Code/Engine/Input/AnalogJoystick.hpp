#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/PolarPosition2D.hpp"

class AnalogJoystick 
{
public:
	AnalogJoystick() {}
	~AnalogJoystick() {}
	void Reset();
	void UpdatePosition(float rawPositionX, float rawPositionY);
	void SetDeadZone(float innerDeadZoneFraction, float outerDeadZoneFraction);

	Vec2  GetPositionRaw() const                                                  { return m_rawPosition; }
	float GetInnerDeadZoneFraction() const                                        { return m_innerDeadZoneFraction; }
	float GetOuterDeadZoneFraction() const                                        { return m_outerDeadZoneFraction; }
	Vec2  GetPositionCorrected() const                                            { return m_correctedPosition; }
	float GetOrientationDegrees() const                                           { return m_correctedPositionPolar.angleDegrees; }
	float GetStrengthCorrected() const                                            { return m_correctedPositionPolar.radius; }

private:
	void CorrectWithDeadZone();

private:
	Vec2 m_rawPosition;
	Vec2 m_correctedPosition;
	PolarPosition2D m_correctedPositionPolar;
	float m_innerDeadZoneFraction = 0.30f;
	float m_outerDeadZoneFraction = 0.95f;
};

