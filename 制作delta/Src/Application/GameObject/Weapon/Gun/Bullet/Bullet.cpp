#include "Bullet.h"

#include"../../../../main.h"
#include"../../../Character/CharacterBase.h"

void Bullet::Init()
{
	m_pCollider = std::make_unique<KdCollider>();
	
	if (m_spModelData == nullptr)
	{
		DirectX::BoundingSphere sphere;
		sphere.Center = m_mWorld.Translation();
		sphere.Radius = 0.1f;
		m_pCollider->RegisterCollisionShape("Bullet", sphere, KdCollider::TypeDamage);
	}
	else
	{
		m_pCollider->RegisterCollisionShape("Bullet", m_spModelData, KdCollider::TypeDamage);
	}

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

}

void Bullet::Update()
{
	//生存時間減少を先に
	m_aliveTime -= KdFPSController::GetInstance().GetDeltaTime();
	if (m_aliveTime <= 0)
	{
		m_isExpired = true;
	}

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
			MoveSight();
		}
		else {
			MoveChasing();
		}
	}

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
	
	// トレイル
	m_trail->AddPoint(m_mWorld);


}

void Bullet::PostUpdate()
{
	
}

void Bullet::DrawUnLit()
{
	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_trail);

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);

}

void Bullet::OnHit()
{
	float damage = 0.0f;
	 damage = BulletDamage(m_startPos, m_mWorld.Translation(), m_damage, m_range, m_dampingInterval, m_dampingRate);
	 m_parameter = damage;
	 m_isExpired = true;
	 KdEffekseerManager::GetInstance().Play("burn.efkefc", GetMatrix().Translation(),1.0f,1.0f,false);

}

void Bullet::MoveSight()
{
	auto pos = m_mWorld.Translation();

	Math::Vector3 move = Math::Vector3::Zero;

	// 移動
	move = m_direction * m_speed * KdFPSController::GetInstance().GetDeltaTime();

	Application::Instance().m_log.AddLog("MissileDirection x:%.2f,y:%.2f,z:%.2f\n", m_direction.x, m_direction.y, m_direction.z);

	pos += move;

	m_pos = pos;
	
}

void Bullet::MoveChasing()
{
	auto spTarget = m_wpTarget.lock();
	if (!spTarget) { m_isExpired = true; return; }


	Math::Vector3 move = Math::Vector3::Zero;

	// 自身の座標取得
	auto pos = m_pos;
	
	// 対象の座標取得
	Math::Vector3 targetPos = spTarget->GetMatrix().Translation();

	// 対象の方向ベクトル
	Math::Vector3 toTarget = targetPos - pos;

	float distance = toTarget.Length();

	// 距離が範囲外なら
	if (distance > m_trackingEndDistance) {
		m_isExpired = true;
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


	Application::Instance().m_log.AddLog("MissileDirection x:%.2f,y:%.2f,z:%.2f\n", m_direction.x, m_direction.y, m_direction.z);

	// 移動
	move = m_direction * m_speed * KdFPSController::GetInstance().GetDeltaTime();
	pos += move;

	//Application::Instance().m_log.AddLog("MissileMove x:%.2f,y:%.2f,z:%.2f\n", move.x, move.y, move.z);
	
	m_pos = pos;
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
