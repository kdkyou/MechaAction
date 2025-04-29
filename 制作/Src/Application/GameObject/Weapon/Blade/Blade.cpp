#include "Blade.h"
#include"../../Character/Character.h"

void Blade::Init()
{
	if (!m_spModelWork)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData("Asset/Models/Blade/Blade.gltf");
	//	m_spModelWork->SetModelData("Asset/Models/Leg/Leg.gltf");

	}

	if (!m_spAnimator)
	{
		m_spAnimator = std::make_shared<KdAnimator>();
		m_spAnimator->SetAnimation(m_spModelWork->GetAnimation("Normal"));
	}
	Math::Vector3 pos = { 0.0f,5.0f,0.0f };
	SetPos(pos);
}

void Blade::Update()
{
	//親(プレイヤー)の行列を取得
	const std::shared_ptr<const Character> _spParent = m_wpParent.lock();
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

	
	m_mWorld =  m_parentAttachMat * _parentMat;


	WeaponBase::Update();

	m_spAnimator->AdvanceTime(m_spModelWork->WorkNodes());

}

void Blade::SetParent(std::weak_ptr<Character> _parent)
{
	m_wpParent = _parent;
}

void Blade::DrawBright()
{
	Math::Color colr = { 1,1,1 };

//	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld,colr , Math::Vector3::One);
}
