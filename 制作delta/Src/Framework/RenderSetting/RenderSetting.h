#pragma once

class RenderSetting
{
public:

	Math::Vector3 m_dirVec;
	Math::Vector3 m_dirColor;

	Math::Color	  m_ambinetColor;
	
	Math::Vector3 m_;

	int 		  m_enableFog;
	Math::Vector3 m_fogColor;

	Math::Vector2 m_distance;

	float m_bloom = 0.0f;

	void Editor_ImGui();


private:

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