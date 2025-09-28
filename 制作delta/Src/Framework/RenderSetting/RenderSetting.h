#pragma once

class RenderSetting
{
public:

	void Editor_ImGui();

	void RenderLoad(const std::string& path);

	const bool IsAlphaDither()const { return m_alphaDither; }
	void SetAlpha(const bool alpha) { m_alphaDither = alpha; }

private:

	void Deserialize(const nlohmann::json& jsonObj);
	
	bool m_alphaDither = true;

	Math::Vector3 m_dirVec;
	Math::Vector3 m_dirColor;

	Math::Color	  m_ambinetColor;


	bool 		  m_fogWidth = false;
	bool 		  m_fogHeight = false;
	Math::Vector3 m_widthColor;
	Math::Vector3 m_heightColor;

	Math::Vector2 m_distance;

	float m_bloom = 1.0f;

	float m_bgmVal = 0.2f;
	float m_seVal = 0.3f;

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