#include "EmmisiveTerrain.h"

void EmmisiveTerrain::Init()
{
	m_name = "EmmisiveTerrain";
}

void EmmisiveTerrain::Update()
{
	auto delta = KdFPSController::GetInstance().GetDeltaTime();
	m_durationWait += delta;
	if (m_durationWait > m_waitTime)
	{
		m_durationWait = m_waitTime;
		if (!m_isEnableChange)
		{
			m_isEnableChange = true;

			if (m_soundPath != "")
			{
				auto& am = KdAudioManager::Instance();
				auto sound = am.Play(m_soundPath);
				sound->SetVolume(am.GetSEVolume());
				sound->SetPitch(0.7f);
			}
		}
	}

	if (!m_isEnableChange)
	{
		float t = (m_durationWait / m_waitTime) * m_waitChangeSpeed;

		Math::Vector3 zero = {};
		m_emmisive = Math::Vector3::Lerp(zero, m_firstEmmisive, t);

	}
	else
	{
		m_durationChange += delta;

		//float t = (m_durationChange / m_changeTime) * m_changeSpeed;

		//m_emmisive = Math::Vector3::Lerp(m_firstEmmisive, m_secondEmmisive, t);
		m_emmisive = { 10.0f,10.0f,10.0f };
	}

	
}


void EmmisiveTerrain::PostUpdate()
{
	if (m_spAnimator)
	{
		if (m_spModel) {
			m_spAnimator->AdvanceTime(m_spModel->WorkNodes(), m_animSpeed);
			m_spModel->CalcNodeMatrices();
		}
	}
}

void EmmisiveTerrain::DrawLit()
{
	float t = (m_durationChange / m_changeTime) * m_changeSpeed / 3.0f;

	if (m_isEnableChange)
	{
		KdShaderManager::Instance().m_StandardShader.SetLightningEnable(true);

		KdShaderManager::Instance().m_StandardShader.SetLightningLocalPos(m_maxPos, m_minPos);
		KdShaderManager::Instance().m_StandardShader.SetLightningColor(m_firstEmmisive, m_secondEmmisive);
		KdShaderManager::Instance().m_StandardShader.SetLightningProgress(t);

		DrawTerrain::DrawLit();

		KdShaderManager::Instance().m_StandardShader.SetLightningEnable(false);

	}
	else {

		DrawTerrain::DrawLit();

	}


}

void EmmisiveTerrain::Editor_ImGui()
{
	DrawTerrain::Editor_ImGui();

	if (ImGui::DragFloat((const char*)u8"待機時間", &m_waitTime, 0.01f, 0.0f, 100.0f))
	{
		m_durationWait = 0.0f;
	}
	ImGui::DragFloat((const char*)u8"一回目の遷移スピード", &m_waitChangeSpeed, 0.1f, 0.0f, 10.0f);
	ImGui::DragFloat((const char*)u8"二回目の遷移スピード", &m_changeSpeed, 0.1f, 0.0f, 10.0f);

	if (ImGui::DragFloat((const char*)u8"遷移時間", &m_changeTime, 0.01f, 0.0f, 100.0f))
	{
		m_durationChange = 0.0f;
	}

	ImGui::DragFloat3((const char*)u8"一つ目の色", &m_firstEmmisive.x, 0.1f, 0.0f, 100.0f);
	ImGui::DragFloat3((const char*)u8"二つ目の色", &m_secondEmmisive.x, 0.1f, 0.0f, 100.0f);

	ImGui::Checkbox((const char*)u8"ライトニング", &m_isLightning);

}
