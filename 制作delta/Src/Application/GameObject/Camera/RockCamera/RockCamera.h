#pragma once
#include"../CameraBase.h"

class RockCamera : public CameraBase
{
public:
	RockCamera() {}
	~RockCamera()			override {}

	void Init()				override;
	void PostUpdate()		override;
	
	void SetRock(const std::shared_ptr<KdGameObject>& rockTarget) { 
		m_wpRockTarget = rockTarget; }

private:

	void Rock();

	std::weak_ptr<KdGameObject> m_wpRockTarget;

};