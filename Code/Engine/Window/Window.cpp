#include "Window.hpp"

#include "WindowConfig.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Input/InputSystem.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called by Windows whenever we ask it for notifications
//
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
    Window* window = Window::s_mainWindow;
    InputSystem* input = Window::s_mainWindow->m_theConfig.m_inputSystem;
    switch (wmMessageCode)
    {
        // App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
    case WM_CLOSE:
    {
        if (input->HandleQuitRequested()) return 0; // "Consumes" this message (tells Windows "okay, we handled it")

        break;
    }

    // Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
    case WM_KEYDOWN:
    {
        unsigned char asKey = (unsigned char)wParam;

        if (input->HandleKeyPressed(asKey)) return 0;

        break;
    }

    // Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
    case WM_KEYUP:
    {
        unsigned char asKey = (unsigned char)wParam;

        if (input->HandleKeyReleased(asKey)) return 0;

        break;
    }

    // Raw physical mouse "left-button-was-just-pressed" event
    case WM_LBUTTONDOWN:
    {
        unsigned char asKey = KEYCODE_LEFT_MOUSE;

        if (input->HandleKeyPressed(asKey)) return 0;

        break;
    }

    // Raw physical mouse "left-button-was-just-released" event
    case WM_LBUTTONUP:
    {
        unsigned char asKey = KEYCODE_LEFT_MOUSE;

        if (input->HandleKeyReleased(asKey)) return 0;

        break;
    }

    // Raw physical mouse "right-button-was-just-pressed" event
    case WM_RBUTTONDOWN:
    {
        unsigned char asKey = KEYCODE_RIGHT_MOUSE;

        if (input->HandleKeyPressed(asKey)) return 0;

        break;
    }

    // Raw physical mouse "right-button-was-just-released" event
    case WM_RBUTTONUP:
    {
        unsigned char asKey = KEYCODE_RIGHT_MOUSE;

        if (input->HandleKeyReleased(asKey)) return 0;

        break;
    }

    // Raw physical mouse "middle-button-was-just-pressed" event
    case WM_MBUTTONDOWN:
    {
        unsigned char asKey = KEYCODE_MIDDLE_MOUSE;

        if (input->HandleKeyPressed(asKey)) return 0;

        break;
    }

    // Raw physical mouse "middle-button-was-just-released" event
    case WM_MBUTTONUP:
    {
        unsigned char asKey = KEYCODE_MIDDLE_MOUSE;

        if (input->HandleKeyReleased(asKey)) return 0;

        break;
	}

	// Raw physical mouse "wheel move" event
	case WM_MOUSEWHEEL:
	{
        input->ReportMouseWheel((int(wParam) >> 16) / WHEEL_DELTA);
        return 0;
	}

    // Raw input char event
    case WM_CHAR:
    {
        int charCode = (int) wParam;

        if (input->HandleCharInput(charCode)) return 0;

        break;
    }

    // Raw input char event
    case WM_ACTIVATE:
    {
        bool active = wParam != WA_INACTIVE;

        window->SetWindowActive(active);
        return 0;

        break;
    }
    }

    // Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
    return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}

Window* Window::s_mainWindow = nullptr;

Window::Window(const WindowConfig& theConfig)
    : m_theConfig(theConfig)
{
    ASSERT_RECOVERABLE(s_mainWindow == nullptr, "Setting current window without clearing previous one!");
    s_mainWindow = this;
}

Window::~Window()
{
    if (s_mainWindow == this)
    {
        s_mainWindow = nullptr;
    }
}

void Window::Startup()
{
    CreateOSWindow();
}

void Window::BeginFrame()
{
    // Process OS messages (keyboard/mouse button clicked, application lost/gained focus, etc.)
    RunMessagePump(); // calls our own WindowsMessageHandlingProcedure() function for us!

    UpdateCursor();
}

void Window::EndFrame()
{

}

void Window::Shutdown()
{

}

void Window::UpdateCursor()
{
    if (!m_isActive)
        return;

	InputSystem* input = m_theConfig.m_inputSystem;

    CURSORINFO ci = { sizeof(CURSORINFO) };
    ::GetCursorInfo(&ci);

    bool mouseHidden = input->m_isMouseHidden;
    bool mouseClipped = input->m_isMouseClipped;
    bool mouseRelative = input->m_isMouseRelative;

    if (g_theConsole && g_theConsole->GetMode() == DevConsoleMode::SHOWING)
    {
        mouseHidden = false;
        mouseClipped = false;
        mouseRelative = false;
    }

    bool cursorHidden = ci.flags == 0;
    if (cursorHidden != mouseHidden)
	{
		while (true)
		{
			int count = ::ShowCursor(!mouseHidden);
			if (mouseHidden)
			{
                if (count < 0)
                    break;
			}
			else
			{
				if (count >= 0)
					break;
			}
		}
    }
    
    if (mouseRelative)
    {
        SetNormalizedCursorPos(Vec2(0.5f, 0.5f));

        CURSORINFO ci2 = { sizeof(CURSORINFO) };
        ::GetCursorInfo(&ci2);
        input->ReportMouseDelta(Vec2(float(ci.ptScreenPos.x - ci2.ptScreenPos.x), float(ci.ptScreenPos.y - ci2.ptScreenPos.y)));
    }

    if (mouseClipped)
	{
		HWND windowHandle = HWND(GetHwnd());
        RECT clientRect = {};
		::GetClientRect(windowHandle, &clientRect);
		POINT rectMin = {};
		POINT rectMax = {};
		rectMin.x = clientRect.left;
		rectMin.y = clientRect.top;
		rectMax.x = clientRect.right;
		rectMax.y = clientRect.bottom;
		::ClientToScreen(windowHandle, &rectMin);
		::ClientToScreen(windowHandle, &rectMax);
		clientRect.left = rectMin.x;
		clientRect.top = rectMin.y;
		clientRect.right = rectMax.x;
		clientRect.bottom = rectMax.y;
		::ClipCursor(&clientRect);
    }
    else
	{
		::ClipCursor(NULL);
	}

	HWND windowHandle = HWND(GetHwnd());
	CURSORINFO ci2 = { sizeof(CURSORINFO) };
	::GetCursorInfo(&ci2);
	::ScreenToClient(windowHandle, &ci2.ptScreenPos);
	input->ReportMousePosition(Vec2(float(ci2.ptScreenPos.x), float(ci2.ptScreenPos.y)));
}

InputSystem* Window::GetInputSystem() const
{
    return m_theConfig.m_inputSystem;
}

IntVec2 Window::GetClientDimensions() const
{
    return m_clientDimension;
}

Vec2 Window::GetNormalizedCursorPos() const
{
    HWND windowHandle = HWND(GetHwnd());
    POINT cursorCoords = {};
    RECT clientRect = {};
    ::GetCursorPos(&cursorCoords);
    ::ScreenToClient(windowHandle, &cursorCoords);
    ::GetClientRect(windowHandle, &clientRect);
    float cursorX = (float)cursorCoords.x / (float) clientRect.right;
    float cursorY = (float)cursorCoords.y / (float) clientRect.bottom;
    return Vec2(cursorX, 1.0f - cursorY);
}

bool Window::IsWindowActive() const
{
    return m_isActive;
}

void Window::SetNormalizedCursorPos(const Vec2& position) const
{
	HWND windowHandle = HWND(GetHwnd());
	POINT cursorCoords = {};
	RECT clientRect = {};
	::GetClientRect(windowHandle, &clientRect);
	cursorCoords.x = (long)(position.x * (float)clientRect.right);
	cursorCoords.y = (long)((1.0f - position.y) * (float)clientRect.bottom);
	::ClientToScreen(windowHandle, &cursorCoords);
	::SetCursorPos(cursorCoords.x, cursorCoords.y);
}

void Window::SetWindowActive(bool active)
{
    m_isActive = active;

    if (!active)
	{
		while (true)
		{
			int count = ::ShowCursor(true);
            if (count >= 0)
                break;
		}
        ::ClipCursor(NULL);
    }
    else
    {
		InputSystem* input = m_theConfig.m_inputSystem;

		CURSORINFO ci = { sizeof(CURSORINFO) };
		::GetCursorInfo(&ci);

		bool mouseHidden = input->m_isMouseHidden;
		bool mouseClipped = input->m_isMouseClipped;
		bool mouseRelative = input->m_isMouseRelative;

		if (g_theConsole && g_theConsole->GetMode() == DevConsoleMode::SHOWING)
		{
			mouseHidden = false;
			mouseClipped = false;
			mouseRelative = false;
		}

		bool cursorHidden = ci.flags == 0;
		if (cursorHidden != mouseHidden)
		{
            while (true)
            {
                int count = ::ShowCursor(!mouseHidden);
                if (mouseHidden)
                {
                    if (count < 0)
                        break;
                }
                else
                {
                    if (count >= 0)
                        break;
                }
            }
		}

		if (mouseRelative)
		{
			SetNormalizedCursorPos(Vec2(0.5f, 0.5f));
		}

		if (mouseClipped)
		{
			HWND windowHandle = HWND(GetHwnd());
			RECT clientRect = {};
			::GetClientRect(windowHandle, &clientRect);
			POINT rectMin = {};
			POINT rectMax = {};
			rectMin.x = clientRect.left;
			rectMin.y = clientRect.top;
			rectMax.x = clientRect.right;
			rectMax.y = clientRect.bottom;
			::ClientToScreen(windowHandle, &rectMin);
			::ClientToScreen(windowHandle, &rectMax);
			clientRect.left = rectMin.x;
			clientRect.top = rectMin.y;
			clientRect.right = rectMax.x;
			clientRect.bottom = rectMax.y;
			::ClipCursor(&clientRect);
		}
        else
		{
			::ClipCursor(NULL);
        }
    }
}

void Window::CreateOSWindow()
{
    // Define a window style/class
    HMODULE appInstanceHandle = ::GetModuleHandle(NULL);
    WNDCLASSEX windowClassDescription;
    memset(&windowClassDescription, 0, sizeof(windowClassDescription));
    windowClassDescription.cbSize = sizeof(windowClassDescription);
    windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
    windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
    windowClassDescription.hInstance = ::GetModuleHandle(NULL);
    windowClassDescription.hIcon = NULL;
    windowClassDescription.hCursor = NULL;
    windowClassDescription.lpszClassName = TEXT("Simple Window Class");
    RegisterClassEx(&windowClassDescription);

    // #SD1ToDo: Add support for full screen mode (requires different window style flags than windowed mode)
    const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_OVERLAPPED;
    const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

    // Get desktop rect, dimensions, aspect
    RECT desktopRect;
    HWND desktopWindowHandle = ::GetDesktopWindow();
    ::GetClientRect(desktopWindowHandle, &desktopRect);
    float desktopWidth = (float)(desktopRect.right - desktopRect.left);
    float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
    float desktopAspect = desktopWidth / desktopHeight;

    float clientAspect = m_theConfig.m_clientAspect;
    // Calculate maximum client size (as some % of desktop size)
    constexpr float maxClientFractionOfDesktop = 0.90f;
    float clientWidth = desktopWidth * maxClientFractionOfDesktop;
    float clientHeight = desktopHeight * maxClientFractionOfDesktop;
    if (clientAspect > desktopAspect)
    {
        // Client window has a wider aspect than desktop; shrink client height to match its width
        clientHeight = clientWidth / clientAspect;
    }
    else
    {
        // Client window has a taller aspect than desktop; shrink client width to match its height
        clientWidth = clientHeight * clientAspect;
    }

    // Calculate client rect bounds by centering the client area
    float clientMarginX = 0.5f * (desktopWidth - clientWidth);
    float clientMarginY = 0.5f * (desktopHeight - clientHeight);
    RECT clientRect;
    clientRect.left = (int)clientMarginX;
    clientRect.right = clientRect.left + (int)clientWidth;
    clientRect.top = (int)clientMarginY;
    clientRect.bottom = clientRect.top + (int)clientHeight;

    m_clientDimension.x = (int)clientWidth;
    m_clientDimension.y = (int)clientHeight;

    // Calculate the outer dimensions of the physical window, including frame et. al.
    RECT windowRect = clientRect;
    ::AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

    WCHAR windowTitle[1024];
    ::MultiByteToWideChar(GetACP(), 0, m_theConfig.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
    HWND hWnd = ::CreateWindowEx(
        windowStyleExFlags,
        windowClassDescription.lpszClassName,
        windowTitle,
        windowStyleFlags,
        windowRect.left,
        windowRect.top,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        appInstanceHandle,
        NULL);

    ::ShowWindow(hWnd, SW_SHOW);
    ::SetForegroundWindow(hWnd);
    ::SetFocus(hWnd);

    m_osWindowHandle = hWnd;
    m_isActive = true;

    HCURSOR cursor = ::LoadCursor(NULL, IDC_ARROW);
    ::SetCursor(cursor);
}

//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//    is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
//
void Window::RunMessagePump()
{
    MSG queuedMessage;
    for (;; )
    {
        const BOOL wasMessagePresent = ::PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
        if (!wasMessagePresent)
        {
            break;
        }

        ::TranslateMessage(&queuedMessage);
        ::DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
    }
}

