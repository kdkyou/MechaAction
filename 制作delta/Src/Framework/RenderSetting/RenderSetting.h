#pragma once

class RenderSetting
{
public:

	void Editor_ImGui();

	void RenderLoad(const std::string& path);

private:

	void Deserialize(const nlohmann::json& jsonObj);
	

	Math::Vector3 m_dirVec;
	Math::Vector3 m_dirColor;

	Math::Color	  m_ambinetColor;


	bool 		  m_fogWidth = false;
	bool 		  m_fogHeight = false;
	Math::Vector3 m_widthColor;
	Math::Vector3 m_heightColor;

	Math::Vector2 m_distance;

	float m_bloom = 1.0f;

	RenderSetting()
	{

	}

public:
	static RenderSetting& GetInstance()
	{
		static RenderSetting instance;
		return instance;
	}
};