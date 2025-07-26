#include "Polygon.h"

void Polygon::Update()
{
	if (m_durationAlive <= 0)
	{
		m_isExpired = true;
	}

	if (!m_isAlive)
	{

		m_durationAlive -= KdFPSController::GetInstance().GetDeltaTime();

	}

	Math::Vector3 rad = KdToRadians * m_rot;

	m_mWorld = Math::Matrix::CreateScale(m_scale) * Math::Matrix::CreateFromYawPitchRoll(rad) * Math::Matrix::CreateTranslation(m_pos);
	
}

void Polygon::DrawUnLit()
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

void Polygon::DrawBright()
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

void Polygon::Editor_ImGui()
{
	KdGameObject::Editor_ImGui();

	if (ImGui::Button("LoadModel"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			SetTexture(filepath);
		}
	}
}

void Polygon::SetParam(const std::string& texPath, float alive, effectDraw type, bool isalive)
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
