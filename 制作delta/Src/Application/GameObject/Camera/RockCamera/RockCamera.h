#pragma once
#include"../CameraBase.h"

class RockCamera : public CameraBase
{
public:
	RockCamera() {}
	~RockCamera()			override {}

	void Init()				override;
	void PostUpdate()		override;
	
private:

	void Rock();
	
};