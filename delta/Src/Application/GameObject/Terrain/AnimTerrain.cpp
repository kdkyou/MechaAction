#include "AnimTerrain.h"
#include "DrawTerrain.h"

void AnimTerrain::Init()
{
	m_name = "AnimTerrain";
}

void AnimTerrain::Update()
{
	auto angle = KdToRadians * m_rot;

	m_mWorld = Math::Matrix::CreateScale(m_scale) * Math::Matrix::CreateFromYawPitchRoll(angle) * Math::Matrix::CreateTranslation(m_pos);

}

void AnimTerrain::PostUpdate()
{
	if (m_spAnimator)
	{
		if (m_spModel) {
			m_spAnimator->AdvanceTime(m_spModel->WorkNodes(), m_animSpeed);
			m_spModel->CalcNodeMatrices();
		}
	}
}

void AnimTerrain::DrawLit()
{
	if (!m_spModel) return;

	for (auto& point : m_points)
	{
		KdShaderManager::Instance().WorkAmbientController().AddPointLight(point);
	}

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld,m_modelColor,m_emmisive);
}



void AnimTerrain::Editor_ImGui()
{
	DrawTerrain::Editor_ImGui();
}




