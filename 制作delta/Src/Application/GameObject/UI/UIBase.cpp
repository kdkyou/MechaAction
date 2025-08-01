#include "UIBase.h"

void UIBase::Init()
{
}

void UIBase::DrawSprite()
{
	if (m_spTex)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex,m_pos.x,m_pos.y);
	}
}

void UIBase::Editor_ImGui()
{
	KdGameObject::Editor_ImGui();
}

void UIBase::SetTexture(const std::string& path)
{
	if (!m_spTex)
	{
		m_spTex = KdAssets::Instance().m_textures.GetData(path);
	}
}