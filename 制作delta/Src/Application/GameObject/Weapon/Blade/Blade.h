#pragma once

#include"../WeaponBase.h"

class Character;

class Blade :public WeaponBase
{
public:
	void Init()override;
	void Update()override;

	void DrawBright()override;

	void SetModel(const std::string& path)override;
private:
	
	
};