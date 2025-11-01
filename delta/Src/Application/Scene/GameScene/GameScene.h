#pragma once

#include"../BaseScene/BaseScene.h"

class GameScene : public BaseScene
{
public :

	GameScene()		{ Init(); }
	~GameScene()	{}

private:

	enum Wave
	{
		Start,
		First,
		Second,
		Last,
		Complete,
	};

	void Event() override;
	void Init() override;

	// 乱数生成器
	std::shared_ptr<KdRandomGenerator> m_RandomGen;

	Wave	m_waveProgress = Start;

	float m_duration = 1.0f;
	bool m_fade = false;


};
