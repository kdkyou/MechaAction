#pragma once

#include "Keyboard.h"
#include "Mouse.h"
#include "GamePad.h"

//===========================================
// 
// 入力
// 
//===========================================
class KeyInput
{
public:

	// マウス
	const DirectX::Mouse::State& GetMouseState() const { return m_mouseState; }
	DirectX::Mouse& GetMouse() { return m_mouse; }
	DirectX::Mouse::ButtonStateTracker& GetMouseTracker() { return m_mouseTracker; }
	const std::list<DirectX::Mouse::State>& GetMouseStateData(){return m_mouseDatas; }

	// キーボード
	const DirectX::Keyboard::State& GetKeyboardState() const { return m_keyboardState; }
	DirectX::Keyboard& GetKeyboard() { return m_keyboard; }
	DirectX::Keyboard::KeyboardStateTracker& GetKeyboardTracker() { return m_keyboardTracker; }
	const std::list<DirectX::Keyboard::State>& GetKeyboardStateData(){return m_keyDatas; }

	// ゲームパッド
	const DirectX::GamePad::State& GetGamePadState() const { return m_gamepadState; }
	DirectX::GamePad& GetGamePad() { return m_gamepad; }
	DirectX::GamePad::ButtonStateTracker& GetPadButtonTracker() { return m_gamepadTracker; }
	const std::list<DirectX::GamePad::State>& GetGamePadStateData(){return m_padDatas; }
	

	//保存された入力データ数の変更
	void SetDataLength(int length) { m_MaxDataLength = length; }
	const int GetDataLength()const { return m_MaxDataLength; }

	void SetWindow(HWND hWnd);

	void ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

	void Update();

	bool IsDashingRightFrequently();
	bool IsDashingLeftFrequently();
	bool IsAttackingFrequently();

private:

	DirectX::Mouse::State				m_mouseState = {};
	DirectX::Mouse						m_mouse = {};
	DirectX::Mouse::ButtonStateTracker	m_mouseTracker = {};

	DirectX::Keyboard::State				m_keyboardState = {};
	DirectX::Keyboard						m_keyboard = {};
	DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker = {};

	DirectX::GamePad::State					m_gamepadState = {};
	DirectX::GamePad						m_gamepad = {};
	DirectX::GamePad::ButtonStateTracker m_gamepadTracker = {};

	//	DirectX::GamePad::GamePadStateTracker;



	std::list<DirectX::Keyboard::State>	m_keyDatas;
	std::list<DirectX::Mouse::State>	m_mouseDatas;
	std::list<DirectX::GamePad::State>	m_padDatas;
	UINT								m_MaxDataLength = 40;

	//-------------------------------
	// シングルトン
	//-------------------------------
private:
	KeyInput()
	{
	}
public:
	static KeyInput& GetInstance() {
		static KeyInput instance;
		return instance;
	}

};