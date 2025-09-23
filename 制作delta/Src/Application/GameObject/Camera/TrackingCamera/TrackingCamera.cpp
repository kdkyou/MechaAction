
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

	m_localPos = { 3, 14.5f, -22.0f };
	m_basePos = { 3.0f, 14.5f, -28.0f };

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);


	if (m_wpTarget.expired() == false)
	{
		m_mWorld = m_mLocalPos * m_wpTarget.lock()->GetMatrix();

		m_pos = m_wpTarget.lock()->GetMatrix().Translation();
		
		m_DegAng;
	}


	m_name = "Tracking";
		CameraManager::Instance().EnableChangedCamera(false);

	ShowCursor(false);

}

void TrackingCamera::PostUpdate()
{
	ShowCursor(false);


	Math::Vector3 vMove=Math::Vector3::Zero;

	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpTarget.lock();
	
	if (_spTarget == nullptr) { return; }

	Math::Vector3 targetPos = _spTarget->GetMatrix().Translation();

	m_unEnableChangeTime -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_unEnableChangeTime < 0.0f)
	{
		m_unEnableChangeTime = 0;
		CameraManager::Instance().EnableChangedCamera(true);
	}
	
	float margin = 0.25f;

	// 画面範囲外判定
	CheckScreenPull();
	
	Math::Vector3 pos = {};
	if (m_isPull)
	{
		
		pos = m_localPos;
		m_mLocalPos = Math::Matrix::CreateTranslation(pos);

	m_pos = Math::Vector3::Lerp(
		m_pos,
		targetPos,
		margin
		);

	}
	else {
		//m_speed = 5.0f;
		pos = m_localPos;
	m_mLocalPos = Math::Matrix::CreateTranslation(pos);

	m_pos = Math::Vector3::Lerp(
		m_pos,
		targetPos,
		m_speed*KdFPSController::GetInstance().GetDeltaTime()			//進行速度*デルタタイム
		);
	}
		// 補正処理
	//	m_speed = 10.0f;
	//	m_mLocalPos = Math::Matrix::CreateTranslation(m_basePos);
	//}
	//else {
	//	m_speed = 5.0f;
	//	m_mLocalPos = Math::Matrix::CreateTranslation(m_localPos);
	//}

	/*auto targetVec = CameraManager::Instance().GetLocalDirectionTo(targetPos);
	if (targetVec.y < 0 || targetVec.x< -0.9f || targetVec.x > 1.0f) {
		m_speed = 10.0f;
		m_mLocalPos = Math::Matrix::CreateTranslation(m_basePos);
	}
	else {
		m_speed = 5.0f;
		m_mLocalPos = Math::Matrix::CreateTranslation(m_localPos);
	}*/

	UpdateRotateByMouse();
	m_mRotation = GetRotationMatrix();

	m_mWorld = m_mLocalPos *m_mRotation * Math::Matrix::CreateTranslation(m_pos);

	CameraBase::PostUpdate(); 
}

void TrackingCamera::Editor_ImGui()
{
	ImGui::SliderFloat("LerpSpeed", &m_speed, 0.0f, 100.0f);

	ImGui::Text("pos x:%.2f,y:%.2f,z:%.2f",m_mLocalPos.Translation().x, m_mLocalPos.Translation().y, m_mLocalPos.Translation().z);

}

void TrackingCamera::CheckScreenPull()
{
	auto _spTarget = m_wpTarget.lock();
	auto targetPos = _spTarget->GetPos();

	auto viewProjMat = m_spCamera->GetCameraViewMatrix() * m_spCamera->GetProjMatrix();
	//Math::Vector4 clipPos = DirectX::XMVector4Transform(targetPos,viewProjMat);
	//clipPos /= clipPos.w; // NDC (-1~1)
	Math::Vector3 clipPos = {};
	m_spCamera->ConvertWorldToScreenDetail(targetPos, clipPos);

	if (clipPos.x > 450.0f || clipPos.x < -450.0f || clipPos.y < -350.0f)
	{
		m_isPull = true;
	}

	if (clipPos.x <300.0f && clipPos.x > -300.0f && clipPos.y > -300.0f)
	{
		m_isPull = false;
	}
	
	
	Application::Instance().m_log.AddLog("ClipPos X:%.2f,Y:%.2f\n", clipPos.x, clipPos.y);

}
