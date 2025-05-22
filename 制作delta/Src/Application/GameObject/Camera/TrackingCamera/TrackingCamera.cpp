#include"../CameraBase.h"
#include"TrackingCamera.h"

void TrackingCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateTranslation(3, 12.5f, -10.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	m_pos = {};
}

void TrackingCamera::PostUpdate()
{
	Math::Vector3 vMove=Math::Vector3::Zero;

	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpTarget.lock();
	
	if (_spTarget == nullptr) { return; }

	Math::Vector3 targetPos = _spTarget->GetMatrix().Translation();



	m_pos = Math::Vector3::Lerp(
		m_pos,
		targetPos,
		m_speed*KdFPSController::GetInstance().GetDeltaTime()			//進行速度*デルタタイム
		);

	UpdateRotateByMouse();
	m_mRotation = GetRotationMatrix();

	m_mWorld = m_mLocalPos *m_mRotation * Math::Matrix::CreateTranslation(m_pos);
}

