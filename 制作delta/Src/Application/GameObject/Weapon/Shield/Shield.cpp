#include "Shield.h"

void Shield::SetModel(const std::string& path)
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

void Shield::Init()
{
	
	Math::Vector3 pos = { 0.0f,5.0f,3.0f };
	SetPos(pos);
}

void Shield::Update()
{
	//親(プレイヤー)の行列を取得
	const std::shared_ptr<const CharacterBase> _spParent = m_wpParent.lock();
	Math::Matrix _rotMat = Math::Matrix::Identity;
	Math::Matrix _parentMat = Math::Matrix::Identity;

	if (_spParent)
	{
		const KdModelWork::Node* _pNode = _spParent->GetModelWork().lock()->FindWorkNode("LeftWeapon");
		if (_pNode)
		{
			m_parentAttachMat = _pNode->m_worldTransform;
		}

		_parentMat = _spParent->GetMatrix();

	}

	if (_spParent)
	{
		// 
		if (_spParent->IsLeftAttack()) {
			if (m_animChanged == false)
			{

				m_animChanged = true; m_spAnimator->SetAnimation(m_spModelWork->GetAnimation("Open"), 30.0f, false);
			}
		}
		else
		{
			if (m_animChanged == true)
			{

				m_animChanged = false; m_spAnimator->SetAnimation(m_spModelWork->GetAnimation("Close"), 30.0f, false);
			}

		}

	}

	m_mWorld = m_parentAttachMat * _parentMat;


	WeaponBase::Update();


}

void Shield::PostUpdate()
{
	m_spAnimator->AdvanceTime(m_spModelWork->WorkNodes(), 30.0f);
}

void Shield::DrawBright()
{
	Math::Color colr = { 1,1,1 };


}
