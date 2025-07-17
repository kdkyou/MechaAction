#include"GunBase.h"

void GunBase::SetGunsParam(const std::string& gunModelPath, float fireRate, float coolTime, float burst, int burstNum, int maxNumOnes, int maxNum)
{
	if (gunModelPath != "")
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData(gunModelPath);
	}


	m_fireRate = fireRate;
	m_durationFire = fireRate;
	m_reloadTime = coolTime;
	m_burst = burst;
	m_durationBurst = burst;
	m_numBurst = burstNum;
	m_maxNumofOnce = maxNumOnes;
	m_numOnce = maxNumOnes;
	m_maxNum = maxNum;
	m_num = maxNum;
}

void GunBase::MakeAnimator(const std::string& animation,float compSpeed,bool loop)
{
	if (m_spModelWork)
	{
		m_spAnimator = std::make_shared<KdAnimator>();
		m_spAnimator->SetAnimation(m_spModelWork->GetAnimation(animation),compSpeed,loop);

	}
}

void GunBase::SetBulletsParam(const std::string& bulletModelPath,float aliveTime, int damage, float speed, float range, float dampingInterval, float dampingRate)
{
	m_bulletModelPath = bulletModelPath;
	m_aliveTime = aliveTime;
	m_damage = damage;
	m_speed = speed;
	m_range = range;
	m_dampingInterval = dampingInterval;
	m_dampingRate = dampingRate;
}

void GunBase::SetBulletsTrailParam(const std::string& bulletTrailPath, const Math::Color& color, float width, UINT length)
{
	m_bulletTrailPath = bulletTrailPath;
	m_bulletTrailColor = color;
	m_bulletTrailWidth = width;
	m_bulletTrailLength = length;
}

void GunBase::SetBulletChaisingData(int bulletRotateDeg, float bulletLockAngle, float bulletLostTime, float bulletTrackingDistance)
{
	m_bulletRotateDeg = bulletRotateDeg;
	m_bulletLockAngle = bulletLockAngle;
	m_bulletLostTime = bulletLostTime;
	m_bulletTrackingDistance = bulletTrackingDistance;
}

bool GunBase::SetNodeMats(const std::string& nodeName)
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
