#include "KeyInput.h"

void KeyInput::SetWindow(HWND hWnd)
{
	m_mouse.SetWindow(hWnd);
}

void KeyInput::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	// Mouse
	switch (message)
	{
	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
	};

	// Keyboard
	switch (message)
	{
	case WM_ACTIVATE:
	case WM_ACTIVATEAPP:
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
		break;

	case WM_SYSKEYDOWN:
		/*
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
		// This is where you'd implement the classic ALT+ENTER hotkey for fullscreen toggle
		...
		}
		*/
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
		break;
	}
}

void KeyInput::Update()
{
	m_keyboardState = m_keyboard.GetState();
	m_keyboardTracker.Update(m_keyboardState);

	m_mouseState = m_mouse.GetState();
	m_mouseTracker.Update(m_mouseState);

	m_gamepadState = m_gamepad.GetState(0);

	//データ入力
	m_keyDatas.push_back(m_keyboardState);
	m_mouseDatas.push_back(m_mouseState);

	//ゲームパッドは存在しているときのみ
	if (m_gamepadState.IsConnected() == true)
	{
		m_padDatas.push_back(m_gamepadState);
	}

	if (m_keyDatas.size() > m_MaxDataLength)
	{
		m_keyDatas.erase(m_keyDatas.begin());
	}
	if (m_mouseDatas.size() > m_MaxDataLength)
	{
		m_mouseDatas.erase(m_mouseDatas.begin());
	}

	if (m_padDatas.size() > m_MaxDataLength)
	{
		m_padDatas.erase(m_padDatas.begin());
	}
	/*
	if (m_mouseState.rightButton && m_mouseState.positionMode == DirectX::Mouse::MODE_RELATIVE)
	{
		char s[1000];
		sprintf_s(s, 1000, "%d\n", m_mouseState.x);
		OutputDebugStringA(s);
	}
	*/
}
