#pragma once

#include"../WeaponBase.h"

class Enemy;

class Rifle :public WeaponBase
{
public :



	void Init()override;
	void Update()override;

	void TriggerRock(bool flg) { m_trigger = flg; }

	void SetGunsParam(const std::string gunModelPath, int damage, int fireRate, int coolTime, int maxNumOnes, int maxNum);

private:

	void Trigger();
	void Shot();

	//std::weak_ptr<Enemy> m_wpParent;

	bool					 m_trigger = true;

	int						 m_damage = 0;
	
	float					 m_fireRate = 0.0f;		// 何秒毎に弾を発射させるか
	float					 m_durationFire = 0.0f;		
	
	float					 m_reloadTime = 0.0f;	// リロード時間
	float					 m_durationReload = 0.0f;		
	
	int						 m_num = 0;
	int						 m_numOnce = 0;
	int						 m_maxNumofOnce = 0;
	int						 m_maxNum = 0;


};