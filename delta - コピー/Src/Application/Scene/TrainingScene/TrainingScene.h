#pragma once

#include"../BaseScene/BaseScene.h"

class TrainingScene : public BaseScene
{
public:

	TrainingScene() { Init(); }
	~TrainingScene() {}

private:

	void Event() override;
	void Init() override;

	float m_duration = 1.0f;
	bool m_fade = false;


};
