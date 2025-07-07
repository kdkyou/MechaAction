#include"HitCamera.h"

#include"../CameraManager.h"

void HitCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateTranslation(3, 12.5f, -10.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);


	if (m_wpTarget.expired() == false)
	{
		m_mWorld = m_mLocalPos * m_wpTarget.lock()->GetMatrix();

		m_pos = m_wpTarget.lock()->GetMatrix().Translation();

		m_DegAng;
	}


	m_name = "Tracking";

	ShowCursor(false);

	m_pos = {};

	m_movePow = { 0.0f,1.0f };

	CameraManager::Instance().EnableChangedCamera(false);
}

void HitCamera::PostUpdate()
{
	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpTarget.lock();

	if (_spTarget == nullptr) { return; }

	auto targetMat = _spTarget->GetMatrix();

	switch (m_shakeType)
	{
	case HitCamera::NoneShake:
		break;
	case HitCamera::UpDown:

		m_pos.y = randam.GetFloat(m_movePow.x, m_movePow.y) - m_movePow.y/2;

		break;
	case HitCamera::LefRi:

		m_pos.x = randam.GetFloat(m_movePow.x, m_movePow.y) - m_movePow.y / 2;
		break;
	case HitCamera::Intensely:

		m_pos.x = randam.GetFloat(m_movePow.x, m_movePow.y) - m_movePow.y / 2;
		m_pos.y = randam.GetFloat(m_movePow.x, m_movePow.y) - m_movePow.y / 2;

		break;
	default:
		break;
	}

	m_duration += KdFPSController::GetInstance().GetDeltaTime();

	if (m_duration >= m_limitDuration)
	{
		CameraManager::Instance().EnableChangedCamera(true);
		CameraManager::Instance().SetNextType(CameraManager::Tracking);
	}

	UpdateRotateByMouse();
	m_mRotation = GetRotationMatrix();

	auto trans= Math::Matrix::CreateTranslation(m_pos);

	m_mWorld = trans * m_mLocalPos * m_mRotation * targetMat;

	CameraBase::PostUpdate();

}

void HitCamera::Editor_ImGui()
{

}