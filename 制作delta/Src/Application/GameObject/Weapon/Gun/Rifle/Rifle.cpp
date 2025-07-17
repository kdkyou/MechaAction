#include "Rifle.h"

#include "../../../Character/CharacterBase.h"
#include "../Bullet/Bullet.h"

#include"../../../../Scene/SceneManager.h"

void Rifle::Init()
{

}


void Rifle::Update()
{
	//
	Trigger();

	auto parent = m_wpParent.lock();
	if (parent)
	{
		const KdModelWork::Node* _pNode = parent->GetModelWork().lock()->FindWorkNode(m_attachPath);
		int i = 0;
		if (_pNode)
		{
			m_parentAttachMat = _pNode->m_worldTransform;
		}

		m_mLocalRot = parent->GetMatrix();
	}

	m_mWorld = m_parentAttachMat * m_mLocalRot;
}

void Rifle::Trigger()
{

	CheckTrigger();

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

		KdEffekseerManager::GetInstance().Play("Thruster.efkefc",trans.Translation(), 1.0f, 3.0f,false);
	}
	

	std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>();
	bullet->SetModelData(m_bulletModelPath);
	bullet->SetBulletParam(m_aliveTime, m_damage, m_range, startPos, direct, m_speed, m_dampingInterval, m_dampingRate);
	bullet->Init();
	bullet->SetBulletTrail(m_bulletTrailPath, m_bulletTrailColor,m_bulletTrailWidth,m_bulletTrailLength);
	SceneManager::Instance().AddObject(bullet);

}

void Rifle::OnTrigger()
{
	m_trigger = true;
	m_durationFire = m_fireRate;
}


void Rifle::Editor_ImGui()
{

}


