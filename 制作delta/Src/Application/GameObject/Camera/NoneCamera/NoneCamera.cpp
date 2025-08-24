#include"NoneCamera.h"

void NoneCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(90)) * Math::Matrix::CreateTranslation(0, 50.0f, 0.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	m_name = "None";

	ShowCursor(false);
}

void NoneCamera::PostUpdate()
{

	m_mWorld = Math::Matrix::CreateFromYawPitchRoll(m_rot * KdToRadians) * Math::Matrix::CreateTranslation(m_pos);

	CameraBase::PostUpdate();
}

void NoneCamera::Deserialize(const nlohmann::json& jsonObj)
{
	KdJsonUtility::GetArray(jsonObj, "Pos", &m_pos.x, 3);
	KdJsonUtility::GetArray(jsonObj, "Rot", &m_rot.x, 3);
	KdJsonUtility::GetArray(jsonObj, "Scale", &m_scale.x, 3);
}

void NoneCamera::Serialize(nlohmann::json& outJson) const
{
	CameraBase::Serialize(outJson);
}
