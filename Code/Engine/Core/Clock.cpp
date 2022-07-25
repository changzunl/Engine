#include "Engine/Core/Clock.hpp"

#include "Engine/Core/Time.hpp"

Clock g_systemClock(*(Clock*)nullptr);

Clock::Clock() : Clock(g_systemClock)
{
}

Clock::Clock(Clock& parent)
    : m_parent(&parent)
{
    if (m_parent)
    {
        m_parent->AddChild(this);
    }
}

Clock::~Clock()
{
	for (auto& child : m_children)
	{
		delete child;
	}
	m_children.clear();
	if (m_parent)
	{
		m_parent->RemoveChild(this);
	}
}

void Clock::SetParent(Clock& parent)
{
    m_parent->RemoveChild(this);
    m_parent = &parent;
    m_parent->AddChild(this);
}

void Clock::Pause()
{
    m_isPaused = true;
}

void Clock::Unpause()
{
    m_isPaused = false;
}

void Clock::TogglePause()
{
    m_isPaused = !m_isPaused;
}

void Clock::StepFrame()
{
    m_isPaused = false;
    m_pauseAfterFrame = true;
}

void Clock::SetTimeDilation(double dilationAmount)
{
    m_timeDilation = dilationAmount;
}

double Clock::GetDeltaTime() const
{
    return m_deltaTime;
}

double Clock::GetTotalTime() const
{
    return m_totalTime;
}

size_t Clock::GetFrameCount() const
{
    return m_frameCount;
}

bool Clock::IsPaused() const
{
    return m_isPaused;
}

double Clock::GetTimeDilation() const
{
    return m_timeDilation;
}

void Clock::SystemBeginFrame()
{
    g_systemClock.Tick();
}

Clock& Clock::GetSystemClock()
{
    return g_systemClock;
}

double Clamp(double input, double lower, double upper)
{
    return input < lower ? lower : (input > upper ? upper : input);
}

void Clock::Tick()
{
    double currentTime = GetCurrentTimeSeconds();
    double deltaTime = Clamp(currentTime - m_lastUpdateTime, 0.0, 0.1);
    m_lastUpdateTime = currentTime;

    Advance(deltaTime);
}

void Clock::Advance(double deltaTimeSeconds)
{
    double dialation = (m_isPaused ? 0.0 : 1.0) * m_timeDilation;

    m_deltaTime = deltaTimeSeconds * dialation;
    m_totalTime += m_deltaTime;
    m_frameCount++;

    for (auto& child : m_children)
    {
        child->Advance(m_deltaTime);
    }

    if (m_pauseAfterFrame)
    {
        m_pauseAfterFrame = false;
        m_isPaused = true;
    }
}

void Clock::AddChild(Clock* childClock)
{
    m_children.push_back(childClock);
}

void Clock::RemoveChild(Clock* childClock)
{
    std::vector<Clock*> children = m_children;
    m_children.clear();
    for (auto& child : children)
    {
        if (child != childClock)
        {
            m_children.push_back(child);
        }
    }
}
