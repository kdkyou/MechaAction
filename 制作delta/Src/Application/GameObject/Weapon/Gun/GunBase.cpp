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

	ImGui::Separator();
	ImGui::DragFloat((const char*)u8"発射間隔時間", &m_fireRate, 0.01f, 0.01f, 10.0f);
	ImGui::DragFloat((const char*)u8"リロード時間", &m_reloadTime, 0.01f, 0.01f, 15.0f);
	ImGui::DragFloat((const char*)u8"バースト時間", &m_burst, 0.01f, 0.01f, 15.0f);
	ImGui::DragInt((const char*)u8"バースト回数", &m_numBurst,1, 1, 15);
	ImGui::DragInt((const char*)u8"総弾数", &m_maxNum, 1,0);
	ImGui::DragInt((const char*)u8"装填数", &m_maxNumofOnce, 1,0);

	static std::string str;
	ImGui::InputText((const char*)u8"発射口", &str);
	if (str != "")
	{
		if(ImGui::Button((const char*)u8"セット")){
			SetNodeMats(str);
		}
	}

	ImGui::Separator();
	ImGui::Text((const char*)u8"弾パラメータ");
	if (ImGui::Button((const char*)u8"弾モデルのロード"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			m_bulletModelPath = filepath;
		}
	}
	ImGui::DragInt((const char*)u8"ダメージ", &m_damage, 1, 0,10000);
	ImGui::DragFloat((const char*)u8"時間", &m_aliveTime,0.01f,0.0f);
	ImGui::DragFloat((const char*)u8"速さ", &m_speed, 0.1f, 0.0f);
	ImGui::DragFloat((const char*)u8"有効範囲",&m_range,0.1f,0.0f);
	ImGui::DragFloat((const char*)u8"減衰間隔", &m_dampingInterval, 0.01f, 0.0f);
	ImGui::DragFloat((const char*)u8"減衰力", &m_dampingRate, 0.01f, 0.0f);
	ImGui::DragFloat((const char*)u8"追尾可能角度", &m_bulletLockAngle,0.01f,0.0f);
	ImGui::DragFloat((const char*)u8"延長索敵時間", &m_bulletLostTime,0.01f,0.0f);
	ImGui::DragFloat((const char*)u8"旋回角度", &m_bulletRotateDeg,0.01f,0.0f);
	ImGui::DragFloat((const char*)u8"最大追尾距離",&m_bulletTrackingDistance,0.1f,0.0f);
	//ImGui::DragFloat((const char*)u8"移動方式");


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
	KdJsonUtility::GetValue(jsonObj, "AliveTime", &m_aliveTime);
	KdJsonUtility::GetValue(jsonObj, "Damage", &m_damage);
	KdJsonUtility::GetValue(jsonObj, "Range", &m_range);
	KdJsonUtility::GetValue(jsonObj, "FireRate", &m_fireRate);
	KdJsonUtility::GetValue(jsonObj, "MaxNum", &m_maxNum);
	KdJsonUtility::GetValue(jsonObj, "MaxNumOfOnce", &m_maxNumofOnce);
	KdJsonUtility::GetValue(jsonObj, "ReloadTime", &m_reloadTime);
	KdJsonUtility::GetValue(jsonObj, "BurstTime", &m_burst);
	KdJsonUtility::GetValue(jsonObj, "BurstNum", &m_numBurst);
	KdJsonUtility::GetValue(jsonObj, "DampingInterval", &m_dampingInterval);
	KdJsonUtility::GetValue(jsonObj, "DampingRate",&m_dampingRate);
	KdJsonUtility::GetValue(jsonObj, "ShotSoundPath", &m_shotSoundPath);
	KdJsonUtility::GetValue(jsonObj, "BulletModelPath", &m_bulletModelPath);
	KdJsonUtility::GetValue(jsonObj, "BulletLockAngle", &m_bulletLockAngle);
	KdJsonUtility::GetValue(jsonObj, "BulletRotateDeg", &m_bulletRotateDeg);
	KdJsonUtility::GetValue(jsonObj, "BulletLostTime", &m_bulletLostTime);
	KdJsonUtility::GetValue(jsonObj, "BulletTrackingDistance", &m_bulletTrackingDistance);
	KdJsonUtility::GetValue(jsonObj, "BulletTrailPath", &m_bulletTrailPath);
	KdJsonUtility::GetValue(jsonObj, "Speed", &m_speed);

	m_num = m_maxNum;
	m_numOnce = m_maxNumofOnce;

	m_nodeMats.clear();

	if (jsonObj.contains("ShotNodes"))
	{
		for (const auto& nodeJson : jsonObj["ShotNodes"])
		{
			SetNodeMats(nodeJson["Name"].get<std::string>());
		}
	}

}

void GunBase::Serialize(nlohmann::json& outJson) const
{
	WeaponBase::Serialize(outJson);

	outJson["AliveTime"] = m_aliveTime;
	outJson["Damage"] = m_damage;
	outJson["Range"] = m_range;
	outJson["FireRate"] = m_fireRate;
	outJson["MaxNum"] = m_maxNum;
	outJson["MaxNumOfOnce"] = m_maxNumofOnce;
	outJson["ReloadTime"] = m_reloadTime;
	outJson["BurstTime"] = m_burst;
	outJson["BurstNum"] = m_numBurst;
	outJson["BulletModelPath"] = m_bulletModelPath;
	outJson["BulletRotateDeg"] = m_bulletRotateDeg;
	outJson["BulletLockAngle"] = m_bulletLockAngle;
	outJson["BulletLostTime"] = m_bulletLostTime;
	outJson["BulletTrackingDistance"] = m_bulletTrackingDistance;
	outJson["DampingInterval"] = m_dampingInterval;
	outJson["DampingRate"] = m_dampingRate;
	outJson["BulletTrailPath"] = m_bulletTrailPath;
	outJson["ShotSoundPath"] = m_shotSoundPath;
	outJson["Speed"] = m_speed;

	outJson["ShotNodes"] = nlohmann::json::array();
	for (const auto& node : m_nodeMats)
	{
		if (!node)continue;

		nlohmann::json nodejson;
		nodejson["Name"] = node->name;
		outJson["ShotNodes"].push_back(nodejson);
	}
}

void GunBase::Init()
{
	m_name = "Gun";
}

void GunBase::PreUpdate()
{
	WeaponBase::PreUpdate();
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
		SetModel(gunModelPath);
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

void GunBase::SetBulletChaisingData(float bulletRotateDeg, float bulletLockAngle, float bulletLostTime, float bulletTrackingDistance)
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
		auto node = std::make_shared<ShotNode>();
		Math::Matrix mat = pNode->m_worldTransform;
		node->matrix = mat;
		node->name = nodeName;
		m_nodeMats.push_back(node);
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
