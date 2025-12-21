#include "Rifle.h"

#include "../../../Character/CharacterBase.h"
#include "../Bullet/Bullet.h"

#include"../../../../Scene/SceneManager.h"

#include "../../../Camera/CameraManager.h"

void Rifle::Init()
{
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

	m_mWorld = m_mLocalRot * m_mParentAttach * m_mParent;

	GunBase::Update();
}

void Rifle::Trigger()
{

	CheckTrigger();

	if (m_num <= 0 && m_numOnce <= 0) { return; }

	if (m_nowTrigger & m_AttackTrigger) { 
		OnTrigger(); }
	else {
		m_trigger = false; }

	if (m_isReload) 
	{
		m_durationReload += m_reloadAccel * KdFPSController::GetInstance().GetDeltaTime();

		// リロード完了したら
		if (m_durationReload >= 1.0f)
		{
			// 一回の残弾を最大値へ
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
	if (!m_trigger) { 
		return; }

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

void Rifle::Shot()
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

	if (m_shotSoundPath!="")
	{
	auto& am = KdAudioManager::Instance();
	am.Play(m_shotSoundPath)->SetVolume(am.GetSEVolume());
	}

	std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>();
	bullet->SetModelData(m_bulletModelPath);
	bullet->SetBulletParam(m_aliveTime, m_damage, m_range, startPos, direct, m_speed, m_dampingInterval, m_dampingRate);
	bullet->SetBulletTrail(m_bulletTrailPath, m_bulletTrailColor,m_bulletTrailWidth,m_bulletTrailLength);
	bullet->Init();
	bullet->SetTag(m_tag);

	SceneManager::Instance().AddObject(bullet);

	m_numShot++;

	if ((UINT)m_numShot >= m_nodeMats.size())
	{
		m_numShot = 0;
	}

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
	GunBase::Editor_ImGui();

}

void Rifle::Deserialize(const nlohmann::json& jsonObj)
{
	GunBase::Deserialize(jsonObj);
}

void Rifle::Serialize(nlohmann::json& outJson) const
{
	GunBase::Serialize(outJson);
}


