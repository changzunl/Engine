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

    g_theEventSystem->Subscribe("ExecuteCommand", [this](auto args)
        {
            auto command = args.GetValue("cmd", "");
            if (command.empty())
            {
                AddLine(LOG_WARN, Stringf("Command not provided."));
                return false;
            }

            Execute(command);

            return true;
        }, this);

	g_theEventSystem->Subscribe("RunScriptFile", [this](auto args)
		{
			auto path = args.GetValue("path", "");
			if (path.empty())
			{
				AddLine(LOG_WARN, Stringf("File path not provided."));
				return false;
			}

			ExecuteXmlCommandScriptFile(path);

			return true;
		}, this);

    g_theEventSystem->Subscribe("RunScriptNode", [this](auto args)
        {
            auto node = args.GetValue<const XmlElement*>("node", nullptr);
            if (!node)
            {
                AddLine(LOG_WARN, Stringf("Node not provided."));
                return false;
            }

            ExecuteXmlCommandScriptNode(*node);

            return true;
        }, this);
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

// bug: using xml parser requires all arguments to have consist quotes around value string
void ParseCommand(const std::string& line, std::string& cmd, EventArgs& args)
{
	if (line.find('"') == std::string::npos)
	{
		auto cmds = ParseStringOnSpace(line);
		auto ite = cmds.begin();
		cmd = *(ite++);

		while (ite != cmds.end())
		{
			auto arg = ParseArgumentOnEquals(*(ite++));
			args.SetValue(arg[0], arg[1]);
		}

		return;
	}

    XmlDocument xml;
    auto error = xml.Parse(Stringf("<%s />", line.c_str()).c_str());

    if (error)
    {
        return;
    }

	cmd = xml.RootElement()->Name();

    for (auto attr = xml.RootElement()->FirstAttribute(); attr; attr = attr->Next())
    {
        args.SetValue(attr->Name(), attr->Value());
    }
}

bool ParseCommandPairs(const std::string& line, EventArgs& args);

bool ParseCommand2(const std::string& line, std::string& cmd, EventArgs& args)
{
	auto pos = line.find(' ');

	if (pos != std::string::npos)
	{
		cmd = line.substr(0, pos);

		auto arg = line.substr(pos + 1);

		return ParseCommandPairs(arg, args);
	}
	else
	{
		if (line.find('=') != std::string::npos || line.find('"') != std::string::npos)
			return false;

		cmd = line;
		return true;
	}
}

bool ParseCommandPairs(const std::string& line, EventArgs& args)
{
    std::vector<std::string> list;
    std::vector<char> buffer;
    buffer.reserve(line.size());

    bool inValue = false;
    bool inQoute = false;
    bool inSpace = false;

    for (auto c : line)
    {
		if (inSpace)
		{
			if (c != ' ')
				return false;

			inSpace = false;
		}

		if (inValue)
		{
            if (inQoute)
            {
                if (c == '"')
                {
                    inQoute = false;
                    inValue = false;
                    list.emplace_back(buffer.begin(), buffer.end());
                    buffer.clear();
					inSpace = true;
                }
                else
                {
                    buffer.push_back(c);
                }
            }
            else if (c == '"')
            {
                if (buffer.size())
                    return false; // space should only be in front of a key

				inQoute = true;
            }
            else if (c == ' ')
            {
                inValue = false;
                list.emplace_back(buffer.begin(), buffer.end());
                buffer.clear();
            }
			else
            {
                buffer.push_back(c);
			}
		}
		else
		{
			if (c == '=')
			{
				inValue = true;
				list.emplace_back(buffer.begin(), buffer.end());
				buffer.clear();
			}
			else if (c == '"')
			{
				return false; // qoute should not be in key
            }
            else if (c == ' ')
            {
				if (buffer.size())
					return false; // space should only be in front of a key
            }
			else
			{
				buffer.push_back(c);
			}
		}
    }

	if (inValue)
	{
		if (inQoute)
			return false;

        list.emplace_back(buffer.begin(), buffer.end());
	}

	if ((list.size() & 1) != 0)
		return false;

	for (size_t i = 0; i < list.size(); i += 2)
	{
		args.SetValue(list[i], list[i + 1]);
	}

	return true;
}

void DevConsole::Shutdown()
{
	g_theEventSystem->Unsubscribe(this);
}

void DevConsole::Execute(const std::string& consoleCommandText, PermissionLevel permission /*= PERMISSION_ROOT*/)
{
	StringList cmds = SplitStringOnDelimiter(consoleCommandText, '\n');
	for (const std::string& cmd : cmds)
    {
		std::string cmdName;
        EventArgs eventArgs;

		bool result = ParseCommand2(cmd, cmdName, eventArgs);

		if (!result)
        {
            AddLine(DevConsole::LOG_WARN, Stringf("Malformed command line: %s", cmd.c_str()));
			continue;
		}

		if (permission != PERMISSION_ROOT)
        {
            auto& banList = m_bannedCmds[permission];
            auto ite = std::find(banList.begin(), banList.end(), cmdName);
            if (ite != banList.end())
            {
                AddLine(DevConsole::LOG_WARN, Stringf("Banned command: %s", cmdName.c_str()));
                continue;
            }
		}
		
		result = g_theEventSystem->FireEvent(cmdName, eventArgs);
		if (!result)
        {
            AddLine(DevConsole::LOG_WARN, Stringf("Unknown command: %s", cmdName.c_str()));
		}
	}
}

void DevConsole::AddLine(const Rgba8& color, const std::string& text)
{
	if (m_netMesssageSink)
		m_netMesssageSink(color, text);

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

void DevConsole::SetNetMessageSink(MessageSink ptr)
{
	m_netMesssageSink = ptr;
}

void DevConsole::SetBannedCmds(PermissionLevel level, CmdList cmds)
{
    m_bannedCmds[level] = cmds;
}

void DevConsole::ExecuteXmlCommandScriptNode(const XmlElement& commandScriptXmlElement)
{
	for (auto child = commandScriptXmlElement.FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
	{
		auto cmd = child->Name();
		EventArgs args;
		for (auto attr = child->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
			args.SetValue(attr->Name(), attr->Value());
		}

        bool result = g_theEventSystem->FireEvent(cmd, args);
        if (!result)
        {
            AddLine(DevConsole::LOG_WARN, Stringf("Unknown command: %s", cmd));
        }
	}
}

void DevConsole::ExecuteXmlCommandScriptFile(const std::string& commandScriptXmlFilePathName)
{
	XmlDocument doc;
    auto error = doc.LoadFile(commandScriptXmlFilePathName.c_str());

	if (error)
	{
		AddLine(DevConsole::LOG_WARN, Stringf("Error running script file: %s", commandScriptXmlFilePathName.c_str()));
		return;
	}

	XmlElement* root = doc.RootElement();

	if (!root)
    {
        AddLine(DevConsole::LOG_WARN, Stringf("No root element found in script file: %s", commandScriptXmlFilePathName.c_str()));
        return;
	}

	ExecuteXmlCommandScriptNode(*root);
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
