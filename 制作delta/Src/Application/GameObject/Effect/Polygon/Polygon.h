#pragma once

#include "../Effect.h"

class Polygon :public Effect
{
public:

	enum effectDraw
	{
		eUnLit,
		eUnAdd,
		eBright,
		eBriAdd,
	};

	void Init()override;

	void Update()override;

	void DrawUnLit()override;
	void DrawBright()override;

	virtual void Editor_ImGui()override;

	// 
	void SetParam(const std::string& texPath, float alive, effectDraw type, bool isalive);


private:

	

	// 各種フラグ予定
	effectDraw m_type = eUnLit;

	bool m_isAlive = false;

};