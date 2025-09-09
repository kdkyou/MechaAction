
#include"TrackingCamera.h"

#include "../../../main.h"

#include"../../Character/CharacterBase.h"

#include "../CameraManager.h"


void TrackingCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateTranslation(m_localPos);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);


	if (m_wpTarget.expired() == false)
	{
		m_mWorld = m_mLocalPos * m_wpTarget.lock()->GetMatrix();

		m_pos = m_wpTarget.lock()->GetMatrix().Translation();
		
		m_DegAng;
	}


	m_name = "Tracking";

	ShowCursor(false);

}

void TrackingCamera::PostUpdate()
{
	ShowCursor(false);


	Math::Vector3 vMove=Math::Vector3::Zero;

	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpTarget.lock();
	
	if (_spTarget == nullptr) { return; }

	Math::Vector3 targetPos = _spTarget->GetMatrix().Translation();

	auto targetVec = CameraManager::Instance().GetLocalDirectionTo(targetPos);
	if (targetVec.y < 0) {
		m_speed = 15.0f;
		m_mLocalPos = Math::Matrix::CreateTranslation(m_basePos);
	}
	else {
		m_speed = 10.0f;
		m_mLocalPos = Math::Matrix::CreateTranslation(m_localPos);
	}
	
	Application::Instance().m_log.AddLog("CamSpeed:%.f\n", m_speed);

	m_pos = Math::Vector3::Lerp(
		m_pos,
		targetPos,
		m_speed*KdFPSController::GetInstance().GetDeltaTime()			//進行速度*デルタタイム
		);

	UpdateRotateByMouse();
	m_mRotation = GetRotationMatrix();

	m_mWorld = m_mLocalPos *m_mRotation * Math::Matrix::CreateTranslation(m_pos);

	CameraBase::PostUpdate(); 
}

void TrackingCamera::Editor_ImGui()
{
	ImGui::SliderFloat("LerpSpeed", &m_speed, 0.0f, 100.0f);
}