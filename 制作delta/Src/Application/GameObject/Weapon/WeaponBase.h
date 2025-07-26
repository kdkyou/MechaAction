#pragma once

class CharacterBase;
#include"../Character/CharacterBase.h"

class WeaponBase :public KdGameObject
{
public:
	WeaponBase(){}
	~WeaponBase() override{};

	void SetParent(std::shared_ptr<CharacterBase>_parent);
	virtual void SetModel(const std::string& path);
	virtual void SetModelData(const std::string& path);
	void SetAttachPath(const std::string& AttachPath);


	void DrawLit()override;
	void GenerateDepthMapFromLight()override;

	enum TriggerType
	{
		NoneTrigger,
		RightHand		= 1 << 1,
		LeftHand		= 1 << 2,
		RightShoulder	= 1 << 3,
		LeftShoulder	= 1 << 4,
		Max,
	};

	enum CollType
	{
		NoneType,
		Player = 1 << 1,
		Enemy = 1 << 2,
		Terrain = 1 << 3,
	};

	void SetAttackTrigger(TriggerType type) { m_AttackTrigger = type; }

	void CheckTrigger();

	void SetCollisionType(CollType type) { m_collisionType = type; }

	void SetDamage(int damage) { m_damage = damage; }

protected:

	
	std::weak_ptr<CharacterBase>    m_wpParent;

	std::shared_ptr<KdModelData>	m_spModelData = nullptr;
	std::shared_ptr<KdModelWork>	m_spModelWork = nullptr;

	std::shared_ptr<KdAnimator>		m_spAnimator = nullptr;
	bool							m_animChanged = false;

	Math::Vector3					m_pos = Math::Vector3::Zero;

	Math::Matrix					m_mLocalRot = Math::Matrix::Identity;
	Math::Matrix					m_mParentAttach = Math::Matrix::Identity;
	Math::Matrix					m_mParent = Math::Matrix::Identity;

	std::shared_ptr<KdModelWork::Node>	m_pNode;

	std::string						m_attachPath = "";

	UINT							m_nowTrigger = 0;
	TriggerType						m_AttackTrigger = NoneTrigger;

	// 当たり判定対象
	UINT							m_collisionType = 0;

	// 当たり判定回数
	int								m_attackNum = 1;
	int								m_maxAttackNum = 1;
	int								m_damage = 0;

	// サウンド関連
	bool							m_isOnece = true;	//重なり防止

};