#pragma once

#define UNUSED(x) (void)(x);

class Clock;
class NamedStrings;
class DevConsole;
class EventSystem;

extern NamedStrings g_gameConfigBlackboard; // declared in EngineCommon.hpp, defined in EngineCommon.cpp
extern DevConsole*  g_theConsole;
extern EventSystem* g_theEventSystem;

extern Clock*       g_debugRendererClock;

