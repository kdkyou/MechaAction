#include "Charge.h"

#include"../Bullet/Bullet.h"
#include"../../../../Scene/SceneManager.h"

#include"../../../../main.h"

void Charge::Init()
{
	m_durationFire = 0.0f;
}

void Charge::Update()
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

		if ( parent->IsLeftShoulderAttack()) {
			if (m_animChanged == false)
			{

				m_animChanged = true; 
				m_spAnimator->SetAnimation(m_spModelWork->GetAnimation("Open"), 30.0f, false);
			}
		}
		else
		{
			if (m_animChanged == true)
			{

				m_animChanged = false; 
				m_spAnimator->SetAnimation(m_spModelWork->GetAnimation("Close"), 30.0f, false);
			}

		}
	}

	m_mWorld = m_parentAttachMat * m_mLocalRot;
}

void Charge::Editor_ImGui()
{
}

void Charge::Trigger()
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

	Application::Instance().m_log.AddLog("RightShoulderNowTrigger:%d\n", m_nowTrigger);
	// 発射可能か
	if (!m_trigger && m_durationFire<=0) { return; }


	m_durationFire += m_fireRateAccel * KdFPSController::GetInstance().GetDeltaTime();

	Application::Instance().m_log.AddLog("durarionFire:%.3f\n", m_durationFire);
	Application::Instance().m_log.AddLog("fireRate:%.1f\n", m_fireRate);


	if ((m_nowTrigger ^ m_AttackTrigger)!= 0)
	{
		if (m_durationFire < m_fireRate)
		{
			Shot();
			
		}
		else
		{
			ShotCharge();
			
		}
	}
	
	if (m_numOnce <= 0)
	{
		m_isReload = true;
		m_trigger = false;
	}
	

}

void Charge::Shot()
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

		KdEffekseerManager::GetInstance().Play("Thruster.efkefc", trans.Translation(), 1.0f, 3.0f, false);
	}


	std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>();
	bullet->SetModelData(m_bulletModelPath);
	bullet->SetBulletParam(m_aliveTime, m_damage, m_range, startPos, direct, m_speed, m_dampingInterval, m_dampingRate);
	bullet->Init();

	float width = m_durationFire / m_fireRate;

	bullet->SetBulletTrail(m_bulletTrailPath, m_bulletTrailColor, m_bulletTrailWidth * width, m_bulletTrailLength);
	SceneManager::Instance().AddObject(bullet);

	m_durationFire = 0;
	m_num -= 1;
	m_numOnce -= 1;
}

void Charge::ShotCharge()
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

		KdEffekseerManager::GetInstance().Play("Thruster.efkefc", trans.Translation(), 1.0f, 3.0f, false);
	}
	// 残弾で倍率上昇
	int chargeNum = m_numOnce;

	float damage = m_damage + float(chargeNum / m_maxNumofOnce) * m_damage;

	std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>();
	bullet->SetModelData(m_bulletModelPath);
	bullet->SetBulletParam(m_aliveTime, damage, m_range, startPos, direct, m_speed, m_dampingInterval, m_dampingRate);
	bullet->Init();
	bullet->SetBulletTrail(m_bulletTrailPath, m_bulletTrailColor, m_bulletTrailWidth, m_bulletTrailLength);
	SceneManager::Instance().AddObject(bullet);

	m_durationFire = 0.0f;
	m_num -= m_numOnce;
	m_numOnce = 0;
}

void Charge::OnTrigger()
{
	m_trigger = true;
}
