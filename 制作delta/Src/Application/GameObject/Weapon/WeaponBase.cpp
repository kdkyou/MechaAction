#include "WeaponBase.h"



void WeaponBase::SetParent(std::shared_ptr<CharacterBase> _parent)
{
	m_wpParent = _parent;
}

void WeaponBase::SetModel(const std::string& path) {
	if (!m_spModelWork)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData(path);
	}
}

void WeaponBase::SetModelData(const std::string& path)
{
	if (!m_spModelData)
	{
		m_spModelData = KdAssets::Instance().m_modeldatas.GetData(path);
	}
}

void WeaponBase::SetAttachPath(const std::string& attachPath)
{
	m_attachPath = attachPath;

}

void WeaponBase::DrawLit()
{
	if (m_spModelWork)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld);
	}

	if (m_spModelData)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelData, m_mWorld);
	}

}

void WeaponBase::GenerateDepthMapFromLight()
{
	if (m_spModelWork)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld);
	}

	if (m_spModelData)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelData, m_mWorld);
	}
}
