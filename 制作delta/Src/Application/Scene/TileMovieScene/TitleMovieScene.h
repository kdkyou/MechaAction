#pragma once

#include "../BaseScene/BaseScene.h"

class TitleMovieScene :public BaseScene
{
public:

	TitleMovieScene() { Init(); }
	~TitleMovieScene() {}

private:

	void Init()override;
	void Event()override;

};