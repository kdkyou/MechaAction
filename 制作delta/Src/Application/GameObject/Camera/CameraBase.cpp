#include "CameraBase.h"

#include"CameraManager.h"

#include"../Character/CharacterBase.h"

void CameraBase::Init()
{
	if (!m_spCamera)
	{
		m_spCamera = std::make_shared<KdCamera>();
	}
	// ↓画面中央座標
	m_FixMousePos.x = 640;
	m_FixMousePos.y = 360;

	m_localPos = { 3, 14.5f, -22.0f };

	KdEffekseerManager::GetInstance().SetCamera(m_spCamera);

	ShowCursor(false);
}

void CameraBase::PostUpdate()
{
	if (!m_spCamera) { return; }

	KdAudioManager::Instance().SetListnerMatrix(m_mWorld);
}


void CameraBase::PreDraw()
{
	if (!m_spCamera) { return; }

	m_spCamera->SetCameraMatrix(m_mWorld);
	m_spCamera->SetToShader();
}

void CameraBase::SetLook(const std::weak_ptr<KdGameObject>& target)
{
	if (target.expired() == true) { return; }

	m_wpLook = target;
}

void CameraBase::SetTarget(const std::weak_ptr<CharacterBase>& target)
{
	if (target.expired() == true) { return; }

	m_wpTarget = target;
}

void CameraBase::SetLockTarget(const std::weak_ptr<CharacterBase>& target)
{
	if (target.expired() == true) { return; }

	m_wpLockTarget = target;
}

void CameraBase::Editor_ImGui()
{
	KdGameObject::Editor_ImGui();
}

void CameraBase::Deserialize(const nlohmann::json& jsonObj)
{
	KdJsonUtility::GetArray(jsonObj, "Pos", &m_pos.x, 3);
	KdJsonUtility::GetArray(jsonObj, "Rot", &m_rot.x, 3);
	KdJsonUtility::GetArray(jsonObj, "Scale", &m_scale.x, 3);

	
}

void CameraBase::Serialize(nlohmann::json& outJson) const
{
	outJson["Type"] = CameraManager::Instance().GetNowType();
	
	KdGameObject::Serialize(outJson);

}

void CameraBase::UpdateRotateByMouse()
{
	// マウスでカメラを回転させる処理
	POINT _nowPos;

	GetCursorPos(&_nowPos);
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	bool moveCamera = false;

	POINT _mouseMove{};
	_mouseMove.x = _nowPos.x - m_FixMousePos.x;
	_mouseMove.y = _nowPos.y - m_FixMousePos.y;

	if (_mouseMove.x > 0.0f || _mouseMove.y > 0.0f)
	{
		moveCamera = true;
	}

	if (pad.IsRightThumbStickDown()) {
		_mouseMove.y += m_mouseSpeed.y;
		moveCamera = true;
	}
	if (pad.IsRightThumbStickUp()) {
		_mouseMove.y -= m_mouseSpeed.y;
		moveCamera = true;
	}
	if (pad.IsRightThumbStickLeft()) {
		_mouseMove.x -= m_mouseSpeed.x;
		moveCamera = true;
	}
	if (pad.IsRightThumbStickRight()) {
		_mouseMove.x += m_mouseSpeed.x;
		moveCamera = true;
	}

	

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	// 実際にカメラを回転させる処理(0.15はただの補正値)
	m_DegAng.x += _mouseMove.y * 0.15f;
	m_DegAng.y += _mouseMove.x * 0.15f;

	// 回転制御
	m_DegAng.x = std::clamp(m_DegAng.x, -45.f, 45.f);
}