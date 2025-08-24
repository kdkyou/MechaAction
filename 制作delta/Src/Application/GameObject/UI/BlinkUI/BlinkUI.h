#pragma once

#include "../UIBase.h"

class BlinkUI :public UIBase
{
public:
	void Init()override;

	void Update()override;

	void Editor_ImGui()override;

	virtual void Deserialize(const nlohmann::json& jsonObj)
	{
		UIBase::Deserialize(jsonObj);
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const
	{
		UIBase::Serialize(outJson);
	}


	void DrawSprite()override;

private:

	float m_alpha = 1.0f;

	float m_WorstAlpha = 0.4f;

	float m_time = 0.5f;
	float m_durationTime = 0.0f;
	int   m_plamai = 1;

};