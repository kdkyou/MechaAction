#include "TPSCamera.h"

#include"../../Character/CharacterBase.h"

#include "../../../Scene/SceneManager.h"

void TPSCamera::Editor_ImGui()
{
	CameraBase::Editor_ImGui();
	ImGui::DragFloat3((const char*)u8"相対座標", &m_localPos.x);
}

void TPSCamera::Deserialize(const nlohmann::json& jsonObj)
{
	CameraBase::Deserialize(jsonObj);

	KdJsonUtility::GetArray(jsonObj, "LocalPos", &m_localPos.x, 3);
}

void TPSCamera::Serialize(nlohmann::json& outJson) const
{
	CameraBase::Serialize(outJson);
	outJson["LocalPos"] = KdJsonUtility::CreateArray(&m_localPos.x, 3);

}

void TPSCamera::Init()
{
	// 親クラスの初期化呼び出し
	if (!m_spCamera)
	{
		m_spCamera = std::make_shared<KdCamera>();
	}

	// ↓画面中央座標
	m_FixMousePos.x = 640;
	m_FixMousePos.y = 360;

	KdEffekseerManager::GetInstance().SetCamera(m_spCamera);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	m_name = "TPS";

	ShowCursor(false);
}

void TPSCamera::PostUpdate()
{
	// ターゲットの行列(有効な場合利用する)
	Math::Matrix								_targetMat = Math::Matrix::Identity;
	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpLook.lock();
	if (_spTarget)
	{
		_targetMat = Math::Matrix::CreateTranslation(_spTarget->GetPos());
	}

	//// カメラの回転
	auto ang = m_rot * KdToRadians;
	m_mLocalPos = Math::Matrix::CreateTranslation(m_localPos);
	m_mRotation = Math::Matrix::CreateFromYawPitchRoll(ang);
	m_mWorld = m_mLocalPos * m_mRotation * _targetMat;

	
	CameraBase::PostUpdate();
}
