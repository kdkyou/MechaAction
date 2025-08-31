#pragma once

#include "../BaseScene/BaseScene.h"

class TitleMovieScene :public BaseScene
{
public:

	TitleMovieScene() { Init(); }
	~TitleMovieScene() {}

private:

	float duration = 8.0f;

	void Init()override;
	void Event()override;

	bool m_isFirstChange = false;
	bool m_isSecondChange = false;

};