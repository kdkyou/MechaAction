#include "MoveTerrain.h"
#include "DrawTerrain.h"

void MoveTerrain::Init()
{
	m_name = "MoveTerrain";
}

void MoveTerrain::Update()
{
	auto angle = KdToRadians * m_rot;
	
	m_durationAnimWait += KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationAnimWait > m_animWaitTime) 
	{
		m_durationAnimWait = m_animWaitTime;

		if (m_spAnimator)
		{
			if (m_spAnimator->IsAnimationEnd())
			{
				m_isMovable = true;
			}
		}
		else {
			m_isMovable = true;
		}
	}

	if (m_isMovable) {
		m_durationWait += KdFPSController::GetInstance().GetDeltaTime();
		if (m_durationWait > m_waitTime)
		{
			m_durationWait = m_waitTime;
			if (!m_isMove)
			{
				m_isMove = true;

				if (m_soundPath != "")
				{
					auto& am = KdAudioManager::Instance();
					am.Play(m_soundPath)->SetVolume(am.GetSEVolume());
				}
			}
		}
	}

	if (m_isMove) {

		m_durationMove += KdFPSController::GetInstance().GetDeltaTime();

		if (m_durationMove > m_moveTime) {
			m_durationMove = m_moveTime;
		}

		auto t = m_durationMove / m_moveTime * m_moveSpeed;

		if (t > 1.0f) {
			t = 1.0f;
		}

		m_pos = Math::Vector3::Lerp(m_startPos, m_endPos, t);

	}

	m_mWorld = Math::Matrix::CreateScale(m_scale) * Math::Matrix::CreateFromYawPitchRoll(angle) * Math::Matrix::CreateTranslation(m_pos);

	for (auto& point : m_points) {
		KdShaderManager::Instance().WorkAmbientController().AddPointLight(point);
	}

}

void MoveTerrain::PostUpdate()
{
	if (m_durationAnimWait >= m_animWaitTime)
	{
		if (m_spAnimator)
		{
			if (m_spModel) {
				m_spAnimator->AdvanceTime(m_spModel->WorkNodes(), m_animSpeed);
				m_spModel->CalcNodeMatrices();
			}
		}
	}
	else
	{
		if (m_spAnimator) {
			if (m_spAnimator->IsComp())
			{
				m_spAnimator->AdvanceTime(m_spModel->WorkNodes(), m_animSpeed);
				m_spModel->CalcNodeMatrices();
			}
		}
	}
}

void MoveTerrain::DrawLit()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld, m_modelColor, m_emmisive);
}

void MoveTerrain::Editor_ImGui()
{
	DrawTerrain::Editor_ImGui();

	if (ImGui::DragFloat((const char*)u8"アニメーション待機時間", &m_animWaitTime, 0.01f, 0.0f, 100.0f))
	{
		m_durationAnimWait = 0.0f;
	}
	if (ImGui::DragFloat((const char*)u8"移動待機時間", &m_waitTime, 0.01f, 0.0f, 100.0f))
	{
		m_durationWait = 0.0f;
	}
	ImGui::DragFloat((const char*)u8"スピード", &m_moveSpeed, 0.1f, 0.0f, 100.0f);
	ImGui::DragFloat3((const char*)u8"方向", &m_moveVec.x, 0.01f, 0.0f, 1.0f);



}

