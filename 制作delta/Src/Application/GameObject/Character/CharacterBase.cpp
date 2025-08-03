#include "CharacterBase.h"

#include "../../Scene/SceneManager.h"

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

	//if(){}
	if (!m_spModelWork)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
	}

	m_spModelWork->SetModelData(KdAssets::Instance().m_modeldatas.GetData(path));

}

void CharacterBase::Editor_ImGui()
{
	KdGameObject::Editor_ImGui();

	if (ImGui::Button((const char*)u8"モデルのロード"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			SetModelWork(filepath);
		}
	}

	ImGui::DragFloat((const char*)u8"HP", &m_hp);
}

void CharacterBase::Deserialize(const nlohmann::json& jsonObj)
{
	KdGameObject::Deserialize(jsonObj);
}

void CharacterBase::Serialize(nlohmann::json& outJson) const
{
	KdGameObject::Serialize(outJson);
}

bool CharacterBase::Move(float speed, const Math::Vector3& dir, const KdCollider::Type type, bool ray, bool rotate, bool direct, bool step)
{
	auto direction = dir;
	direction.Normalize();

	auto pos = m_mWorld.Translation();

	auto deltaTime = KdFPSController::GetInstance().GetDeltaTime();

	Math::Vector3 move = Math::Vector3::Zero;

	auto deltaSpeed = speed * deltaTime;

	if (step == true)
	{
		pos += {0.0f, 0.05f, 0.0f};
	}

	bool isHit = RayCast(pos, direction, deltaSpeed, type, move);
	if (ray == true)
	{
		return isHit;
	}

	if (isHit == true)
	{
		auto corre = (direction * deltaSpeed) * 0.1f;
		pos = move - corre;


		if (direct == true)
		{
			pos = move;
		}
	}
	else
	{
		move = direction * deltaSpeed;
		pos += move;
	}



	if (rotate == true)
	{
		UpdateRotate(direction);
	}


	m_pos = pos;

	Math::Matrix _rotation = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rot.y));
	m_mWorld = m_scale * _rotation * Math::Matrix::CreateTranslation(pos);


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
			if(obj->GetTag() != tPlayerAttack)
			{
				obj->Intersects(rayInfo, &retRayList);
			}
		}
	}

	if (type & KdCollider::TypeDamage)
	{
		// ②HIT判定対象オブジェクトに総当たり
		for (auto& obj : SceneManager::Instance().GetEnemyList())
		{
			{
				obj->Intersects(rayInfo, &retRayList);
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

	return hit;
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

	auto  flg =  RayCast(pos, vTarget, length, KdCollider::TypeGround, rPos);

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


bool CharacterBase::SphereCast(const Math::Vector3& center, const Math::Vector3& vec, const float radius, const KdCollider::Type& type, Math::Vector3& resultPos)
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
		for (auto obj : SceneManager::Instance().GetTerrainList())
		{

			obj->Intersects(sphereInfo, &retBumpList);

		}

	}

	if (type & KdCollider::TypeDamage)
	{
		// ②HIT対象オブジェクトに総当たり
		for (auto obj : SceneManager::Instance().GetEnemyList())
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

		Math::Vector3 newPos = GetPos() + (hitDir * maxOverLap);
		resultPos = newPos;
	}


	return true;
}