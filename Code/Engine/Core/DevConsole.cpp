#include "Engine/Core/DevConsole.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/KeyCodes.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"

DevConsole* g_theConsole = nullptr;

extern bool* g_debugRendererShow;

void DebugRenderSetVisible();
void DebugRenderSetHidden();
void DebugRenderClear();

DevConsole::DevConsole(const DevConsoleConfig& theConfig)
	: m_theConfig(theConfig)
	, m_clock(Clock::GetSystemClock())
	, m_caretStopwatch(&m_clock, 0.75)
{
}

DevConsole::~DevConsole()
{

}

bool Event_KeyPressed(EventArgs& args)
{
	char keyCode = args.GetValue("key", "")[0];
	return g_theConsole->OnKeyPressed(keyCode);
}

bool Event_CharInput(EventArgs& args)
{
	char charCode = args.GetValue("char", "")[0];
	return g_theConsole->OnCharInput(charCode);
}

bool Command_Clear(EventArgs& args)
{
	UNUSED(args);

	return g_theConsole->ClearLines();
}

bool Command_Help(EventArgs& args)
{
	std::string filter = args.GetValue("filter", "");
	return g_theConsole->PrintHelpMessage(filter);
}

bool Command_Test(EventArgs& args)
{
	std::string cmd = "testcommand";
	cmd = cmd + " value1=" + args.GetValue("value1", "empty");
	cmd = cmd + " value2=" + args.GetValue("value2", "empty");

	g_theConsole->AddLine(DevConsole::LOG_INFO, Stringf("Console Command %s", cmd.c_str()));
	return true;
}

bool Command_DebugRendererClear(EventArgs& args)
{
	UNUSED(args);

	DebugRenderClear();
	return true;
}

bool Command_DebugRendererToggle(EventArgs& args)
{
	UNUSED(args);

	if (*g_debugRendererShow)
	{
		DebugRenderSetHidden();
	}
	else
	{
		DebugRenderSetVisible();
	}
	return true;
}

void DevConsole::Startup()
{
	m_mainThreadId = std::this_thread::get_id();

	g_theEventSystem->SubscribeEventCallbackFunction("clear",            Command_Clear);
	g_theEventSystem->SubscribeEventCallbackFunction("help",             Command_Help);
	g_theEventSystem->SubscribeEventCallbackFunction("testcommand",      Command_Test);
	g_theEventSystem->SubscribeEventCallbackFunction("Input:CharInput",  Event_CharInput);
	g_theEventSystem->SubscribeEventCallbackFunction("Input:KeyPressed", Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFunction("DebugRendererClear", Command_DebugRendererClear);
	g_theEventSystem->SubscribeEventCallbackFunction("DebugRendererToggle", Command_DebugRendererToggle);
}

void DevConsole::BeginFrame()
{
	if (m_caretStopwatch.CheckDurationElapsedAndDecrement())
	{
		m_caretVisible = !m_caretVisible;
	}

	{
		std::lock_guard<std::mutex> guard(m_asyncMutex);

		m_lines.insert(m_lines.end(), m_asyncLines.begin(), m_asyncLines.end());
		m_asyncLines.clear();
	}
}

void DevConsole::EndFrame()
{
	m_frameNumber++;
}

void DevConsole::Shutdown()
{
}

void DevConsole::Execute(const std::string& consoleCommandText)
{
	StringList cmds = SplitStringOnDelimiter(consoleCommandText, '\n');
	for (const std::string& cmd : cmds)
	{
		StringList cmdArgs = SplitStringOnDelimiter(cmd, ' ');
		EventArgs eventArgs = EventArgs();
		const std::string& cmdName = cmdArgs[0];
		for (int idx = 1; idx < cmdArgs.size(); idx++)
		{
			const std::string& argument = cmdArgs[idx];
			StringList keyValues = SplitStringOnDelimiter(argument, '=');
			if (keyValues.size() != 2)
			{
				g_theConsole->AddLine(DevConsole::LOG_WARN, Stringf("Invalid command argument : %s", cmd.c_str()));
				return;
			}
			else
			{
				eventArgs.SetValue(keyValues[0], keyValues[1]);
			}
		}
		g_theEventSystem->FireEvent(cmdName, eventArgs);
	}
}

void DevConsole::AddLine(const Rgba8& color, const std::string& text)
{
	if (std::this_thread::get_id() == m_mainThreadId)
	{
		// synchronized
		m_lines.emplace_back(m_frameNumber, GetCurrentTimeSeconds(), color, text);
	}
	else
	{
		// async
		std::lock_guard<std::mutex> guard(m_asyncMutex);

		m_asyncLines.emplace_back(m_frameNumber, GetCurrentTimeSeconds(), color, text);
	}
}

void DevConsole::Render(const AABB2& bounds, Renderer* rendererOverride /*= nullptr*/) const
{
	constexpr float CARET_WIDTH = 2.5f;
	constexpr float FONT_ASPECT = 0.65f;

	if (m_mode == DevConsoleMode::HIDDEN)
		return;

	Renderer* renderer = rendererOverride ? rendererOverride : m_theConfig.m_renderer;
	BitmapFont* font = m_theConfig.m_font;

	ASSERT_OR_DIE(renderer, "No renderer for dev console!");
	ASSERT_OR_DIE(font, "No font for dev console!");

	static std::vector<Vertex_PCU> s_verts;
	float lineHeight = bounds.GetDimensions().y / (float)(m_theConfig.m_linesPerScreen + 1);
	float lineSide = lineHeight * 0.05f;
	float textHeight = lineHeight * 0.9f;

	// draw console area & input area & carret
	AddVertsForAABB2D(s_verts, bounds, Rgba8(0, 0, 0, 80));

	AABB2 input(bounds);
	input.m_maxs.y = lineHeight + lineSide;
	AddVertsForAABB2D(s_verts, input, Rgba8(255, 255, 255, 80));

	if (m_caretVisible)
	{

		AABB2 carret(lineSide, 0, lineSide, 0);
		float carretPos = font->GetTextWidth(textHeight, m_inputText.substr(0, m_caretPosition), FONT_ASPECT);
		carret.m_mins.x += carretPos;
		carret.m_maxs.x += carretPos;
		carret.m_maxs.x += CARET_WIDTH;
		carret.m_maxs.y += lineHeight;

		AddVertsForAABB2D(s_verts, carret, Rgba8::WHITE);
	}

	renderer->BindTexture(nullptr);
	renderer->DrawVertexArray((int)s_verts.size(), s_verts.data());
	s_verts.clear();

	// draw command lines & input line
	for (int idx = 0; idx < m_lines.size() && idx < m_theConfig.m_linesPerScreen; idx++)
	{
		const DevConsoleLine& line = m_lines[m_lines.size() - idx - 1];
		font->AddVertsForText2D(s_verts, Vec2(lineSide, lineSide + lineHeight * (idx + 1)) + bounds.m_mins, textHeight, line.m_text, line.m_color, FONT_ASPECT);
	}

	font->AddVertsForText2D(s_verts, Vec2(lineSide, lineSide) + bounds.m_mins, textHeight, m_inputText, DevConsole::LOG_INFO, FONT_ASPECT);

	renderer->BindTexture(&font->GetTexture());
	renderer->DrawVertexArray((int)s_verts.size(), s_verts.data());
	renderer->BindTexture(nullptr);
	s_verts.clear();
}

void DevConsole::SetMode(DevConsoleMode mode)
{
	m_mode = mode;

	if (mode == DevConsoleMode::SHOWING)
	{
		static bool first = true;
		if (first)
		{
			first = false;
			AddLine(LOG_INFO, "Type help for a list of commands");
		}
	}
}

void DevConsole::ToggleMode(DevConsoleMode mode)
{
	if (m_mode != mode)
	{
		SetMode(mode);
	}
}

bool DevConsole::OnCharInput(char charCode)
{
	if (GetMode() == DevConsoleMode::SHOWING)
	{
// 		std::string text = "Char input:  ";
// 		*(text.end() - 1) = charCode;
// 		g_theConsole->AddLine(DevConsole::LOG_INFO, text);

		if (charCode >= 32 && charCode <= 126 && charCode != '`' && charCode != '~')
		{
			char str[2] = {};
			str[0] = charCode;
			m_inputText.insert(m_caretPosition++, &str[0]);
			m_caretStopwatch.Restart();
			m_caretVisible = true;
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool DevConsole::OnKeyPressed(char keyCode)
{
	if (GetMode() == DevConsoleMode::SHOWING)
	{
		// AddLine(LOG_INFO, Stringf("Key: %d", (unsigned char) keyCode));

		// exit
		if (keyCode == m_theConfig.m_triggerKey)
		{
			SetMode(DevConsoleMode::HIDDEN);
			return true;
		}
		if (keyCode == KEYCODE_ESC && m_inputText.size() == 0)
		{
			SetMode(DevConsoleMode::HIDDEN);
			return true;
		}

		int inputTextSize = (int)m_inputText.size();
		int commandHistorySize = (int)m_commandHistory.size();

		// caret move
		if (keyCode == KEYCODE_LEFT)
		{
			m_caretPosition = ClampInt(m_caretPosition - 1, 0, inputTextSize);
			m_caretStopwatch.Restart();
			m_caretVisible = true;
			return true;
		}
		if (keyCode == KEYCODE_RIGHT)
		{
			m_caretPosition = ClampInt(m_caretPosition + 1, 0, inputTextSize);
			m_caretStopwatch.Restart();
			m_caretVisible = true;
			return true;
		}
		if (keyCode == KEYCODE_HOME)
		{
			m_caretPosition = 0;
			m_caretStopwatch.Restart();
			m_caretVisible = true;
			return true;
		}
		if (keyCode == KEYCODE_END)
		{
			m_caretPosition = (int)m_inputText.size();
			m_caretStopwatch.Restart();
			m_caretVisible = true;
			return true;
		}

		// history move
		if (keyCode == KEYCODE_UP)
		{
			if (m_commandHistory.size() > 0)
			{
				m_historyIndex = ClampInt(m_historyIndex + 1, 0, commandHistorySize - 1);
				m_inputText = m_commandHistory[m_historyIndex];
				m_caretPosition = (int)m_inputText.size();
				m_caretStopwatch.Restart();
				m_caretVisible = true;
			}
			return true;
		}
		if (keyCode == KEYCODE_DOWN)
		{
			if (m_commandHistory.size() > 0)
			{
				m_historyIndex = ClampInt(m_historyIndex - 1, 0, commandHistorySize - 1);
				m_inputText = m_commandHistory[m_historyIndex];
				m_caretPosition = (int)m_inputText.size();
				m_caretStopwatch.Restart();
				m_caretVisible = true;
			}
			return true;
		}

		// caret delete
		if (keyCode == KEYCODE_DELETE)
		{
			if (inputTextSize > 0 && m_caretPosition < inputTextSize)
			{
				m_inputText.erase(m_caretPosition, 1);
				m_caretStopwatch.Restart();
				m_caretVisible = true;
			}
			return true;
		}
		if (keyCode == KEYCODE_BACKSPACE)
		{
			if (inputTextSize > 0 && m_caretPosition > 0)
			{
				m_inputText.erase(--m_caretPosition, 1);
				m_caretStopwatch.Restart();
				m_caretVisible = true;
			}
			return true;
		}
		if (keyCode == KEYCODE_ESC)
		{
			m_inputText = "";
			m_caretPosition = 0;
			m_caretStopwatch.Restart();
			m_caretVisible = true;
			return true;
		}

		// execute
		if (keyCode == KEYCODE_ENTER)
		{
			Execute(m_inputText);

			m_commandHistory.insert(m_commandHistory.begin(), m_inputText);
			if (m_commandHistory.size() > m_maxCommandHistory)
			{
				m_commandHistory.resize(m_maxCommandHistory);
			}
			m_historyIndex = -1;

			m_inputText = "";
			m_caretPosition = 0;
			m_caretStopwatch.Restart();
			m_caretVisible = true;
			return true;
		}

		m_caretStopwatch.Restart();
		m_caretVisible = true;
		return true;
	}
	else
	{
		if (keyCode == m_theConfig.m_triggerKey)
		{
			SetMode(DevConsoleMode::SHOWING);
			m_caretStopwatch.Restart();
			m_caretVisible = true;
			return true;
		}

		return false;
	}
}

bool DevConsole::PrintHelpMessage(const std::string& filter)
{
	g_theConsole->AddLine(DevConsole::LOG_INFO, Stringf("Available commands(%s): ", filter.c_str()));

	StringList commands;
	g_theEventSystem->GetRegisteredEventNames(commands);

	for (auto& command : commands)
	{
		if (filter.size() == 0 || command.find(filter) != std::string::npos)
		{
			g_theConsole->AddLine(DevConsole::LOG_INFO, command);
		}
	}

	return true;
}

bool DevConsole::ClearLines()
{
	m_lines.clear();

	return true;
}

BitmapFont* DevConsole::GetFont() const
{
	return m_theConfig.m_font;
}

const Clock* DevConsole::GetClock() const
{
	return &m_clock;
}

const Rgba8 DevConsole::LOG_ERROR = Rgba8(255, 0, 0);
const Rgba8 DevConsole::LOG_WARN  = Rgba8(255, 255, 0);
const Rgba8 DevConsole::LOG_INFO  = Rgba8(255, 255, 255);
const Rgba8 DevConsole::LOG_FINE  = Rgba8(0, 255, 0);

DevConsoleLine::DevConsoleLine(int frameNumber, double timeSinceStart, const Rgba8& color, const std::string& text)
	: m_frameNumber(frameNumber)
	, m_timeSinceStart(timeSinceStart)
	, m_color(color)
	, m_text(text)
{

}

DevConsoleLine::~DevConsoleLine()
{

}
