#pragma once

#include"CharacterBase.h"

class CameraBase;

#define NORMAL_SPEED 1.0f
#define TRANS_SPEED 1.5f
#define INNER_LENGTH 11.0f
#define DENGERLINE 0.3f

class Character : public CharacterBase
{
public:
	Character() {}
	~Character()			override {}

	void Init()				override;
	void Update()			override;
	void PostUpdate()		override;
	void DrawLit()			override;
	void DrawUnLit()		override;
	void DrawParticle()		override;

	void SetThis(const std::shared_ptr<Character>& _this) { m_wpThis = _this; }

	void OnHit()override;

	void ResetPosition() override;

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

private:

	
	// WASDの入力があったかどうか
	const bool IsMove();

	// SHIFTの入力があったか
	const bool IsBoost();

	// 右クリック入力
	const bool IsAttack();

	// スペース入力
	const bool IsFlow();
	// 左クリック入力
	const bool IsGuard();

	const bool IsLeftShoulder();
	const bool IsRightShoulder();

	// スティックの押し込み
	const bool IsRStick();
	const bool IsLStick();


	// キャラクターの回転行列を作成する
	void UpdateRotate(const Math::Vector3& srcMoveVec) override;
	// 衝突判定とそれに伴う座標の更新
	void UpdateCollision()override;

	void LockOn();
	// LockOn を間引くためのフラグ/タイマー（LockOn の頻度制御）
	void LockOn(bool force /*= false*/);
	
	// Auto-aim smoothing: desiredDir を渡して毎フレーム呼び、滑らかな向きを返す
	const Math::Vector3& GetSmoothedAimDir(const Math::Vector3 & desiredDir, float dt);
	
	void ResetGravity() { m_gravity = 0.0f; }

	bool IsIgnoreGravityState()const;

	bool IsBoostState()const;

	void ResetPrevMove() { m_preMove = m_vMove; }

	void WalkSounds();

	void CreatePolygon();

	enum CharacterStateName
	{
		Start,
		Stand,
		StandUp,
		StandGuard,
		Walk,
		WalkGuard,
		Fly,
		Fall,
		FlyGuard,
		FallGuard,
		Boost,
		BoostNow,
		BoostEnd,
		BoostDush,
		BoostDushGuard,
		BoostFloat,
		BoostFloatGuard,
		BoostFall,
		DushEnd,
		RightSorwdBef,
		RightSorwdMid,
		RightSorwdAf,
		RightSorwdSeco,
		RightSorwdCharge,
		LeftShoulderAttack,
		Hited,
		Destoryed,
	};

	const bool SwordRangeCheck();
	
	void OverTrans(const std::string& nowAnimName,const float animProgress);

	Math::Vector3								m_stepHigh = { 0.0f,0.2f,0.0f };

	//パラメータ
	float										m_speedMag = NORMAL_SPEED;	//スピードの掛け算
	float										m_stopSpeed = 0.0f;
	float										m_walkSpeed = 65.0f;
	float										m_jumpSpeed = 40.0f;
	float										m_boostSpeed = 340.0f;
	float										m_boostEndSpeed = 180.0f;
	float										m_boostDushSpeed = 180.0f;
	float										m_bladeAttackSpeed = 240.0f;
	float										m_hitedSpeed = 100.0f;

	float                                       m_boostFloatMeg = 0.6f;
	float										m_boostLerpMeg = 0.2f;

	// LockOn 関連タイマー
	float m_lockOnFindTimer = 0.0f;
	float m_lockOnFindInterval = 0.12f; // 0.08～0.25 が目安

	// エイムの滑らかさと回転上限
	Math::Vector3 m_aimSmoothedDir = Math::Vector3::Backward;
	float m_aimSmoothSpeed = 12.0f;			// 追従速さ（調整）
	float m_aimMaxTurnDegPerSec = 720.0f;	// 1秒当たりの最大回転角（度）

	// デバッグ用
	Math::Color m_debugColor = { 0,1,0,1 };

	bool										m_isWalkSounds =true;
	bool										m_isParticle = false;
	



	//トレイル
	void InitTrail();
	bool EnableTrail();
	bool UnEnableTrail();
	bool AddTrail();

	std::string LEFTUP = "LeftUp";
	std::string LEFTDOWN = "LeftDown";
	std::string RIGHTUP = "RightUp";
	std::string RIGHTDOWN = "RightDown";

	struct TrailParam
	{
		std::string								name;
		std::shared_ptr<KdTrailPolygon>			trail;
		Math::Matrix							mat;
	};
	std::vector<std::shared_ptr<TrailParam>>								m_spTrails;


	std::weak_ptr<Character>					m_wpThis;

	//ステートパターン管理系!
private:

	class ActionStateBase
	{
	public:
		virtual ~ActionStateBase() {}

		virtual void Enter(std::weak_ptr<Character>& owner) {}
		virtual void Update(std::weak_ptr<Character>& owner) {}
		virtual void PostUpdate(std::weak_ptr<Character>& owner) {}
		virtual void Exit(std::weak_ptr<Character>& owner) {}

		void Checkkey(std::weak_ptr<Character>& owner);

		const std::string& GetName() { return m_animName; }
		const UINT GetState() const { return m_stateNum; }

		const UINT GetPrevMove()const { return m_prevType; }
		const UINT GetMove()const { return m_type; }

protected:

		const Math::Vector3 Direct(std::weak_ptr<Character>& owner, bool isCamera);

		void Trans(std::weak_ptr<Character>& owner,const float animProgress)const;

		bool IsBoostDush();

		void EffectUpdate(std::weak_ptr<Character>& owner);
		void EffectExit();

		bool m_isBoost = false;
		bool m_isMove = false;
		bool m_isFlow = false;
		bool m_isRightAttack = false;
		bool m_isGuard =false;
		bool m_isRightShoulder = false;
		bool m_isLeftShoulder = false;

		bool m_isDuration = false;

		bool m_isOneShot = false;

		float m_stiffnessTime = 0.0f;
		float m_durationStiffness = 0.0f;

		std::string		m_animName;
		UINT			m_stateNum = 0;
	

		float m_speed = 0.0f;
		Math::Vector3 m_direction = Math::Vector3::Zero;

		struct Effect
		{
			std::string name;
			std::weak_ptr<KdEffekseerObject> wpEffect;
			Effekseer::Handle handle = 0;
			Math::Matrix pNodeMat = Math::Matrix::Identity;
		};

		std::list<std::shared_ptr<Effect>> m_spEffects;

		std::weak_ptr<KdSoundInstance> m_wpRunSound;

		enum MoveType
		{
			NoMove,
			Right,
			Left,
			Back,
			Front,
		};

		MoveType m_type =NoMove;
		MoveType m_prevType = NoMove;

	};

	class ActionStart :public ActionStateBase
	{
	public:
		virtual ~ActionStart() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:

	};

	class ActionIdle :public ActionStateBase
	{
	public:
		virtual ~ActionIdle() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionStandUp :public ActionStateBase
	{
	public:
		virtual ~ActionStandUp()  override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionJump :public ActionStateBase
	{
	public:
		virtual ~ActionJump() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionFall :public ActionStateBase
	{
	public:
		virtual ~ActionFall() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionStandShield : public ActionStateBase
	{
	public:
		virtual ~ActionStandShield() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionJumpShield :public ActionStateBase
	{
	public:
		virtual ~ActionJumpShield() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionMove :public ActionStateBase
	{
	public:
		virtual ~ActionMove()  override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;


	private:
	};

	class ActionMoveShield : public ActionStateBase
	{
	public:
		virtual ~ActionMoveShield() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionBoost :public ActionStateBase
	{
	public:
		virtual ~ActionBoost() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;

	private:

	};

	class ActionBoostNow :public ActionStateBase
	{
	public:
		virtual ~ActionBoostNow()  override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionBoostEnd :public ActionStateBase
	{
	public:
		virtual ~ActionBoostEnd() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:

		float m_easeSpeed = 0.0f;

		KdEase m_ease;
	};

	class ActionBoostDush :public ActionStateBase
	{
	public:
		virtual ~ActionBoostDush() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:

};

	class ActionBoostDushEnd :public ActionStateBase
	{
	public:
		virtual ~ActionBoostDushEnd() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:

		float m_easeSpeed = 0.0f;

		KdEase m_ease;
	};

	class ActionBoostShield :public ActionStateBase
	{
	public:
		virtual ~ActionBoostShield() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionBoostFloat :public ActionStateBase
	{
	public:
		virtual ~ActionBoostFloat() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionBoostFloatShield :public ActionStateBase
	{
	public:
		virtual ~ActionBoostFloatShield() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionBoostFall :public ActionStateBase
	{
	public:
		virtual ~ActionBoostFall() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};
	
	class ActionBoostFallShield :public ActionStateBase
	{
	public:
		virtual ~ActionBoostFallShield() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};


	class ActionRightAttack :public ActionStateBase
	{
	public:
		virtual ~ActionRightAttack() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionRightAttackMid :public ActionStateBase
	{
	public:
		virtual ~ActionRightAttackMid()  override {}
		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionRightAttackAf :public ActionStateBase
	{
	public:
		virtual ~ActionRightAttackAf()  override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionRightAttackSecond :public ActionStateBase
	{
	public:
		virtual ~ActionRightAttackSecond()  override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionRightAttackCharge :public ActionStateBase
	{
	public:
		virtual ~ActionRightAttackCharge()  override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionLeftShoulderAttack :public ActionStateBase
	{
	public:
		virtual ~ActionLeftShoulderAttack()  override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:

	};


	class ActionHited :public ActionStateBase
	{
	public:
		virtual ~ActionHited() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionDestroyed :public ActionStateBase
	{
	public:
		virtual ~ActionDestroyed() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};



	void ChangeActionState(std::shared_ptr<ActionStateBase> nextAction);
	std::shared_ptr<ActionStateBase>		m_nowAction = nullptr;
	std::shared_ptr<ActionStateBase>		m_prvAction = nullptr;
};