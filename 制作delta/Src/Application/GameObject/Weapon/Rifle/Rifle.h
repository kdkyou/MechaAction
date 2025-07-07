#pragma once

#include"../WeaponBase.h"

class CharacterBase;

class Rifle :public WeaponBase
{
public :



	void Init()override;
	void Update()override;

	void TriggerRock(bool flg) { m_trigger = flg; }

	void SetGunsParam(const std::string gunModelPath,float fireRate, float coolTime, int maxNumOnes, int maxNum);
	void SetBulletsParam(const std::string& bulletModelPath, const std::string& bulletTrailPath, int damage,float speed, float range, float dampingInterval, float dampingRate);

	void Editor_ImGui()override;

private:

	void Trigger();
	void Shot();

	
	bool					 m_trigger = false;

	float					 m_fireRateAccel = 1.0f; 
	float					 m_fireRate = 0.0f;		// 何秒毎に弾を発射させるか
	float					 m_durationFire = 0.0f;		
	
	bool					 m_isReload = false;			// リロードするか
	float					 m_reloadAccel = 1.0f;			// リロード加速度
	float					 m_reloadTime = 0.0f;	// 何秒のリロードか
	float					 m_durationReload = 0.0f;	// リロード進行度
	
	int						 m_num = 0;
	int						 m_numOnce = 0;
	int						 m_maxNumofOnce = 0;
	int						 m_maxNum = 0;

	std::string				 m_bulletModelPath;
	std::string				 m_bulletTrailPath;
	int						 m_damage = 0;
	float					 m_speed = 0.0f;
	float					 m_range = 0.0f;
	float					 m_dampingInterval = 0.0f;
	float					 m_dampingRate = 0.0f;

};