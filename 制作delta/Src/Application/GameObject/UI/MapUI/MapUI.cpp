#include "MapUI.h"

#include "../../../Scene/SceneManager.h"

void MapUI::Init()
{
	m_name = "MapUI";

	m_spTex = SceneManager::Instance().GetRenderTargetTexture();
	m_rectX = 0;
	m_rectY = 0;
	m_rectWi = m_spTex->GetWidth();
	m_rectHe = m_spTex->GetHeight();
	m_drawWi = 200;
	m_drawHe = 200;
}

void MapUI::Update()
{
	m_spTex = SceneManager::Instance().GetRenderTargetTexture();
}

void MapUI::Editor_ImGui()
{
	UIBase::Editor_ImGui();
}
