#pragma once

#include "../../Effect/Effect.h"

class Alert :public Effect
{
public:

	void Init()override;
	void Update()override;
	void PostUpdate()override;

	void DrawUnLit()override;
	void DrawSprite()override;

	void CalcPos(const Math::Vector2& dir2D);

private:

	enum AlertP
	{
		None,
		Left,
		Right,
		Up,
		Down,
	};

	AlertP m_nowDraw = None;

	float m_durationAlive = 0.0f;

	float m_angle = 0.0f;


};