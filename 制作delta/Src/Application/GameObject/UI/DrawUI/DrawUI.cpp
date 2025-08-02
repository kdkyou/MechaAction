#include "DrawUI.h"

void DrawUI::Init()
{

}

void DrawUI::DrawSprite()
{
	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, 0, 0);

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
}
