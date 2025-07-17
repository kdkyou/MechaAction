#pragma once

#include"../GunBase.h"

class CharacterBase;

class Rifle :public GunBase
{
public :
	void Init()override;
	void Update()override;


	void Editor_ImGui()override;

private:

	void Trigger() override;
	void Shot() override;
	void OnTrigger()override;

};