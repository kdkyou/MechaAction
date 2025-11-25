#pragma once

#include"../BaseScene/BaseScene.h"

class KdSoundInstance;

class TrainingScene : public BaseScene
{
public:

	TrainingScene() { Init(); }
	~TrainingScene() {}

private:

	enum TrainingState
	{
		None,
		WASD,
		Space,
		Shift,
		EnemyLock,
		RightAttack,
		LeftAttack,
		RSAttack,
		LSAttack,
		Clear,
	};

	void Event() override;
	void Init() override;

	float m_duration = 1.0f;
	bool m_fade = false;

	TrainingState m_trainingState = TrainingState::None;

	std::shared_ptr<KdSoundInstance> m_voice = nullptr;

};
