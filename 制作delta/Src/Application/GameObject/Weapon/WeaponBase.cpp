#include "WeaponBase.h"

void WeaponBase::SetAttachModel(const std::string& modelPath, const std::string& attachPath)
{
	if (m_spModelWork == nullptr)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData(modelPath);
	}

	if (attachPath != "")
	{
		m_attachPath = attachPath;
		const KdModelWork::Node* _pNode = m_spModelWork->FindWorkNode(attachPath);
		if (_pNode)
		{
			m_parentAttachMat = _pNode->m_worldTransform;
		}
	}

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
