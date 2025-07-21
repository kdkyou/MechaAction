#pragma once

#include"CharacterBase.h"

class CameraBase;

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

	void SetThis(const std::shared_ptr<Character>& _this) { m_wpThis = _this; }

	void OnHit()override;

private:

	void Editor_ImGui()override;

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

	//bool Move(float speed,const Math::Vector3& dir,const Math::Vector3& step={}, const KdCollider::Type type=KdCollider::TypeGround, bool ray = true, bool sphere = true, bool camera = true);

	// キャラクターの回転行列を作成する
	void UpdateRotate(const Math::Vector3& srcMoveVec) override;
	// 衝突判定とそれに伴う座標の更新
	void UpdateCollision()override;


	bool  SphereCast(const Math::Vector3& pos, const Math::Vector3& vec, const float radius, const KdCollider::Type& type, Math::Vector3& resultPos);

	void ResetGravity() { m_gravity = 0.0f; }

	bool IsIgnoreGravityState()const;

	enum CharacterStateName
	{
		Stand,
		StandUp,
		StandGuard,
		Walk,
		WalkGuard,
		Fly,
		FlyGuard,
		Boost,
		BoostNow,
		BoostEnd,
		BoostDush,
		BoostDushGuard,
		RightSorwdBef,
		RightSorwdMid,
		RightSorwdAf,
		LeftShoulderAttack,
		Hited,
		Destoryed,
	};


	
	void OverTrans(const std::string& nowAnimName,float animProgress);

	Math::Vector3								m_stepHigh = { 0.0f,0.2f,0.0f };

	//パラメータ
	float										m_speedMag = 1.0f;	//スピードの掛け算
	float										m_stopSpeed = 0.0f;
	float										m_walkSpeed = 10.0f;
	float										m_jumpSpeed = 15.0f;
	float										m_boostSpeed = 120.0f;
	float										m_boostEndSpeed = 20.0f;
	float										m_boostDushSpeed = 100.0f;
	float										m_bladeAttackSpeed = 200.0f;
	float										m_hitedSpeed = 15.0f;

	// デバッグ用
	Math::Color color = { 0,1,0,1 };

	Math::Vector3 m_limColor = { 0.19f,0.09f,0.09f };
	float m_limPow = 8.0f;

	bool									    m_transAC = false;

	bool m_limEnable = false;

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


	Math::Vector3 m_vMove = Math::Vector3::Zero;

	std::weak_ptr<Character>					m_wpThis;

	std::weak_ptr<KdGameObject>					m_wpRockTarget;




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
		const UINT GetState() { return m_stateNum; }

protected:

		const Math::Vector3 Direct(std::weak_ptr<Character>& owner, bool isCamera);

		void Trans(std::weak_ptr<Character>& owner,float animProgress);


		void EffectUpdate(std::weak_ptr<Character>& owner);
		void EffectExit();

		bool m_isBoost = false;
		bool m_isMove = false;
		bool m_isFlow = false;
		bool m_isRightAttack = false;
		bool m_isGuard =false;
		bool m_isRightShoulder = false;
		bool m_isLeftShoulder = false;

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
	};

	class ActionMoveShield : public ActionStateBase
	{
	public:
		virtual ~ActionMoveShield() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
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
	const std::weak_ptr<ActionStateBase>& GetPrvAction() { return m_prvAction; }
	std::shared_ptr<ActionStateBase>		m_nowAction = nullptr;
	std::shared_ptr<ActionStateBase>		m_prvAction = nullptr;
};