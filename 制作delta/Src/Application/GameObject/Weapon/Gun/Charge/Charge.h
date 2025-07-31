#pragma once

#include"../GunBase.h"

class Charge :public GunBase
{
public:
	void Init()override;
	void Update()override;
	void PostUpdate()override;


	void Editor_ImGui()override;

private:

	void Trigger() override;
	void Shot() override;
	void ShotCharge();
	void OnTrigger()override;

	std::string m_chargeSoundPath;

	std::weak_ptr<KdSoundInstance> m_sounds;

};