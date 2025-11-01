#pragma once

#include "../WeaponBase.h"

class GunBase :public WeaponBase
{
public:
	void Init()override;

	void PreUpdate()override;

	void Update()override;
	void PostUpdate()override;

	void TriggerLock(bool flg) { m_trigger = flg; }

	void SetGunsParam(const std::string& gunModelPath, float fireRate, float coolTime,float burst,int burstNum, int maxNumOnes, int maxNum);
	// モデル生成後にすること
	void MakeAnimator(const std::string& animation, float compSpeed, bool loop = false);
	void SetBulletsParam(const std::string& bulletModelPath, float aliveTime, int damage, float speed, float range, float dampingInterval, float dampingRate);
	void SetBulletsTrailParam(const std::string& bulletTrailPath,const Math::Color& color,float width,UINT length);
	void SetBulletChaisingData(float bulletRotateAngle, float bulletLockAngle, float bulletLostTime, float bulletTrackingDistance);
	void SetShotSoundPath(const std::string& path) { m_shotSoundPath = path; }

	bool SetNodeMats(const std::string& nodeName);
	

	void Editor_ImGui()override;

	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;
	

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;
	
	const float GetReloadTime()const { return m_reloadTime; }
	const float GetReloadDuration()const { return m_durationReload; }

	const int GetRemainingNumber()const { return m_num; }
	const int GetRemainingOnce()const { return m_numOnce; }
	const int GetNumberOnce()const { return m_maxNumofOnce; }

protected:

	// 目的の対象に銃口を向ける //xとyのみ考える  targetDirはローカル空間に
	const Math::Matrix RotateWeaponDirect(const Math::Vector2& enableAngle, const Math::Vector3& targetDir,const Math::Matrix& rotMat);


	virtual void Trigger(){}
	virtual void Shot(){}
	bool Flash(const Math::Matrix& occurMat);

	virtual void OnTrigger(){}

	virtual void NoNum(){}

	
	bool					 m_trigger = false;

	float					 m_fireRateAccel = 1.0f;
	float					 m_fireRate = 0.0f;		// 何秒毎に弾を発射させるか
	float					 m_durationFire = 0.0f;

	bool					 m_isReload = false;			// リロードするか
	float					 m_reloadAccel = 1.0f;			// リロード加速度
	float					 m_reloadTime = 0.0f;	// 何秒のリロードか
	float					 m_durationReload = 0.0f;	// リロード進行度

	float					 m_burstAccel = 1.0f;		// バースト加速度
	float					 m_durationBurst = 0.0f;	// バースト進行度
	float					 m_burst = 0.0f;			// バースト

	int						 m_numBurst = 0;		// 何発連続か
	int						 m_num = 0;				// 総残弾
	int						 m_numOnce = 0;			// 一度の残弾
	int						 m_maxNum = 0;			// 総弾数
	int						 m_maxNumofOnce = 0;	// 一度の弾倉

	int						 m_numShot = 0;			// 発射口カウント

	struct ShotNode
	{
		std::string name;
		Math::Matrix matrix;
	};
	std::vector<std::shared_ptr<ShotNode>> m_nodeMats;			// 発射口行列

	std::string				 m_muzzleFlashPath = "";	// マズルフラッシュ用テクスチャパス

	// 弾本体の情報
	std::string				 m_bulletModelPath;
	float					 m_aliveTime =0.0f;
	float					 m_speed = 0.0f;
	float					 m_range = 0.0f;
	float					 m_dampingInterval = 0.0f;
	float					 m_dampingRate = 0.0f;

	// 弾のトレイル関係
	std::string				 m_bulletTrailPath;
	Math::Color				 m_bulletTrailColor = {0.3f,0.1f,0.1f};
	float					 m_bulletTrailWidth = 0.3f;
	UINT					 m_bulletTrailLength = 10;

	// 追尾式の情報
	float m_bulletRotateDeg = 0.0f;
	float m_bulletLockAngle = 0.0f;
	float m_bulletLostTime = 0.0f;
	float m_bulletTrackingDistance = 0.0f;

	// サウンド関連
	std::string				m_shotSoundPath;
};