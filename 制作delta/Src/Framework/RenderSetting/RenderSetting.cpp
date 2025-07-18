#include "RenderSetting.h"

void RenderSetting::Editor_ImGui()
{
	if (ImGui::DragFloat3("dirVec", &m_dirVec.x, 0.01f, -1.0f, 1.0f))
	{
		KdShaderManager::Instance().WorkAmbientController().SetDirLight(m_dirVec, m_dirColor);
	}
	if (ImGui::DragFloat3("dirColor", &m_dirColor.x, 0.01f, -1.0f, 1.0f))
	{
		KdShaderManager::Instance().WorkAmbientController().SetDirLight(m_dirVec,m_dirColor);
	}


	if(ImGui::DragFloat4("AmbientColor", &m_ambinetColor.x, 0.01f, 0.0f, 1.0f))
	{
		KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(m_ambinetColor);
	}

}
