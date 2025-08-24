#include "BlinkUI.h"

void BlinkUI::Init()
{
	m_name = "BlinkUI";
}

void BlinkUI::Update()
{
	m_durationTime += m_plamai * KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationTime > m_time || m_durationTime < 0.0f)
	{
		m_plamai *= -1;
	}

  m_alpha =	std::clamp(m_durationTime / m_time, m_WorstAlpha, 1.0f);

}

void BlinkUI::Editor_ImGui()
{
	UIBase::Editor_ImGui();
}

void BlinkUI::DrawSprite()
{
	if (m_spTex)
	{
		m_rect = { m_rectX,m_rectY,m_rectWi,m_rectHe };

		m_color.w = m_alpha;
		 
		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

		KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, m_pos.x, m_pos.y, m_drawWi, m_drawHe, &m_rect, &m_color, m_pivot);

		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
	}
}
