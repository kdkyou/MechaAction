#include "Rifle.h"

#include "../../../Character/CharacterBase.h"
#include "../Bullet/Bullet.h"

#include"../../../../Scene/SceneManager.h"

#include "../../../Camera/CameraManager.h"

void Rifle::Init()
{
	m_rot.x = 15.0f;
	m_rot.y = 20.0f;

	m_muzzleFlashPath = "Asset/Textures/GameObject/MuzzleCross.png";

	m_name = "Rifle";

}


void Rifle::PreUpdate()
{
	GunBase::PreUpdate();
}

void Rifle::Update()
{
	//
	Trigger();

	auto parent = m_wpParent.lock();
	if (parent)
	{
		const KdModelWork::Node* _pNode = parent->GetModelWork().lock()->FindWorkNode(m_attachPath);
		if (_pNode)
		{
			m_mParentAttach = _pNode->m_worldTransform;
		}

		m_mParent = parent->GetMatrix();
	}

	m_mWorld = m_mLocalRot * m_mParentAttach * m_mParent;

	GunBase::Update();
}

void Rifle::Trigger()
{

	CheckTrigger();

	if (m_num <= 0) { return; }

	if (m_nowTrigger & m_AttackTrigger) { OnTrigger(); }
	else { m_trigger = false; }

	if (m_isReload) 
	{
		m_durationReload += m_reloadAccel * KdFPSController::GetInstance().GetDeltaTime();

		// リロード完了したら
		if (m_durationReload >= 1.0f)
		{
			// 一回の残弾を最大値へ
			m_numOnce = m_maxNumofOnce;
			// リロード時間を0に
			m_durationReload = 0.0f;
			m_isReload = false;
		}

		return;
	}

	// 発射可能か
	if (!m_trigger) { return; }

	// 砲身調整
	auto  angle =Math::Vector2{ m_rot.x,m_rot.y };
	auto parent = m_wpParent.lock();
	if (parent != nullptr)
	{
		if (parent->GetTag() == tEnemy)
		{
		
			auto target = parent->GetCharacterTarget().lock();
			if (target != nullptr)
			{
				auto current = target->GetCorrectionMatrix().Translation();
				Math::Vector3 diff = target->GetMatrix().Translation() + current - m_mWorld.Translation();
				m_mLocalRot = RotateWeaponDirect(angle, diff,m_mLocalRot);
			}
		}
	}

	m_durationFire +=m_fireRateAccel * KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationFire >= m_fireRate)
	{
		m_durationFire = 0;
		Shot();
		m_num -= 1;
		m_numOnce -= 1;
		if (m_numOnce <= 0)
		{
			m_isReload = true;
			m_trigger = false;
		}
	}

}

void Rifle::Shot()
{
	const KdModelWork::Node* pNode = m_spModelWork->FindWorkNode("SP");
	Math::Vector3 startPos = Math::Vector3::Zero;
	Math::Vector3 direct = Math::Vector3::Zero;
	if (pNode)
	{
		auto trans = pNode->m_worldTransform * m_mWorld;
		startPos = trans.Translation();

		direct = trans.Backward();
		direct.Normalize();

		Flash(trans);
	}

	
	KdAudioManager::Instance().Play(m_shotSoundPath)->SetVolume(0.1f);

	std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>();
	bullet->SetModelData(m_bulletModelPath);
	bullet->SetBulletParam(m_aliveTime, m_damage, m_range, startPos, direct, m_speed, m_dampingInterval, m_dampingRate);
	bullet->SetBulletTrail(m_bulletTrailPath, m_bulletTrailColor,m_bulletTrailWidth,m_bulletTrailLength);
	bullet->Init();
	bullet->SetTag(m_tag);


	SceneManager::Instance().AddObject(bullet);

}

void Rifle::OnTrigger()
{
	if (m_trigger == false)
	{

		m_trigger = true;
		m_durationFire = m_fireRate;
	}
}


void Rifle::Editor_ImGui()
{

}


