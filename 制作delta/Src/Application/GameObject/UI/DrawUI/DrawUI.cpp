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
	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, m_pos.x, m_pos.y,&m_rect);

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
}
