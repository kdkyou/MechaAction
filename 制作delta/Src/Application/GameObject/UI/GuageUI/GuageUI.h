#pragma once

#include "../UIBase.h"

class GuageUI :public UIBase
{
public:

	enum class FillDirection
	{
		LeftToRight,
		RightToLeft,
		BottomToTop,
		TopToBottom,
	};

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
	
	// 残弾割合を設定（0.0f〜1.0f）
	void SetRatio(float ratio) { m_ratio = std::clamp(ratio, 0.0f, 1.0f); }

private:

	void DrawGuage();

	FillDirection m_fillDir = FillDirection::TopToBottom;
	
	float m_ratio = 1.0f;
	float m_time = 0.0f;
	float m_duration = 0.0f;

	bool m_isIncDec = true; // true=Inc
	bool m_isAuto = false;

	int m_current;
	int m_max =1;

	bool m_isHorizontal = true;	// true 横方向ゲージ
	bool m_isReverse = false;	// true = 右からor 下から

};