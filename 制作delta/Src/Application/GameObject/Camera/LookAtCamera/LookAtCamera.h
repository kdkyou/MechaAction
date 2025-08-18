#pragma once

#include "../CameraBase.h"

class LookAtCamera : public CameraBase
{
public:

	void Init()override;

	void PostUpdate()override;

	

private:

	void Look();
};