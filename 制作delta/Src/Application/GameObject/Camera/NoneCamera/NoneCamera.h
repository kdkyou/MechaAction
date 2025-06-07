#pragma once

#include"../CameraBase.h"

class NoneCamera :public CameraBase
{
public:

	void Init()override;
	void PostUpdate()override;

private:

};