#pragma once

#include"../CameraBase.h"

class AnimationCamera :public CameraBase
{
public:

	void Init()override;
	void Update()override;
	void PostUpdate()override;

	void AddMatrix(const std::vector<Math::Matrix>& mats);

private:

	
	std::vector<Math::Matrix> m_mats;

	

};