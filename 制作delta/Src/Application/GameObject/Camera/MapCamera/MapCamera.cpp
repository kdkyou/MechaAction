#include "MapCamera.h"

#include "../../../main.h"

#include "../../Character/CharacterBase.h"

void MapCamera::Init()
{
	CameraBase::Init();

	m_pos = { 0.0f,0.0f,0.0f };
	m_mLocalPos = Math::Matrix::CreateTranslation({ 0.0f,80.0f,0.0f });
	m_rot = { 90.0f,0.0f,0.0f };

	m_name = "Map";

	m_mWorld = m_mLocalPos * Math::Matrix::CreateFromYawPitchRoll(m_rot * KdToRadians) * Math::Matrix::CreateTranslation(m_pos);
}

void MapCamera::PostUpdate()
{
	// ターゲットの行列(有効な場合利用する)
	Math::Matrix								_targetMat = Math::Matrix::Identity;
	auto _spTarget = m_wpTarget.lock();
	if (_spTarget)
	{
		_targetMat = Math::Matrix::CreateTranslation(_spTarget->GetPos());
	}

	m_mWorld = Math::Matrix::CreateFromYawPitchRoll(m_rot * KdToRadians) * _targetMat * m_mLocalPos;
}

void MapCamera::Editor_ImGui()
{
	CameraBase::Editor_ImGui();
	ImGui::Text("MapCamera");
}

void MapCamera::Deserialize(const nlohmann::json& jsonObj)
{
	CameraBase::Deserialize(jsonObj);
}

void MapCamera::Serialize(nlohmann::json& outJson) const
{
	CameraBase::Serialize(outJson);
}