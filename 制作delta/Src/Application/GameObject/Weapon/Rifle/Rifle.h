#pragma once

#include"../WeaponBase.h"

class Enemy;

class Rifle :public WeaponBase
{
public :

	void Init()override;
	void Update()override;

	void Trigger();
	void Shot();

private:

	std::weak_ptr<Enemy> m_wpParent;

	int					 m_fireRate;
	int					 m_coolTime;
	int					 m_maxNumofOnce;


};