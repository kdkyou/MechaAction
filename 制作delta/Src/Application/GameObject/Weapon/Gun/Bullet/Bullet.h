#pragma once

#include"../../WeaponBase.h"

#define SIGHTTIME 0.6f;

class CharacterBase;

class Bullet :public WeaponBase
{
public:

	enum moveType
	{
		None,
		Sight,
		Chasing,
		SightChasing,
	};

	void Init()override;
	void Update()override;
	void PostUpdate()override;

	void DrawUnLit()override;

	void SetBulletParam(float _aliveTime,int _damage, float _range, const Math::Vector3& _startPos, const Math::Vector3 direction,float _speed,float _dampingInterval,float _dampingRate);
	void SetBulletTrail(const std::string& _path, const Math::Color _color, float _width, UINT _length);
	void SetBulletType(const moveType _type,const std::weak_ptr<CharacterBase>& _chasigTarget);
	void SetChasingData(int rotateSpeedDeg,float lockAngle,float lostTime,float trackingDistance );

	void OnHit()override;



private:

	
	void MoveSight();
	void MoveChasing();

	void Intersects();

	bool Ray(const Math::Vector3& pos, const Math::Vector3& vec, float length);

	float m_aliveTime = 0;

	float m_range = 0.0f;
	Math::Vector3 m_direction = {};
	float m_speed = 0.0f;
	float m_dampingInterval = 0.0f;
	float m_dampingRate = 0.0f;
	Math::Vector3 m_startPos = {};

	//追尾用
	float m_durationChase = 0.0f;
	float   m_rotateSpeedDeg = 90.0f;
	float m_maxLockAngle = 150.0f;
	float m_durationLost = 0.0f;
	float m_lockLostTime = 1.0f;
	float m_turnSpeed = 20.0f;
	float m_trackingEndDistance = 3000.0f;

	float m_durationSight = 0.0f;


	std::weak_ptr<CharacterBase> m_wpTarget;

	std::shared_ptr<KdTrailPolygon> m_trail = nullptr;

	moveType						m_moveType = Sight;

};
