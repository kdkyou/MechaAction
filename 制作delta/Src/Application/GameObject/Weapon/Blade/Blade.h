#pragma once

#include"../WeaponBase.h"

class Character;

class Blade :public WeaponBase
{
public:
	void Init()override;
	void Update()override;

	void SetParent(std::weak_ptr<Character>_parent);

	void DrawBright()override;
private:
	
	std::weak_ptr<Character> m_wpParent;
};