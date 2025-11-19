#include "Effect.h"

void Effect::Update()
{
	if (m_durationAlive <= 0)
	{
		m_isExpired = true;
	}

	if (!m_isAlive)
	{

		m_durationAlive -= KdFPSController::GetInstance().GetDeltaTime();

	}

}

void Effect::DrawUnLit()
{
	if (m_type == eUnLit)
	{
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spPoly, m_mWorld);
	}

	else if (m_type == eUnAdd)
	{
		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spPoly, m_mWorld);

		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
	}

}

void Effect::DrawBright()
{
	if (m_type == eBright)
	{
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spPoly, m_mWorld);
	}

	else if (m_type == eBriAdd)
	{
		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spPoly, m_mWorld);

		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
	}
}

void Effect::ImGuiProcess()
{

}

void Effect::SetParam(const std::string& texPath, float alive, effectDraw type,bool isalive)
{
	if (texPath != "")
	{
		m_spPoly = std::make_shared<KdSquarePolygon>();

		m_spPoly->SetMaterial(texPath);
	}

	m_durationAlive = alive;

	m_type = type;

	m_isAlive = isalive;
}
