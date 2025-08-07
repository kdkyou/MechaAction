#pragma once

#include "../UIBase.h"

class DrawUI :public UIBase
{
public:

	void Init()override;

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
};