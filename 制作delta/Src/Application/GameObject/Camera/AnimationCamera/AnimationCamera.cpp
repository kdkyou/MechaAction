#include "AnimationCamera.h"

void AnimationCamera::Init()
{
	m_name = "Animation";
}

void AnimationCamera::Update()
{
	
}

void AnimationCamera::PostUpdate()
{
	CameraBase::PostUpdate();
}

void AnimationCamera::AddMatrix(const std::vector<Math::Matrix>& mats)
{
	m_mats = mats;
}
