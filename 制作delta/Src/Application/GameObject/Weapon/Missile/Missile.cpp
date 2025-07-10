#include"Missile.h"

#include"../Rifle/Bullet/Bullet.h"

#include"../../../Scene/SceneManager.h"

void Missile::Init()
{
}

void Missile::Update()
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

void Missile::Trigger()
{

	CheckTrigger();

	if (m_nowTrigger & m_AttackTrigger) { m_trigger = true; }
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

	static int burstCount = 0;

	// 発射レート
	m_durationFire += m_fireRateAccel * KdFPSController::GetInstance().GetDeltaTime();

	//時間になったか
	if (m_durationFire >= m_fireRate)
	{
		//　複数を連続して発射する
		m_durationBurst += m_burstAccel * KdFPSController::GetInstance().GetDeltaTime();

		//時間になったか
		if (m_durationBurst >= m_burst)
		{
			Shot();
			m_num -= 1;
			m_numOnce -= 1;

			m_durationBurst = 0.0f;

			burstCount++;

			if (burstCount > m_numBurst)
			{
				m_durationBurst = 0.0f;
			}
			
		}

		if (m_numOnce <= 0)
		{
			m_isReload = true;
		}
	}

}

void Missile::Shot()
{

	Math::Vector3 startPos = Math::Vector3::Zero;
	Math::Vector3 direct = Math::Vector3::Zero;
	
	{
		auto trans = m_nodeMats[m_numShot] * m_mWorld;
		startPos = trans.Translation();

		direct = trans.Backward();
		//	direct = Math::Vector3(0, 0, 1);
		direct.Normalize();

		KdEffekseerManager::GetInstance().Play("Thruster.efkefc", trans.Translation(), 1.0f, 3.0f, false);
	}
	Math::Color color = { 0.7f,0.3f,0.2f,1.0f };


	std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>();
	bullet->SetModelData(m_bulletModelPath);
	bullet->SetBulletParam(m_damage, m_range, startPos, direct, m_speed, m_dampingInterval, m_dampingRate);
	bullet->SetBulletTrail(m_bulletTrailPath, color, 0.4f, 20);
	SceneManager::Instance().AddObject(bullet);

	m_numShot++;
	if (m_numShot >= m_nodeMats.size())
	{
		m_numShot = 0;
	}

}

void Missile::Editor_ImGui()
{
}

bool Missile::SetNodeMats(const std::string& nodeName)
{
	const KdModelWork::Node* pNode = m_spModelWork->FindWorkNode(nodeName);

	if (pNode)
	{
		Math::Matrix mat = pNode->m_worldTransform;
		m_nodeMats.push_back(mat);
		return true;
	}

	return false;

}

void Missile::SetBulletsParam(const std::string& bulletModelPath, const std::string& bulletTrailPath, int damage, float speed, float range, float dampingInterval, float dampingRate)
{
	m_bulletModelPath = bulletModelPath;
	m_bulletTrailPath = bulletTrailPath;

	m_damage = damage;
	m_speed = speed;
	m_range = range;
	m_dampingInterval = dampingInterval;
	m_dampingRate = dampingRate;
}

void Missile::SetGunsParam(const std::string gunModelPath, float fireRate, float coolTime, float burst, int burstNum, int maxNumOnes, int maxNum)
{
	if (gunModelPath != "")
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData(gunModelPath);
	}


	m_fireRate = fireRate;
	m_reloadTime = coolTime;
	m_burst = burst;
	m_numBurst = burstNum;
	m_maxNumofOnce = maxNumOnes;
	m_numOnce = maxNumOnes;
	m_maxNum = maxNum;
	m_num = maxNum;
}