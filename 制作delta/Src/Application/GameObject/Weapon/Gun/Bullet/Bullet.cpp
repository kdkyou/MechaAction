#include "Bullet.h"

#include"../../../../main.h"
#include"../../../Character/CharacterBase.h"

#include "../../../../Scene/SceneManager.h"
#include "../../../Camera/CameraManager.h" 


void Bullet::Init()
{
	m_pCollider = std::make_unique<KdCollider>();
	
	if (m_spModelData == nullptr)
	{
		DirectX::BoundingSphere sphere;
		sphere.Center = m_mWorld.Translation();
		sphere.Radius = 1.0f;
		m_pCollider->RegisterCollisionShape("Bullet", sphere, KdCollider::TypeDamage);
	}
	else
	{
		m_pCollider->RegisterCollisionShape("Bullet", m_spModelData, KdCollider::TypeDamage);
	}

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_name = "Bullet";

	m_isExpired = false;

	m_attackNum = 1;
}

void Bullet::PreUpdate()
{

}

void Bullet::Update()
{
	//生存時間減少を先に
	m_aliveTime -= KdFPSController::GetInstance().GetDeltaTime();
	if (m_aliveTime <= 0.0f)
	{
		m_isExpired = true;
	}
	float sightTime = 0.0f;

	switch (m_moveType)
	{
	case Bullet::None:
		break;
	case Bullet::Sight:
		
		MoveSight();
		break;
	case Bullet::Chasing:

		MoveChasing();
		break;
	case Bullet::SightChasing:

	{

		m_durationSight += KdFPSController::GetInstance().GetDeltaTime();

		float sightTime = SIGHTTIME;

		if (m_durationSight < sightTime)
		{
			MoveSlow();
		}
		else {
			MoveChasing();
		}
	}

		break;
	case Bullet::SightScale:
		
		MoveSight();

		break;
	default:
		break;
	}


	// 向きから姿勢を再構築
	{
		Math::Vector3 forward = m_direction;
		forward.Normalize();

		Math::Vector3 up = Math::Vector3::Up;
		if (fabsf(forward.Dot(up)) > 0.99f) up = Math::Vector3::Right;

		m_mLocalRot = Math::Matrix::CreateWorld(Math::Vector3::Zero, -forward, up);
	}


	Math::Matrix trans = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = m_mLocalRot * trans;
	

	Intersects();
}

void Bullet::PostUpdate()
{
	// トレイル
	m_trail->AddPoint(m_mWorld);
}

void Bullet::Intersects()
{
	// その他球による衝突判定
	// ---- ---- ---- ---- ---- ----
	// ①当たり判定(球判定)用の情報を作成
	KdCollider::SphereInfo sphereInfo;
	sphereInfo.m_sphere.Center = m_mWorld.Translation();
	sphereInfo.m_sphere.Radius = 0.5f;
	sphereInfo.m_type = KdCollider::TypeGround;

	std::list<KdCollider::CollisionResult> retBumpList;
	{
		// ②HIT対象オブジェクトに総当たり
		for (auto& obj : SceneManager::Instance().GetTerrainList())
		{
			//if (obj->GetTag() != tPlayerAttack)
			{
				obj->Intersects(sphereInfo, &retBumpList);
			}
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
			hitDir = ret.m_hitDir;
			hit = true;

		}

	}

	DirectX::BoundingOrientedBox box;

	box.Center = GetPos();
	box.Extents = { 0.5f,0.5f,1.0f };
	UINT type = KdCollider::TypeDamage;
	KdCollider::BoxInfo boxInfo(type, box);

	/*if (m_tag == ObjectTag::tPlayerAttack)
	{
		for (auto& obj : SceneManager::Instance().GetEnemyList())
		{
			if (obj->Intersects(boxInfo, nullptr))
			{
				{
					OnHit();
					m_parameter = obj->GetParameter();
					obj->HitDamage(GetParameter());
					obj->OnHit();
				}
			}
		}
	}
	else if (m_tag == ObjectTag::tEnemyAttack)
	{
		for (auto& obj : SceneManager::Instance().GetPlayerList())
		{
			if (obj->Intersects(boxInfo, nullptr))
			{
				{
					
				}
			}
		}
	}*/


	if (hit)
	{
		OnHit();
	}

}

bool Bullet::Ray(const Math::Vector3& pos, const Math::Vector3& vec, float length)
{

	// レイ情報作成
	KdCollider::RayInfo rayInfo;
	rayInfo.m_pos = pos;
	rayInfo.m_dir = vec;
	rayInfo.m_range = length;
	rayInfo.m_type = KdCollider::TypeGround;

	std::list<KdCollider::CollisionResult> retBumpList;

	{
		// ②HIT対象オブジェクトに総当たり
		for (auto& obj : SceneManager::Instance().GetTerrainList())
		{
			obj->Intersects(rayInfo, &retBumpList);
		}

		rayInfo.m_type = KdCollider::TypeDamage;
		if (m_tag == ObjectTag::tEnemyAttack) {
			for (auto& obj : SceneManager::Instance().GetPlayerList()) {
				if (obj->Intersects(rayInfo, nullptr))
				{
					OnHit();
					obj->HitDamage(GetParameter());
					obj->OnHit();
					return true;
				}
			}
		}

		if (m_tag == ObjectTag::tPlayerAttack) {
			for (auto& obj : SceneManager::Instance().GetEnemyList()) {
				if (obj->Intersects(rayInfo, nullptr))
				{
					OnHit();
					obj->HitDamage(GetParameter());
					obj->OnHit();
					return true;
				}
			}
		}

	}




	float maxOverLap = 0;
	Math::Vector3 hitPos = Math::Vector3::Zero;
	bool hit = false;
	// ③結果を使って座標を補完する
	for (auto& ret : retBumpList)
	{
		if (maxOverLap < ret.m_overlapDistance)
		{
			maxOverLap = ret.m_overlapDistance;
			hitPos = ret.m_hitPos;
			hit = true;

		}

	}

	if (hit)
	{
		m_pos = hitPos;
		return true;
	}

	return false;
}

void Bullet::DrawLit()
{

	if (m_moveType != SightScale)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelData, m_mWorld);

	}
}

void Bullet::DrawUnLit()
{
	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_trail);

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);

}

void Bullet::DrawBright()
{
	if (m_moveType == SightScale)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelData,m_mWorld);
	}
}

void Bullet::OnHit()
{
	if (m_attackNum > 0)
	{
		m_attackNum -= 1;
	float damage = 0.0f;
	 damage = BulletDamage(m_startPos, m_mWorld.Translation(), m_damage, m_range, m_dampingInterval, m_dampingRate);
	 m_parameter = damage;
	 m_isExpired = true;
	 m_trail->SetEnable(false);
	float scale = CameraManager::Instance().CalcLength(m_mWorld.Translation());
	 KdEffekseerManager::GetInstance().Play("burn.efkefc", GetMatrix().Translation(),scale,1.0f,false);
	}

}

void Bullet::MoveSight()
{
	auto pos = m_mWorld.Translation();

	Math::Vector3 move = Math::Vector3::Zero;

	// 移動
	float len = m_speed * KdFPSController::GetInstance().GetDeltaTime();
	move = m_direction *len;

	if (!Ray(pos, m_direction, len)) {
		pos += move;

		m_pos = pos;
	}
}

void Bullet::MoveSlow()
{
	auto pos = m_mWorld.Translation();

	Math::Vector3 move = Math::Vector3::Zero;

	float progress = 1.0f / SIGHTTIME;
	progress *= m_durationSight;

	KdEase ease;

	float speed	= m_speed * ease.OutSine(progress);
	// 移動
	float len = speed * KdFPSController::GetInstance().GetDeltaTime();
	move = m_direction * len;

	if (!Ray(pos, m_direction, len)) {
		pos += move;

		m_pos = pos;
	}
}

void Bullet::MoveChasing()
{
	auto spTarget = m_wpTarget.lock();
	if (!spTarget) { 
		m_moveType = Sight; return; }


	Math::Vector3 move = Math::Vector3::Zero;

	// 自身の座標取得
	auto pos = m_pos;
	
	// 対象の座標取得
	Math::Vector3 targetPos = spTarget->GetCorrectionMatrix().Translation() + spTarget->GetMatrix().Translation();

	// 対象の方向ベクトル
	Math::Vector3 toTarget = targetPos - pos;

	float distance = toTarget.Length();

	// 距離が範囲外なら
	if (distance > m_trackingEndDistance) {
		m_moveType = Sight;
		return;
	}

	toTarget.Normalize();

	// 現在の向き (ミサイルが進んでいる方向)
	Math::Vector3 forward = m_direction;
	forward.Normalize();

	// 角度でロック解徐判定
	float dot = forward.Dot(toTarget);
	float angleDeg = acosf(dot) * KdToDegrees;

	// 角度外ならロスト時間の進行
	if (angleDeg > m_maxLockAngle)
	{
		m_durationLost += KdFPSController::GetInstance().GetDeltaTime();
	}
	// 内ならリセット
	else
	{
		m_durationLost = 0.0f;
	}

	if (m_durationLost > m_lockLostTime)
	{
		// 見失ったら直進に切り替え
		m_moveType = Sight;
		return;
	}

	// 向きを滑らかに補間
	Math::Vector3 newForward = SlerpDirection(forward, toTarget, (m_rotateSpeedDeg) * KdFPSController::GetInstance().GetDeltaTime());

	// 進行方向を再設定
	m_direction = newForward;

	// 移動
	float len = m_speed * KdFPSController::GetInstance().GetDeltaTime();
	move = m_direction * len;
	if (!Ray(pos, m_direction, len)) {
		pos += move;

		m_pos = pos;
	}
}



void Bullet::SetBulletParam(float _aliveTime, int _damage, float _range, const Math::Vector3& _startPos, const Math::Vector3 direction, float _speed, float _dampingInterval, float _dampingRate)
{
	m_aliveTime = _aliveTime;
	m_damage = _damage;
	m_range = _range;
	m_startPos = _startPos;
	m_pos = _startPos;
	m_direction = direction;
	m_speed = _speed;
	m_dampingInterval = _dampingInterval;
	m_dampingRate = _dampingRate;

	// 進行方向に回転行列を合わせる
	Math::Vector3 forward = direction;
	forward.Normalize();

	// Upベクトルが方向に近すぎると姿勢が不安定になるので切り替える
	Math::Vector3 up = Math::Vector3::Up;
	if (fabsf(forward.Dot(up)) > 0.99f) {
		up = Math::Vector3::Right;
	}

	// ローカル行列に回転をセット（位置はゼロ）
	m_mLocalRot = Math::Matrix::CreateWorld(Math::Vector3::Zero, -forward, up);

	SetPos(_startPos);

}

void Bullet::SetBulletTrail(const std::string& _path, const Math::Color _color, float _width, UINT _length)
{
	m_trail = std::make_shared<KdTrailPolygon>();
	if (_path != "")
	{
		m_trail->SetMaterial(KdAssets::Instance().m_textures.GetData(_path));
	}
	m_trail->SetColor(_color);
	m_trail->SetWidth(_width);
	m_trail->SetLength(_length);
	m_trail->SetPattern(KdTrailPolygon::Trail_Pattern::eBillboard);
	m_trail->ClearPoints();

}

void Bullet::SetBulletType(const moveType _type, const std::weak_ptr<CharacterBase>& _chasigTarget)
{
	m_moveType = _type;
	m_wpTarget = _chasigTarget;
}

void Bullet::SetChasingData(int rotateSpeedDeg, float lockAngle, float lostTime,float trackingDistance)
{
	m_rotateSpeedDeg = rotateSpeedDeg;
	m_maxLockAngle = lockAngle;
	m_lockLostTime = lostTime;
	m_trackingEndDistance = trackingDistance;
}

void Bullet::ScaleUp(float scale, int damageNum)
{
	SetScale(scale);
	m_attackNum = damageNum;
}
