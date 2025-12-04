#pragma once

#include "../UIBase.h"

class DrawUI :public UIBase
{
public:

	void Init()override;

	void Editor_ImGui()override;

	void Update()override;


	virtual void Deserialize(const nlohmann::json& jsonObj)
	{
		UIBase::Deserialize(jsonObj);
		KdJsonUtility::GetValue(jsonObj,"Add",&m_isAdd);
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const
	{
		UIBase::Serialize(outJson);
		
		outJson["Add"] = m_isAdd;
	}
	

	void DrawSprite()override;

private:

	bool m_isAdd = false;
};