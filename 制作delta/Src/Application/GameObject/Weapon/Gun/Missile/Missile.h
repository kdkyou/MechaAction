#pragma once

#include "../GunBase.h"

class Missile :public GunBase
{
public:
	void Init()override;
	void Update()override;
	void PostUpdate()override;

	void Editor_ImGui()override;


private:

	void Trigger() override;
	void Shot()override;
	void OnTrigger()override;

	


	KdRandomGenerator rund;
};