#include "PolygonEffect.h"

void PolygonEffect::Init()
{
	m_spPoly = std::make_shared<KdSquarePolygon>();


	m_name = "PolygonEffect";
}

void PolygonEffect::Update()
{

	if (!m_isAlive)
	{
		if (m_durationAlive <= 0)
		{
			m_isExpired = true;
		}

		m_durationAlive -= KdFPSController::GetInstance().GetDeltaTime();

	}
}

void PolygonEffect::DrawUnLit()
{
	if (!m_spPoly) { return; }

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

void PolygonEffect::DrawBright()
{
	if (!m_spPoly) { return; }

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

void PolygonEffect::Editor_ImGui()
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

	ImGui::Checkbox((const char*)u8"描画し続けるか", &m_isAlive);

	if (ImGui::BeginListBox("DrawType"))
	{
		if (ImGui::Selectable("UnLit")) {
			m_type = effectDraw::eUnLit;
		}
		if (ImGui::Selectable("UnAdd")) {
			m_type = effectDraw::eUnAdd;
		}
		if (ImGui::Selectable("Bright")) {
			m_type = effectDraw::eBright;
		}
		if (ImGui::Selectable("BriAdd")) {
			m_type = effectDraw::eBriAdd;
		}
		ImGui::EndListBox();
	}

}

void PolygonEffect::SetParam(const std::string& texPath, float alive, effectDraw type, bool isalive, const Math::Matrix& occurMat)
{

	if (texPath != "")
	{
		if (!m_spPoly)
		{
			m_spPoly = std::make_shared<KdSquarePolygon>();
		}

		m_spPoly->SetMaterial(texPath);
	}

	m_durationAlive = alive;

	m_type = type;

	m_isAlive = isalive;

	m_mWorld = occurMat;
}
