#include "GuageUI.h"

void GuageUI::Init()
{
	m_name = "GuageUI";

	SetTexture("Asset/Textures/UI/Guage.png");

	m_drawWi = m_spTex->GetInfo().Width;
	m_drawHe = m_spTex->GetInfo().Height;

	m_rectWi = m_drawWi;
	m_rectHe = m_drawHe;

	m_rect = { 0, 0, m_rectWi, m_rectHe };
}

void GuageUI::Update()
{

}

void GuageUI::DrawSprite()
{
	DrawGuage();
}

void GuageUI::Editor_ImGui()
{
	UIBase::Editor_ImGui();

	ImGui::DragInt("Current", &m_current, 1, 0, m_max);
	ImGui::DragInt("Max", &m_max, 1, 1, 999);
	ImGui::Checkbox("Horizontal", &m_isHorizontal);
	ImGui::Checkbox("Reverse", &m_isReverse);

}

void GuageUI::Deserialize(const nlohmann::json& jsonObj)
{
	UIBase::Deserialize(jsonObj);

}

void GuageUI::Serialize(nlohmann::json& outJson) const
{
	UIBase::Serialize(outJson);
}

void GuageUI::DrawGuage()
{
	if (m_max <= 0) { return; }

	float ratio = std::clamp((float)m_current / (float)m_max, 0.0f, 1.0f);

	Math::Rectangle rect = m_rect;

	if (m_isHorizontal)
	{
		int cutWidth = static_cast<int>(m_rectWi * ratio);

		if (m_isReverse)
		{
			rect.x = m_rectWi - cutWidth; //右端から
		}

		rect.width = cutWidth;

	}
	else {
		int cutHeight = static_cast<int>(m_rectHe * ratio);

		if (m_isReverse)
		{
			rect.y = m_rectHe - cutHeight;
		}

		rect.height = cutHeight;
	}

	KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, m_pos.x, m_pos.y, m_drawWi, m_drawHe, &rect);
}


