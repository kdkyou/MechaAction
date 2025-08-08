#pragma once

#include "../UIBase.h"

class GuageUI :public UIBase
{
public:
	void Init()override;

	void Update()override;

	void Editor_ImGui()override;

	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

//	void DrawSprite()override;

	void SetBaseNum(const int num) { m_baseNum = num; }
	void SetIndeNum(const int num) { m_indeNum = num; }

private:

	int m_baseNum;
	int m_indeNum;

};