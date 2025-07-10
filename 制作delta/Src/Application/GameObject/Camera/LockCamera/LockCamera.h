#pragma once
#include"../CameraBase.h"

class LockCamera : public CameraBase
{
public:
	LockCamera() {}
	~LockCamera()			override {}

	void Init()				override;
	void PostUpdate()		override;

private:

	void Lock();

};