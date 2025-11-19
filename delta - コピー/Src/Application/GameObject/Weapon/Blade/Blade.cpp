#include "Blade.h"

void Blade::SetModel(const std::string& path)
{
	if (!m_spModelWork)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData(path);
	}

	if (!m_spAnimator)
	{
		m_spAnimator = std::make_shared<KdAnimator>();
		m_spAnimator->SetAnimation(m_spModelWork->GetAnimation("Close"), 10.0f,false);
	}
}

void Blade::Init()
{
	
	Math::Vector3 pos = { 0.0f,5.0f,3.0f };
	SetPos(pos);
}

void Blade::Update()
{
	//親(プレイヤー)の行列を取得
	const std::shared_ptr<const CharacterBase> _spParent = m_wpParent.lock();
	Math::Matrix _rotMat = Math::Matrix::Identity;
	Math::Matrix _parentMat = Math::Matrix::Identity;

	if (_spParent)
	{
		const KdModelWork::Node* _pNode = _spParent->GetModelWork().lock()->FindWorkNode(m_attachPath);
		if (_pNode)
		{
			m_mParentAttach = _pNode->m_worldTransform;
		}

		_parentMat = _spParent->GetMatrix();

	}

	
	m_mWorld =  m_mParentAttach * _parentMat;


	WeaponBase::Update();

	m_spAnimator->AdvanceTime(m_spModelWork->WorkNodes());

}

void Blade::DrawBright()
{
	Math::Color colr = { 1,1,1 };


}
