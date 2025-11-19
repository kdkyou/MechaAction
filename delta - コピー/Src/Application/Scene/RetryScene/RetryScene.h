#pragma once

#include "../BaseScene/BaseScene.h"

class RetryScene :public BaseScene
{
public :

	RetryScene() { Init(); }
	~RetryScene(){}

private:

	void Init()override;
	void Event()override;

};