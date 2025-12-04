#pragma once

#include "../BaseScene/BaseScene.h"

class GameMovieScene :public BaseScene
{
public:

	GameMovieScene() { Init(); }
	~GameMovieScene() {}

private:

	void Event() override;
	void Init() override;

};