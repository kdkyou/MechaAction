#include "Rifle.h"

void Rifle::Init()
{

}


void Rifle::Update()
{
	//
	Trigger();

	const std::shared_ptr<KdGameObject> parent = m_wpParent.lock();
	if(parent)
	{
		m_mLocalMat = parent->GetMatrix();
	}

	m_mWorld = m_parentAttachMat * m_mLocalMat;
}

void Rifle::Trigger()
{
	if (!m_trigger) { return; }

	if (KeyInput::GetInstance().GetKeyboardState().F)
	{
		if (m_durationReload >= 1)
		{
		}
		m_durationFire += KdFPSController::GetInstance().GetDeltaTime();

		if (m_durationFire >= m_fireRate)
		{
			m_durationFire = 0;
			Shot();
			m_num -= 1;
			if (m_num <= 0)
			{
				m_num = m_maxNumofOnce;

			}
		}
	}
}

void Rifle::Shot()
{
	int i = 0;

}

void Rifle::SetGunsParam(const std::string gunModelPath, int damage, int fireRate, int coolTime, int maxNumOnes, int maxNum)
{
	if (gunModelPath != "")
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData(gunModelPath);
	}

	m_damage = damage;
	m_fireRate = fireRate;
	m_reloadTime = coolTime;
	m_maxNumofOnce = maxNumOnes;
	m_maxNum = maxNum;
}
