#pragma once

#include"../CameraBase.h"

class AnimationCamera :public CameraBase
{
public:

	void Init()override;
	void Update()override;
	void PostUpdate()override;


private:


};