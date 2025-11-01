#include "MapUI.h"

#include "../../../Scene/SceneManager.h"

void MapUI::Init()
{
	m_name = "MapUI";

	m_mapTex = SceneManager::Instance().GetRenderTargetTexture();
	m_mapRect = { 0,0,1280,720 };
}

void MapUI::Update()
{
	m_mapTex = SceneManager::Instance().GetRenderTargetTexture();
}

void MapUI::DrawSprite()
{
	if (m_mapTex)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(m_mapTex, (int)m_pos.x, (int)m_pos.y, m_drawWi, m_drawHe, &m_mapRect);
	}

	if (m_spTex)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, (int)m_pos.x, (int)m_pos.y, m_drawWi, m_drawHe, &m_rect,&m_color,m_pivot);
	}
}

void MapUI::Editor_ImGui()
{
	UIBase::Editor_ImGui();
}
