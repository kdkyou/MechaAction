#include "GuageUI.h"

void GuageUI::Init()
{
	m_name = "GuageUI";

}

void GuageUI::Update()
{

}

void GuageUI::Editor_ImGui()
{
	UIBase::Editor_ImGui();
}

void GuageUI::Deserialize(const nlohmann::json& jsonObj)
{
	UIBase::Deserialize(jsonObj);

}

void GuageUI::Serialize(nlohmann::json& outJson) const
{
	UIBase::Serialize(outJson);
}


