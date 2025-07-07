#include "Rifle.h"

#include "../../Character/CharacterBase.h"
#include "Bullet/Bullet.h"

#include"../../../Scene/SceneManager.h"

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

		m_mLocalMat = parent->GetMatrix();
	}

	m_mWorld = m_parentAttachMat * m_mLocalMat;
}

void Rifle::Trigger()
{
	if (KeyInput::GetInstance().GetKeyboardState().F) { m_trigger = true; }

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
	//	direct = Math::Vector3(0, 0, 1);
		direct.Normalize();

		KdEffekseerManager::GetInstance().Play("Thruster.efkefc",trans.Translation(), 1.0f, 3.0f,false);
	}
	Math::Color color = { 0.7f,0.3f,0.2f,1.0f };


	std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>();
	bullet->SetModelData(m_bulletModelPath);
	bullet->SetBulletParam(m_damage, m_range, startPos, direct, m_speed, m_dampingInterval, m_dampingRate);
	bullet->SetBulletTrail(m_bulletTrailPath, color, 0.4f,20);
	SceneManager::Instance().AddObject(bullet);

}

void Rifle::SetBulletsParam(const std::string& bulletModelPath, const std::string& bulletTrailPath, int damage,float speed, float range, float dampingInterval, float dampingRate)
{
	m_bulletModelPath = bulletModelPath;
	m_bulletTrailPath = bulletTrailPath;

	m_damage = damage;
	m_speed = speed;
	m_range = range;
	m_dampingInterval = dampingInterval;
	m_dampingRate = dampingRate;
}

void Rifle::Editor_ImGui()
{

}

void Rifle::SetGunsParam(const std::string gunModelPath, float fireRate, float coolTime, int maxNumOnes, int maxNum)
{
	if (gunModelPath != "")
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData(gunModelPath);
	}


	m_fireRate = fireRate;
	m_reloadTime = coolTime;
	m_maxNumofOnce = maxNumOnes;
	m_numOnce = maxNumOnes;
	m_maxNum = maxNum;
	m_num = maxNum;
}
