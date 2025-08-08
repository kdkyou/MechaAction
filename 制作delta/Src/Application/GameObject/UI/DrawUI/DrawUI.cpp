#include "DrawUI.h"

void DrawUI::Init()
{
	m_name = "DrawUI";
}

void DrawUI::Editor_ImGui()
{
	UIBase::Editor_ImGui();

}

void DrawUI::DrawSprite()
{
	if (m_spTex)
	{
		m_rect = { m_rectX,m_rectY,m_rectWi,m_rectHe };

		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

		KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, m_pos.x, m_pos.y, m_drawWi, m_drawHe, &m_rect,&m_color,m_pivot);

		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
	}
}
