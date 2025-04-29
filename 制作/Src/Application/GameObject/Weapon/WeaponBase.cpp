#include "WeaponBase.h"


void WeaponBase::Update()
{

}


void WeaponBase::DrawLit()
{
	if (!m_spModelWork)return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld);
}

void WeaponBase::GenerateDepthMapFromLight()
{

	if (!m_spModelWork)return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld);
}
