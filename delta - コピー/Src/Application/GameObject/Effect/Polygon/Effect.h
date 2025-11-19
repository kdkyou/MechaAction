#pragma once

class Effect :public KdGameObject
{
public:

	enum effectDraw
	{
		eUnLit,
		eUnAdd,
		eBright,
		eBriAdd,
	};

	void Update()override;

	void DrawUnLit()override;
	void DrawBright()override;

	void ImGuiProcess()override;

	// 
	void SetParam(const std::string& texPath, float alive, effectDraw type,bool isalive);


private:
	
	// 生存時間 
	float m_durationAlive = 0.0f;

	// 板ポリ本体
	std::shared_ptr<KdSquarePolygon> m_spPoly;

	// 各種フラグ予定
	effectDraw m_type = eUnLit;

	bool m_isAlive = false;

};