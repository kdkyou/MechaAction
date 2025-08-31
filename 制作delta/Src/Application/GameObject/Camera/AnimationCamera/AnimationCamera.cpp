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

void AnimationCamera::Editor_ImGui()
{
	CameraBase::Editor_ImGui();

	ImGui::DragFloat3()
}

void AnimationCamera::Deserialize(const nlohmann::json& jsonObj)
{
}

void AnimationCamera::Serialize(nlohmann::json& outJson) const
{
}
