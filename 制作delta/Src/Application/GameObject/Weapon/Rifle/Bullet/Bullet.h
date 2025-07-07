#pragma once

#include"../../WeaponBase.h"


class Bullet :public WeaponBase
{
public:
	void Init()override;
	void Update()override;
	void PostUpdate()override;

	void DrawUnLit()override;

	void SetBulletParam(int _damage, float _range, const Math::Vector3& _startPos, const Math::Vector3 direction,float _speed,float _dampingInterval,float _dampingRate);
	void SetBulletTrail(const std::string& _path, const Math::Color _color, float _width, UINT _length);

	void OnHit()override;

private:

	int m_damage = 0;
	float m_range = 0.0f;
	Math::Vector3 m_direction = {};
	float m_speed = 0.0f;
	float m_dampingInterval = 0.0f;
	float m_dampingRate = 0.0f;
	Math::Vector3 m_startPos = {};

	std::shared_ptr<KdTrailPolygon> m_trail = nullptr;

};
