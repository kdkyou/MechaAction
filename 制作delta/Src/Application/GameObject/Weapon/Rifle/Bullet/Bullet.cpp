#include "Bullet.h"

void Bullet::Init()
{
	// 弾は当てられる方である
	m_pCollider = std::make_unique<KdCollider>();
	DirectX::BoundingSphere sphere;
	sphere.Center = m_mWorld.Translation();
	sphere.Radius = 0.1f;
	m_pCollider->RegisterCollisionShape("Bullet", sphere, KdCollider::TypeDamage);

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

}

void Bullet::Update()
{
	auto pos = m_mWorld.Translation();
	// 移動
	Math::Vector3 move = m_direction * m_speed * KdFPSController::GetInstance().GetDeltaTime();

	pos += move;

	m_mWorld.Translation(pos);

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
}

void Bullet::SetBulletParam(int _damage, float _range, const Math::Vector3& _startPos,const Math::Vector3 direction ,float _speed, float _dampingInterval, float _dampingRate)
{
	m_damage = _damage;
	m_range = _range;
	m_startPos = _startPos;
	m_direction = direction;
	SetPos(_startPos);
	m_speed = _speed;
	m_dampingInterval = _dampingInterval;
	m_dampingRate = _dampingRate;
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
