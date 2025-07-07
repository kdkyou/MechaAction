#pragma once

#include"../WeaponBase.h"

class Shield :public WeaponBase
{
public:
	void Init()override;
	void Update()override;
	void PostUpdate()override;

	
	void DrawBright()override;

	void SetModel(const std::string& path)override;

private:

};