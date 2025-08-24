#include"GunBase.h"

#include "../../Character/CharacterBase.h"

#include "../../Effect/Polygon/PolygonEffect.h"

#include "../../../Scene/SceneManager.h"

#include "../../UI/UIManager.h"


void GunBase::SetTarget(const std::weak_ptr<CharacterBase>& target) 
{
	m_wpTarget = target;
}

void GunBase::Editor_ImGui() 
{
	WeaponBase::Editor_ImGui();

	ImGui::DragInt((const char*)"ダメージ", &m_damage, 1, 0,10000);
	ImGui::DragFloat((const char*)"発射時間", &m_fireRate, 0.01f, 0.01f, 10.0f);
	ImGui::DragFloat((const char*)"リロード時間", &m_reloadTime, 0.01f, 0.01f, 15.0f);
	ImGui::DragFloat((const char*)"バースト時間", &m_burst, 0.01f, 0.01f, 15.0f);
	ImGui::DragInt((const char*)"バースト回数", &m_numBurst,1, 1, 15);
	ImGui::DragInt((const char*)"総弾数", &m_maxNum, 1,0);
	ImGui::DragInt((const char*)"装填数", &m_maxNumofOnce, 1,0);

	if (ImGui::Button((const char*)u8"テクスチャのロード"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			m_muzzleFlashPath = filepath;
		}
	}
}

void GunBase::Deserialize(const nlohmann::json& jsonObj)
{
	WeaponBase::Deserialize(jsonObj);
}

void GunBase::Serialize(nlohmann::json& outJson) const
{
	WeaponBase::Serialize(outJson);
}

void GunBase::Init()
{
	m_name = "Gun";
}

void GunBase::Update()
{
	if (m_tag == tPlayerAttack)
	{
		if (m_attachPath == "RightShoulderWeapon")
		{
			UIManager::GetInstance().SetRightAmmo(m_num);
			UIManager::GetInstance().SetRightAmmoOne(m_numOnce);
		}
		else if (m_attachPath == "LeftShoulderWeapon")
		{
			UIManager::GetInstance().SetLeftAmmo(m_num);
			UIManager::GetInstance().SetLeftAmmoOne(m_numOnce);
		}
	}
}

void GunBase::PostUpdate()
{
	if (m_spAnimator)
	{
		m_spAnimator->AdvanceTime(m_spModelWork->WorkNodes(), 30.0f);
	}
}

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

const Math::Matrix GunBase::RotateWeaponDirect(const Math::Vector2& enableAngle, const Math::Vector3& targetDir,const Math::Matrix& rotMat)
{
	auto localVec = rotMat.Backward();

	// 対象の方向をローカル空間に調整
	auto parentRotMat = m_mParentAttach * m_mParent;
	parentRotMat.Translation(Math::Vector3::Zero);
	Math::Vector3 toVec = targetDir;
	toVec = DirectX::XMVector3TransformNormal(targetDir, parentRotMat.Invert());

	toVec.Normalize();

	// ヨー(左右)とピッチ(上下)の角度差を取得 
	float yaw = atan2f(toVec.x, toVec.z);
	float pitch = atan2f(toVec.y, sqrtf(toVec.x * toVec.x + toVec.z * toVec.z));

	yaw = DirectX::XMConvertToDegrees(yaw);
	pitch = DirectX::XMConvertToDegrees(pitch);

	// クランプ
	float clampYaw = std::clamp(yaw, -enableAngle.x, enableAngle.x);
	float clampPitch = std::clamp(pitch, -enableAngle.y, enableAngle.y);

	Math::Matrix mRot = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(clampYaw)) * Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(-clampPitch));

	// ローカル回転を更新
	return mRot;

}

bool GunBase::Flash(const Math::Matrix& occurMat)
{
	if (m_muzzleFlashPath == "") { return false; }

	auto polygon = std::make_shared<PolygonEffect>();

	polygon->Init();
	polygon->SetParam(m_muzzleFlashPath, 0.05f, PolygonEffect::eBright, false,occurMat);

	SceneManager::Instance().AddObject(polygon);

	return true;

}
