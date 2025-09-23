#pragma once
#include "../UIBase.h"

class MapUI : public UIBase
{
public:

	MapUI(){}
	~MapUI(){}

	void Init()override;

	void Update()override;

	virtual void Editor_ImGui()override;

	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override
	{
		UIBase::Deserialize(jsonObj);
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override
	{
		UIBase::Serialize(outJson);
	}

private:
};