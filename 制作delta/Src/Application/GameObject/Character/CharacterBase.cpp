#include "CharacterBase.h"

#include "../../Scene/SceneManager.h"

#include "../Effect/Polygon/PolygonEffect.h"
#include "../Weapon/WeaponBase.h"
#include "../Camera/CameraManager.h"
#include "../Camera/CameraBase.h"

void CharacterBase::PostUpdate()
{
	auto spCamera = CameraManager::Instance().GetMapCamera().lock();
	if (spCamera)
	{
		if (SceneManager::Instance().GetTerrainList().size() <= 0) { return; }
		Math::Vector3 resultPos;
		spCamera->GetCamera()->ConvertWorldToScreenDetail(m_mWorld.Translation(),resultPos);

		resultPos.x = std::clamp(resultPos.x, (float)- EditorData::GetInstance().m_ScreenWh, (float)EditorData::GetInstance().m_ScreenWh);
		resultPos.y = std::clamp(resultPos.y, (float)-EditorData::GetInstance().m_ScreenHh, (float)EditorData::GetInstance().m_ScreenHh);

		auto camPos = spCamera->GetMatrix().Translation();
		Math::Vector3 rayDir = Math::Vector3::Zero;
		float rayRange = 1000.0f;
		POINT Pointpos = { resultPos.x,resultPos.y };

		spCamera->GetCamera()->GenerateRayInfoFromClientPos(Pointpos,camPos,rayDir,rayRange);

		Math::Vector3 endRayPos = camPos + (rayDir * rayRange);
		KdCollider::RayInfo ray(KdCollider::TypeGround, camPos, endRayPos);
		std::list<KdCollider::CollisionResult>results;

		for (auto& obj : SceneManager::Instance().GetTerrainList())
		{
			if (obj->GetTag() != tPlayerAttack)
			{
				obj->Intersects(ray, &results);
			}
		}

		Math::Vector3 pos = Math::Vector3::Zero;
		if (results.size())
		{
			for (auto& result : results)
			{
				pos = result.m_hitPos;
			}
		}

		m_mMarker = m_mWorld;
	}
}

void CharacterBase::GenerateDepthMapFromLight()
{
	if (m_spModelWork)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld);
	}

	if (m_spModelData)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelData, m_mWorld);
	}
}

void CharacterBase::DrawMarker()
{
	if (!m_spMrkModel) { return; }

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spMrkModel, m_mMarker);

}

void CharacterBase::HitDamage(float damage)
{
	if (m_hp - damage < 0)
	{
		m_hp = 0;
	}
	else
	{
		m_hp -= damage;
	}

}

void CharacterBase::SetModelWork(const std::string& path)
{
	if (path == "") { return; }

	m_modelPath = path;
	//if(){}
	if (!m_spModelWork)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
	}

	m_spModelWork->SetModelData(KdAssets::Instance().m_modeldatas.GetData(path));
	if (m_pCollider)
	{
		if (m_tag == tEnemy)
		{
			m_pCollider->RegisterCollisionShape("Enemy", m_spModelWork, KdCollider::TypeDamage);
		}
		else {
			m_pCollider->RegisterCollisionShape("Player", m_spModelWork, KdCollider::TypeDamage);
		}
	}
}

void CharacterBase::Editor_ImGui()
{
	if (ImGui::Button((const char*)u8"消去")) {
		m_isExpired = true;
	}

	if (ImGui::DragFloat3("Pos", &m_pos.x, 0.1f, -FLT_MAX, FLT_MAX))
	{
		m_mWorld = Math::Matrix::CreateScale(m_scale) * Math::Matrix::CreateFromYawPitchRoll(m_rot * KdToRadians) * Math::Matrix::CreateTranslation(m_pos);
	}
	if(ImGui::DragFloat3("Scale", &m_scale.x, 0.1f, -FLT_MAX, FLT_MAX)) {
		m_mWorld = Math::Matrix::CreateScale(m_scale) * Math::Matrix::CreateFromYawPitchRoll(m_rot * KdToRadians) * Math::Matrix::CreateTranslation(m_pos);
	}
	if(ImGui::DragFloat3("Rotation", &m_rot.x, 0.1f, -FLT_MAX, FLT_MAX))
	{
		m_mWorld = Math::Matrix::CreateScale(m_scale) * Math::Matrix::CreateFromYawPitchRoll(m_rot * KdToRadians) * Math::Matrix::CreateTranslation(m_pos);
	}

	static const char* dirNames[] = { "tNone", "tPlayer", "tEnemy", "tPlayerAttack","tEnemyAttack" ,"tTerrain","tUI" };
	int tag = static_cast<int>(m_tag);
	if (ImGui::Combo((const char*)u8"タグ設定", &tag, dirNames, IM_ARRAYSIZE(dirNames)))
	{
		m_tag = static_cast<ObjectTag>(tag);
	}

	ImGui::Checkbox((const char*)u8"カメラ対象", &m_isCameraTarget);
	
	


	if (ImGui::Button((const char*)u8"モデルのロード"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			SetModelWork(filepath);
		}
	}

	ImGui::DragFloat((const char*)u8"HP", &m_hp);
	ImGui::DragFloat((const char*)u8"NockBackDamage", &m_nockBackDamage);
	ImGui::DragFloat3((const char*)u8"Correction", &m_correction.x, 0.1f, -20.0f, 20.0f);

	ImGui::ColorEdit3("EmissiveColor", &m_emissiveColor.x, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

	static std::string str = "";
	if (ImGui::BeginCombo("SelectObject", str.empty() ? (const char*)u8"選択してください" : str.c_str()))
	{
		for (auto obj : KdGameObjectFactory::Instance().GetRegisterWeaponList())
		{
			if (ImGui::Selectable(obj.c_str(), obj == str))
			{
				str = obj;
			}
		}

		ImGui::EndCombo();
	}

	if (ImGui::Button((const char*)u8"ウェポン追加"))
	{
		if (!str.empty())
		{
			auto obj = KdGameObjectFactory::Instance().CreateWeaponBase(str);
			if (obj)
			{
				auto parent = m_wpBase.lock();
				if(!parent){return;}

				obj->Init();
				obj->SetParent(parent);
				SceneManager::Instance().AddObject(obj);
				m_wpWeapons.push_back(obj);
			}
		}
	}

	if (ImGui::BeginListBox((const char*)u8"武器リスト"))
	{
		for (auto& wpObj : m_wpWeapons)
		{
			auto obj = wpObj.lock();
			std::string str = obj->GetName();
			//bool isSelected = (selectedObj.lock() == obj);  // 今の選択と同じか？

			ImGui::PushID(obj.get());
			if (ImGui::Selectable((const char*)str.c_str()))
			{
				EditorData::GetInstance().SelectWeapon = obj;
			}
			ImGui::PopID();
		}
		ImGui::EndListBox();
	}
}

void CharacterBase::Deserialize(const nlohmann::json& jsonObj)
{
	KdGameObject::Deserialize(jsonObj);

	m_mWorld = Math::Matrix::CreateFromYawPitchRoll(m_rot) * Math::Matrix::CreateTranslation(m_pos);

	KdJsonUtility::GetValue(jsonObj, "ModelPath", &m_modelPath);
	SetModelWork(m_modelPath);
	KdJsonUtility::GetValue(jsonObj, "HP", &m_hp);
	KdJsonUtility::GetValue(jsonObj, "NockBackDamage", &m_nockBackDamage);
	KdJsonUtility::GetArray(jsonObj, "Correction", &m_correction.x, 3);

	if (jsonObj.contains("Weapons"))
	{
		for (auto& weaponsData : jsonObj["Weapons"])
		{
			std::string str;
			KdJsonUtility::GetValue(weaponsData, "Name", &str);
			if (!str.empty())
			{
				auto obj = KdGameObjectFactory::Instance().CreateWeaponBase(str);
				if (obj)
				{
					auto parent = m_wpBase.lock();

					if (parent->GetTag() == tPlayer)
					{
						obj->SetTag(tPlayerAttack);
					}
					else if (parent->GetTag() == tEnemy)
					{
						obj->SetTag(tEnemyAttack);
					}
					obj->SetParent(parent);
					obj->Init();
					obj->Deserialize(weaponsData);
					SceneManager::Instance().AddObject(obj);
					m_wpWeapons.push_back(obj);
				}
			}
		}
	}

}


void CharacterBase::Serialize(nlohmann::json& outJson) const
{
	KdGameObject::Serialize(outJson);
	outJson["HP"] = m_hp;
	outJson["NockBackDamage"] = m_nockBackDamage;
	outJson["Correction"] = KdJsonUtility::CreateArray(&m_correction.x, 3);
	outJson["ModelPath"] = m_modelPath;

	nlohmann::json weaponsArray = nlohmann::json::array();

	for (auto& obj : m_wpWeapons)
	{
		auto weapon = obj.lock();
		if (weapon)
		{
			nlohmann::json weaponJson;
			weapon->Serialize(weaponJson);
			weaponsArray.push_back(weaponJson);
		}
	}

	outJson["Weapons"] = weaponsArray;
}

void CharacterBase::SetWeapon()
{
	for (auto& weapon : m_wpWeapons)
	{
		if (m_wpCharacterTarget.lock())
		{
			auto obj = weapon.lock();
			obj->SetTarget(m_wpCharacterTarget);
		}
	}
}

bool CharacterBase::Move(float speed, const Math::Vector3& dir, const KdCollider::Type type, bool ray, bool rotate, bool direct, bool step)
{
	auto direction = dir;
	direction.Normalize();

	auto pos = m_mWorld.Translation();

	auto deltaTime = KdFPSController::GetInstance().GetDeltaTime();

	Math::Vector3 move = Math::Vector3::Zero;

	auto deltaSpeed = speed * deltaTime;
	auto correc = 5.0f;
	if (step == true)
	{
		pos -= direction * 0.006f;
		deltaSpeed += correc;
	}

	bool isHit = RayCast(pos, direction, deltaSpeed, type, move);
	if (ray == true)
	{
		return isHit;
	}

	if (isHit == true)
	{
		auto corre = (direction * deltaSpeed) * 0.05f;
		pos = move - corre;


		if (direct == true)
		{
			pos = move;
		}

		if (step == true)
		{
			//move -= {0.0f, 5.0f, 0.0f};
		}

		auto center = pos + Math::Vector3(0.0f, 0.42f, 0.0f);
		SphereCast(center, 0.419f, KdCollider::TypeGround, pos);
	}
	else
	{
		deltaSpeed = deltaSpeed - correc;
		move = direction * deltaSpeed;
		pos += move;
	}

	if (rotate == true)
	{
		UpdateRotate(direction);
	}


	m_pos = pos;

	Math::Matrix _rotation = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rot.y));
	m_mWorld = m_mScale * _rotation * Math::Matrix::CreateTranslation(m_pos);


	return isHit;
}

bool CharacterBase::MoveSwept(float speed, const Math::Vector3& dir, const KdCollider::Type type, bool ray, bool rotate, bool direct, bool step)
{
	auto direction = dir;
	direction.Normalize();

	auto pos = m_mWorld.Translation();

	auto deltaTime = KdFPSController::GetInstance().GetDeltaTime();

	Math::Vector3 move = Math::Vector3::Zero;

	auto deltaSpeed = speed * deltaTime;

	auto empty = type;

	bool isDir = direct;

	bool isRay = ray;

	if (step == true)
	{
		pos -= direction * 0.05f;
	}

	Math::Vector3 oldPos = pos;
	Math::Vector3 moveVec = dir * deltaSpeed;
	Math::Vector3 newPos = oldPos + moveVec;

	float len = dir.Length();

	float m_radius = len + 1.5f;

	KdCollider::CollisionResult hit;

	bool isHit = false;

	if (CheckSweptSphere(oldPos, newPos, m_radius, hit))
	{
		// 衝突直前の位置にセット
		pos = hit.m_hitPos + hit.m_hitDir * m_radius;

		// スライド移動（壁に沿わせる場合）
		Math::Vector3 slide = moveVec - (moveVec.Dot(hit.m_hitDir)) * hit.m_hitDir;
		pos += slide;

		isHit = true;
	}
	else
	{
		// 当たらなければ普通に移動
		pos = newPos;
	}

	if (rotate == true)
	{
		UpdateRotate(direction);
	}

	m_pos = pos;

	Math::Matrix _rotation = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rot.y));
	m_mWorld = m_mScale * _rotation * Math::Matrix::CreateTranslation(m_pos);


	return isHit;
}

bool CharacterBase::RayCast(const Math::Vector3& startPos, const Math::Vector3& vec, const float length, const KdCollider::Type& type, Math::Vector3& resultPos)
{
	KdCollider::RayInfo rayInfo;

	rayInfo.m_pos = startPos;		// レイの発射位置を設定

	rayInfo.m_dir = vec;				// レイの発射方向を設定

	rayInfo.m_range = length;		// レイの長さ

	// 当たり判定をしたいタイプを設定
	rayInfo.m_type = type;

	if (rayInfo.m_dir.Length() == 0) { return false; }

	bool hit = false;

	std::list<KdCollider::CollisionResult> retRayList;

	if (type & KdCollider::TypeGround)
	{
		// ②HIT判定対象オブジェクトに総当たり
		for (auto& obj : SceneManager::Instance().GetTerrainList())
		{
			if (obj->GetTag() != tPlayerAttack)
			{
				obj->Intersects(rayInfo, &retRayList);
			}
		}
	}

	if (type & KdCollider::TypeDamage)
	{
		if (m_tag == tPlayer) {

			// ②HIT判定対象オブジェクトに総当たり
			for (auto& obj : SceneManager::Instance().GetEnemyList())
			{
				{
					obj->Intersects(rayInfo, &retRayList);
				}
			}

		}

		if (m_tag == tEnemy)
		{

			for (auto& obj : SceneManager::Instance().GetPlayerList())
			{
				{
					obj->Intersects(rayInfo, &retRayList);
				}
			}

		}

	}

	// ③結果を使って座標を補完する
			// レイに当たったリストから一番近いオブジェクトを検出
	float maxOverLap = 0;
	Math::Vector3 hitPos = Math::Vector3::Zero;
	for (auto& ret : retRayList)
	{
		// レイを遮断しオーバーした長さが
		// 一番長いものを探す
		if (maxOverLap < ret.m_overlapDistance)
		{
			maxOverLap = ret.m_overlapDistance;
			resultPos = ret.m_hitPos;

			hit = true;

		}
	}

	if (type & KdCollider::TypeGround)
	{
		if (vec.y <= -0.6f)
		{
			if (hit)
			{
				m_isGround = true;
				m_gravity = 0.0f;
			}
			else
			{
				m_isGround = false;
			}
		}
	}

	return hit;
}

bool CharacterBase::Gravity(const Math::Vector3& startPos, const Math::Vector3& vec, const float length)
{
	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = startPos;
	sphere.m_sphere.Radius = length;   // 足の半径
	sphere.m_type = KdCollider::TypeGround;

	if (vec.Length() == 0) { return false; }
	

	bool hit = false;

	// ③結果を使って座標を補完する
	Math::Vector3 pos = {};
	if (SphereCast(sphere.m_sphere.Center, sphere.m_sphere.Radius, KdCollider::TypeGround,pos))
	{
		m_pos = pos;
		m_isGround = true;
		m_gravity = 0.0f;
	}
	else {
		m_isGround = false;
	}


	Math::Matrix _rotation = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rot.y));
	m_mWorld = m_mScale * _rotation * Math::Matrix::CreateTranslation(m_pos);


	return hit;
}

bool CharacterBase::CheckSweptSphere(const Math::Vector3& oldPos, const Math::Vector3& newPos, float radius, KdCollider::CollisionResult& out)
{
	Math::Vector3 dir = newPos - oldPos;
	float len = dir.Length();
	if (len < FLT_EPSILON) return false;
	dir.Normalize();

	// Ray情報を作る（球の分だけ余裕を持たせる）
	KdCollider::RayInfo ray;
	ray.m_pos = oldPos;
	ray.m_dir = dir;
	ray.m_range = len + radius;   // 移動距離 + 球の半径
	ray.m_type = KdCollider::TypeGround;

	std::list<KdCollider::CollisionResult> results;
	for (auto& obj : SceneManager::Instance().GetTerrainList())
	{
		if (obj->GetTag() != ObjectTag::tPlayerAttack)
		{
			obj->Intersects(ray, &results);
		}
	}

	// 最も近い衝突を選ぶ
	float minDist = FLT_MAX;
	KdCollider::CollisionResult nearest;
	for (auto& r : results)
	{
		if (r.m_overlapDistance < minDist)
		{
			minDist = r.m_overlapDistance;
			nearest = r;
		}
	}

	if (minDist < FLT_MAX)
	{
		out.m_hitPos = nearest.m_hitPos;
		out.m_hitDir = nearest.m_hitDir;
		out.m_overlapDistance = minDist;
		return true;
	}

	return false;
}

bool CharacterBase::SearchDetect(const Math::Vector3& hitPos, const Math::Matrix& myMat, float viewRange)
{
	// オブジェクトの位置から方向を取得
	auto targetDirect = hitPos - myMat.Translation();

	// 現在の方向
	auto nowDir = myMat.Backward();

	// オブジェクトの角度算出
	float d = nowDir.Dot(targetDirect);
	// 丸め誤差使用
	d = std::clamp(d, -1.0f, 1.0f);
	// デグリー角に
	float targetAngle = DirectX::XMConvertToDegrees(acos(d));

	if (targetAngle < viewRange)
	{
		return true;
	}

	return false;
}

bool CharacterBase::SeaarchObstacle(const Math::Vector3& pos, const Math::Vector3& vec, const float length)
{
	auto rPos = Math::Vector3::Zero;

	Math::Vector3 vTarget = vec;
	vTarget.Normalize();

	auto  flg = RayCast(pos, vTarget, length, KdCollider::TypeGround, rPos);

	if (flg)
	{
		return false;
	}
	else {
		return true;
	}
}

void CharacterBase::BoostRotate(const Math::Vector3& vec)
{
	auto nowVec = GetMatrix().Backward();

	//内積を使って回転する角度を求める
	float d = nowVec.Dot(vec);
	//dの中にはコサインΘが入っている

	//角度求める
	float ang = DirectX::XMConvertToDegrees(acos(d));

	if (ang >= 0.1f)
	{
		Math::Vector3 c = vec.Cross(nowVec);



		if (c.y >= 0)
		{
			//右回転
			m_rot.y -= ang;
		}
		else
		{
			//左回転
			m_rot.y += ang;
		}
	}

	if (m_rot.y > 360)
	{
		m_rot.y -= 360;
	}
	else if (m_rot.y < 0)
	{
		m_rot.y += 360;
	}

}

bool CharacterBase::SearchPlayer()
{

	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = m_mWorld.Translation() + m_correction;
	sphere.m_sphere.Radius = m_dist.y;
	sphere.m_type = KdCollider::TypeDamage;

	for (auto& obj : SceneManager::Instance().GetPlayerList())
	{
		if (obj->Intersects(sphere, nullptr))
		{
			m_wpCharacterTarget = obj;
			return true;
		}
	}

	return false;
}

const bool CharacterBase::Burn()
{
	if (m_burnPath == "") { return false; }

	auto polygon = std::make_shared<PolygonEffect>();

	polygon->Init();
	auto pos = m_mWorld.Translation() + m_correctionMat.Translation();
	auto occurMat = Math::Matrix::CreateTranslation(pos);
	polygon->SetParam(m_burnPath, 0.35f, PolygonEffect::eBright, false, occurMat);
	polygon->EffectScale(10.0f);

	SceneManager::Instance().AddObject(polygon);

	return true;
}


bool CharacterBase::SphereCast(const Math::Vector3& center, const float radius, const KdCollider::Type& type, Math::Vector3& resultPos)
{
	// その他球による衝突判定
	// ---- ---- ---- ---- ---- ----
	// ①当たり判定(球判定)用の情報を作成
	KdCollider::SphereInfo sphereInfo;
	sphereInfo.m_sphere.Center = center;
	sphereInfo.m_sphere.Radius = radius;
	sphereInfo.m_type = type;

	std::list<KdCollider::CollisionResult> retBumpList;

	if (type & KdCollider::TypeGround)
	{

		// ②HIT対象オブジェクトに総当たり
		for (auto& obj : SceneManager::Instance().GetTerrainList())
		{
			if (obj->GetTag() != tPlayerAttack)
			{
				obj->Intersects(sphereInfo, &retBumpList);
			}
		}

	}

	if (type & KdCollider::TypeDamage)
	{
		// ②HIT対象オブジェクトに総当たり
		for (auto& obj : SceneManager::Instance().GetEnemyList())
		{
			obj->Intersects(sphereInfo, &retBumpList);
		}
	}

	float maxOverLap = 0;
	Math::Vector3 hitDir = Math::Vector3::Zero;
	bool hit = false;
	// ③結果を使って座標を補完する
	for (auto& ret : retBumpList)
	{
		if (maxOverLap < ret.m_overlapDistance)
		{
			maxOverLap = ret.m_overlapDistance;
			resultPos = ret.m_hitPos;
			hitDir = ret.m_hitDir;
			hit = true;

		}

	}

	if (hit)
	{
		auto pos = m_mWorld.Translation();
		Math::Vector3 newPos = pos + (hitDir * maxOverLap);
		resultPos = newPos;
		return true;
	}

	return false;
}


