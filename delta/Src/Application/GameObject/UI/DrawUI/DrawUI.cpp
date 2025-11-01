#include "DrawUI.h"

void DrawUI::Init()
{
	m_name = "DrawUI";
}

void DrawUI::Editor_ImGui()
{
	UIBase::Editor_ImGui();

	ImGui::Checkbox((const char*)u8"加算", &m_isAdd);
}

void DrawUI::Update()
{
	if (m_isTimeRimit)
	{
		m_aliveTime -= KdFPSController::GetInstance().GetDeltaTime();
		if (m_aliveTime < 0.0f) {
			m_isExpired = true;
		}
	}
}

void DrawUI::DrawSprite()
{
	if (m_spTex)
	{
		m_rect = { m_rectX,m_rectY,m_rectWi,m_rectHe };

		if (m_isAdd) {
			KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);
			KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, (int)m_pos.x, (int)m_pos.y, m_drawWi, m_drawHe, &m_rect, &m_color, m_pivot);
			KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
		}
		else {
			KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, (int)m_pos.x, (int)m_pos.y, m_drawWi, m_drawHe, &m_rect, &m_color, m_pivot);
		}
	}
}
