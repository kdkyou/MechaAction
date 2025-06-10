#include "CameraBase.h"

void CameraBase::Init()
{
	if (!m_spCamera)
	{
		m_spCamera = std::make_shared<KdCamera>();
	}
	// ↓画面中央座標
	m_FixMousePos.x = 640;
	m_FixMousePos.y = 360;

	KdEffekseerManager::GetInstance().SetCamera(m_spCamera);

}

void CameraBase::PreDraw()
{
	if (!m_spCamera) { return; }

	m_spCamera->SetCameraMatrix(m_mWorld);
	m_spCamera->SetToShader();
}

void CameraBase::SetTarget(const std::weak_ptr<KdGameObject>& target)
{
	if (target.expired() == true) { return; }

	m_wpTarget = target;
}

void CameraBase::SetRockTarget(const std::weak_ptr<KdGameObject>& target)
{
	if (target.expired() == true) { return; }

	m_wpRockTarget = target;
}

void CameraBase::UpdateRotateByMouse()
{
	// マウスでカメラを回転させる処理
	POINT _nowPos;

	GetCursorPos(&_nowPos);

	auto& pad = KeyInput::GetInstance().GetGamePadState();

	{
		if (pad.IsRightThumbStickDown()) { _nowPos.y += m_mouseSpeed.y; }
		if (pad.IsRightThumbStickUp()) { _nowPos.y -= m_mouseSpeed.y; }
		if (pad.IsRightThumbStickLeft()) { _nowPos.x -= m_mouseSpeed.x; }
		if (pad.IsRightThumbStickRight()) { _nowPos.x += m_mouseSpeed.x; }
	}

	POINT _mouseMove{};
	_mouseMove.x = _nowPos.x - m_FixMousePos.x;
	_mouseMove.y = _nowPos.y - m_FixMousePos.y;

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	// 実際にカメラを回転させる処理(0.15はただの補正値)
	m_DegAng.x += _mouseMove.y * 0.15f;
	m_DegAng.y += _mouseMove.x * 0.15f;

	// 回転制御
	m_DegAng.x = std::clamp(m_DegAng.x, -45.f, 45.f);
}