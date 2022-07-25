#pragma once

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <thread>

extern const unsigned char KEYCODE_TILDE;

struct AABB2;
class Renderer;
class BitmapFont;

enum class DevConsoleMode
{
    HIDDEN,
    SHOWING,
};

class DevConsole;

struct DevConsoleLine
{
    friend class DevConsole;

public:
    DevConsoleLine(int frameNumber, double timeSinceStart, const Rgba8& color, const std::string& text);
    ~DevConsoleLine();

private:
    int         m_frameNumber;
    double      m_timeSinceStart;
    Rgba8       m_color;
    std::string m_text;
};

struct DevConsoleConfig
{
public:
    Renderer*         m_renderer       = nullptr;
    BitmapFont*       m_font           = nullptr;
    int               m_linesPerScreen = 30;
    char              m_triggerKey     = KEYCODE_TILDE;
};

class DevConsole
{
public:
    DevConsole(const DevConsoleConfig& theConfig);
    ~DevConsole();

    // lifecycle
    void           Startup();
    void           BeginFrame();
    void           EndFrame();
    void           Shutdown();
 
    void           Execute(const std::string& consoleCommandText);
    void           AddLine(const Rgba8& color, const std::string& text); // async supported
    void           Render(const AABB2& bounds, Renderer* rendererOverride = nullptr) const;

    DevConsoleMode GetMode() const                                                          { return m_mode; }
    void           SetMode(DevConsoleMode mode);
    void           ToggleMode(DevConsoleMode mode);

    bool           OnCharInput(char charCode);
    bool           OnKeyPressed(char keyCode);
    
    bool           PrintHelpMessage(const std::string& filter);
    bool           ClearLines();
    BitmapFont*    GetFont() const;
    const Clock*   GetClock() const;

public:
    static const Rgba8             LOG_ERROR;
    static const Rgba8             LOG_WARN;
    static const Rgba8             LOG_INFO;
    static const Rgba8             LOG_FINE;

private:
    DevConsoleConfig               m_theConfig;
    DevConsoleMode                 m_mode                                                   = DevConsoleMode::HIDDEN;
    std::vector<DevConsoleLine>    m_lines;
    int                            m_frameNumber                                            = 0;
    Clock                          m_clock;

    std::string                    m_inputText;
    int                            m_caretPosition                                          = 0;
    bool                           m_caretVisible                                           = true;
    Stopwatch                      m_caretStopwatch;

    int                            m_maxCommandHistory                                      = 128;
    std::vector< std::string >     m_commandHistory;
    int                            m_historyIndex                                           = -1;

    // for handling async call to print line
    std::thread::id                m_mainThreadId;
    std::mutex                     m_asyncMutex;
	std::vector<DevConsoleLine>    m_asyncLines;

};

