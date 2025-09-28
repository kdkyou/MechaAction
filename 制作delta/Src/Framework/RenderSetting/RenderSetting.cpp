#include "RenderSetting.h"

#include "../../Application/Scene/SceneManager.h"

void RenderSetting::Editor_ImGui()
{
	static std::string fileName = "";

	ImGui::InputText((const char*)u8"ファイル名", &fileName);

	nlohmann::json outJson;
	if (ImGui::Button((const char*)u8"シーン保存"))
	{

		{
			nlohmann::json json;
			json["DirVec"] = KdJsonUtility::CreateArray(&m_dirVec.x, 3);
			json["DirColor"] = KdJsonUtility::CreateArray(&m_dirColor.x, 3);
			json["AmbientColor"] = KdJsonUtility::CreateArray(&m_ambinetColor.x, 4);
			json["FogWidth"] = m_fogWidth;
			json["FogHeight"] = m_fogHeight;
			json["Bloom"] = m_bloom;
			outJson.push_back(json);
		}

		std::string str = "";
		auto sceneType = SceneManager::Instance().GetSceneType();
		switch (sceneType)
		{
		case SceneManager::SceneType::Title:
			str = "Asset/Data/Title.render";
			break;
		case SceneManager::SceneType::TitleMovie:
			str = "Asset/Data/TitleMovie.render";
			break;
		case SceneManager::SceneType::Game:
			str = "Asset/Data/Game.render";
			break;
		default:
			break;
		}

		if (fileName != "")
		{
			str = "Asset/Data/" + fileName;
		}



		std::ofstream ofs(str);
		if (ofs.is_open())
		{
			ofs << outJson.dump();
		}
	}

	if (ImGui::DragFloat3("dirVec", &m_dirVec.x, 0.01f, -1.0f, 1.0f))
	{
		KdShaderManager::Instance().WorkAmbientController().SetDirLight(m_dirVec, m_dirColor);
	}
	if (ImGui::DragFloat3("dirColor", &m_dirColor.x, 0.01f, -1.0f, 1.0f))
	{
		KdShaderManager::Instance().WorkAmbientController().SetDirLight(m_dirVec, m_dirColor);
	}


	if (ImGui::DragFloat4("AmbientColor", &m_ambinetColor.x, 0.01f, 0.0f, 1.0f))
	{
		KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(m_ambinetColor);
	}



	static Math::Vector3 heightColor;
	static float		 height = 0.001f;
	static Math::Vector3 widthColor;
	static Math::Vector2 width;
	static float         dist = 0.001f;

	ImGui::Checkbox((const char*)u8"高さフォグ", &m_fogHeight);
	ImGui::Checkbox((const char*)u8"距離フォグ", &m_fogWidth);

	KdShaderManager::Instance().WorkAmbientController().SetFogEnable(m_fogWidth, m_fogHeight);

	if (m_fogWidth)
	{

		ImGui::DragFloat3("widthColor", &widthColor.x, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("deninty", &height, 0.01f, 0.0f);
		KdShaderManager::Instance().WorkAmbientController().SetDistanceFog(widthColor, height);
	}

	if (m_fogHeight)
	{
		ImGui::DragFloat2("height", &width.x, 0.01f, 0.0f);
		ImGui::DragFloat("dist", &dist, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat3("Color", &heightColor.x, 0.01f, 0.0f, 1.0f);
		KdShaderManager::Instance().WorkAmbientController().SetheightFog(heightColor, width.y, width.x, dist);
	}

	if (ImGui::DragFloat((const char*)u8"ブルーム", &m_bloom, 0.01f, 0.0f, 1.0f))
	{
		KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(m_bloom);
	}

	if (ImGui::Checkbox((const char*)u8"アルファディザ", &m_alphaDither)){}
	
	if (ImGui::DragFloat((const char*)u8"BGM", &m_bgmVal,0.01f,0.0f,1.0f))
	{
		KdAudioManager::Instance().SetBGMVolume(m_bgmVal);
	}

	if (ImGui::DragFloat((const char*)u8"SE", &m_seVal, 0.01f, 0.0f, 1.0f))
	{
		KdAudioManager::Instance().SetSEVolume(m_seVal);
	}


}

void RenderSetting::RenderLoad(const std::string& path)
{
	if (path == "") { return; }

	std::ifstream ifs(path);
	if (ifs.is_open())
	{

		nlohmann::json j;
		ifs >> j;
		for (auto& json : j)
		{
			Deserialize(json);
		}
	}

}

void RenderSetting::Deserialize(const nlohmann::json& jsonObj)
{
		KdJsonUtility::GetArray(jsonObj, "DirVec", &m_dirVec.x, 3);
		KdJsonUtility::GetArray(jsonObj, "DirColor", &m_dirColor.x, 3);
		KdJsonUtility::GetArray(jsonObj, "AmbientColor", &m_ambinetColor.x, 4);
		KdJsonUtility::GetValue(jsonObj, "FogWidth", &m_fogWidth);
		KdJsonUtility::GetValue(jsonObj, "FogHeight", &m_fogHeight);
		KdJsonUtility::GetValue(jsonObj, "Bloom", &m_bloom);

		KdShaderManager::Instance().WorkAmbientController().SetDirLight(m_dirVec, m_dirColor);
		KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(m_ambinetColor);
		KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(m_bloom);

}