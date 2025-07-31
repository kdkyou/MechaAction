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

	static bool fogheight = false;
	static bool fogwidth = false;

	static Math::Vector3 heightColor;
	static float		 height = 0.001f;
	static Math::Vector3 widthColor;
	static Math::Vector2 width;
	static float         dist = 0.001f;

	ImGui::Checkbox("Fogheight", &fogheight);
	ImGui::Checkbox("Fogdist", &fogwidth);
	
	KdShaderManager::Instance().WorkAmbientController().SetFogEnable(fogwidth,fogheight);
	
	if (fogwidth)
	{

		ImGui::DragFloat3("widthColor",&widthColor.x,0.01f,0.0f,1.0f);
		ImGui::DragFloat("deninty",&height, 0.01f, 0.0f);
		KdShaderManager::Instance().WorkAmbientController().SetDistanceFog(widthColor,height);
	}

	if (fogheight)
	{
		ImGui::DragFloat2("height",&width.x, 0.01f, 0.0f);
		ImGui::DragFloat("dist", &dist, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat3("HeightColor",&heightColor.x, 0.01f, 0.0f, 1.0f);
		KdShaderManager::Instance().WorkAmbientController().SetheightFog(heightColor,width.y,width.x,dist);
	}


}
