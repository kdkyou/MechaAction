#pragma once

#include "../UIBase.h"

class GuageUI :public UIBase
{
public:
	void Init()override;

	void Update()override;
	void DrawSprite()override;

	void Editor_ImGui()override;

	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

	void SetValue(int current, int max) { m_current = current; m_max = max; }
	void SetHorizontal(bool horizontal) { m_isHorizontal = horizontal; }
	void SetReverse(bool reverse) { m_isReverse = reverse; }

private:

	void DrawGuage();

	int m_current;
	int m_max =1;

	bool m_isHorizontal = true;	// true 横方向ゲージ
	bool m_isReverse = false;	// true = 右からor 下から

};