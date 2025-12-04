#include"Missile.h"

#include"../Bullet/Bullet.h"

#include"../../../../Scene/SceneManager.h"
#include"../../../Camera/CameraManager.h"

void Missile::Init()
{
	m_durationFire = 0.0f;

	m_name = "Missile";
	m_shotSoundPath = "Asset/Sounds/SE/Weapon/MissileShot.wav";
	m_muzzleFlashPath = "Asset/Effects/MuzzleFlash/MuzzleBurn.png";

}

void Missile::Update()
{
	//
	Trigger();
	
	if (m_nowTrigger & m_AttackTrigger) { m_trigger = true; }

	auto parent = m_wpParent.lock();

	
	if (parent)
	{
		if (m_attachPath != "")
		{
			const KdModelWork::Node* _pNode = parent->GetModelWork().lock()->FindWorkNode(m_attachPath);
			if (_pNode)
			{
				m_mParentAttach = _pNode->m_worldTransform;
			}
		}

		m_mParent = parent->GetMatrix();
	}

	m_mWorld = m_mParentAttach * m_mParent;

	GunBase::Update();
}

void Missile::PostUpdate()
{
	GunBase::PostUpdate();
}

void Missile::Trigger()
{
	CheckTrigger();
	
	if (m_num <= 0 && m_numOnce <= 0) { return; }

	if (m_nowTrigger & m_AttackTrigger) { OnTrigger(); }

	if (m_trigger == true) {
		if (m_animChanged == false)
		{
			m_animChanged = true;
			
			m_numShot = 0;

			m_spAnimator->SetAnimation(m_spModelWork->GetAnimation("Open"), 10.0f, false);
		}
	}

	if (m_isReload)
	{
		m_durationReload += m_reloadAccel * KdFPSController::GetInstance().GetDeltaTime();

		// リロード完了したら
		if (m_durationReload >= 1.0f)
		{
			if (m_num >= m_maxNumofOnce)
			{
				m_numOnce = m_maxNumofOnce;
				m_num -= m_maxNumofOnce;
			}
			else {
				m_numOnce = m_num;
				m_num = 0;
			}
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
			burstCount++;

			if (burstCount > m_numBurst)
			{
				burstCount = 0;
				m_durationFire = 0.0f;

				if (m_animChanged == true)
				{
					m_animChanged = false;
					m_trigger = false;
					m_spAnimator->SetAnimation(m_spModelWork->GetAnimation("Close"), 10.0f, false);
				}
			}
			else
			{
				Shot();
				m_numOnce -= 1;
			}

			m_durationBurst = 0.0f;
			
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
		auto trans = m_nodeMats[m_numShot]->matrix * m_mWorld;
		startPos = trans.Translation();

		direct = trans.Backward();
		direct.Normalize();
		Flash(trans);
	}
	if (m_shotSoundPath != "")
	{
		auto& am = KdAudioManager::Instance();
		am.Play(m_shotSoundPath)->SetVolume(am.GetSEVolume());
	}

	std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>();
	bullet->SetModelData(m_bulletModelPath);
	bullet->SetBulletParam(m_aliveTime,m_damage, m_range, startPos, direct, m_speed, m_dampingInterval, m_dampingRate);
	bullet->Init();
	bullet->SetBulletTrail(m_bulletTrailPath, m_bulletTrailColor,m_bulletTrailWidth, m_bulletTrailLength);
	if (m_wpParent.lock())
	{
		if (m_wpParent.lock()->GetTag() == tPlayer)
		{
			auto spParent = CameraManager::Instance().GetLockTarget(0);
			bullet->SetBulletType(Bullet::SightChasing, spParent);
		}
		else {
			auto& target = m_wpParent.lock()->GetCharacterTarget();
			bullet->SetBulletType(Bullet::SightChasing, target);
		}
	}
	bullet->SetTag(m_tag);
	bullet->SetChasingData(m_bulletRotateDeg,m_bulletLockAngle,m_bulletLostTime,m_bulletTrackingDistance);
	SceneManager::Instance().AddObject(bullet);

	m_numShot++;

	if ((UINT)m_numShot >= m_nodeMats.size())
	{
		m_numShot = 0;
	}

}

void Missile::OnTrigger()
{
	m_trigger = true;
}

void Missile::Editor_ImGui()
{
	ImGui::Text((const char*)u8"Burst回数は発射回数\n");
	ImGui::Text((const char*)u8"");
	GunBase::Editor_ImGui();
}

void Missile::Deserialize(const nlohmann::json& jsonObj)
{
	GunBase::Deserialize(jsonObj);

	MakeAnimator("Close", 20.0f, false);
}

void Missile::Serialize(nlohmann::json& outJson) const
{
	GunBase::Serialize(outJson);
}




