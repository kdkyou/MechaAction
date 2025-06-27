#pragma once


class CameraBase;

class Character : public KdGameObject
{
public:
	Character() {}
	~Character()			override {}

	void Init()				override;
	void Update()			override;
	void PostUpdate()		override;
	void GenerateDepthMapFromLight() override;
	void DrawLit()			override;
	void DrawUnLit()		override;

	void SetCamera(const std::shared_ptr<CameraBase>& camera)
	{
		m_wpCamera = camera;
	}

	void RegistHitObject(const std::shared_ptr<KdGameObject>& object)
	{
		m_wpHitObjectList.push_back(object);
	}

	const std::weak_ptr<KdModelWork>GetModelWork()const;

	void SetThis(const std::shared_ptr<Character>& _this) { m_wpThis = _this; }

private:

	void Editor_ImGui()override;

	// WASDの入力があったかどうか
	const bool IsMove();

	// SHIFTの入力があったか
	const bool IsBoost();

	// 左クリック入力
	const bool IsAttack();

	// スペース入力
	const bool IsFlow();
	// 右クリック入力
	const bool IsGuard();

	//				移動量　　方向		当たり判定するタイプ（基本地面） キャラが回転するか　レイに補正をかけるか
	bool Move(float speed, const Math::Vector3& dir, const KdCollider::Type type, bool ray = false, bool camera = true, bool step = false);
	//bool Move(float speed,const Math::Vector3& dir,const Math::Vector3& step={}, const KdCollider::Type type=KdCollider::TypeGround, bool ray = true, bool sphere = true, bool camera = true);

	// キャラクターの回転行列を作成する
	void UpdateRotate(const Math::Vector3& srcMoveVec);
	// 衝突判定とそれに伴う座標の更新
	void UpdateCollision();

	bool  RayCast(const Math::Vector3& startPos, const Math::Vector3& vec, const float length, const KdCollider::Type& type, Math::Vector3& resultPos);

	bool  SphereCast(const Math::Vector3& pos, const Math::Vector3& vec, const float radius, const KdCollider::Type& type, Math::Vector3& resultPos);

	std::shared_ptr<KdModelWork>				m_spModel = nullptr;
	std::shared_ptr<KdAnimator>					m_spAnimator = nullptr;

	std::weak_ptr<CameraBase>					m_wpCamera;
	std::vector<std::weak_ptr<KdGameObject>>	m_wpHitObjectList{};

	Math::Vector3								m_worldRot;

	float										m_clampSize = 10.0f;

	Math::Matrix 								m_scale;

	float										m_gravity = 0;
	const float									m_gravityPow = 9.16f;

	bool										m_isGround = false;

	
	void OverTrans(const std::string& nowAnimName);


	//パラメータ
	Math::Vector3								m_stepHigh = { 0.0f,0.2f,0.0f };
	float										m_stopSpeed = 0.0f;
	float										m_walkSpeed = 10.0f;
	float										m_jumpSpeed = 15.0f;
	float										m_boostSpeed = 120.0f;
	float										m_boostEndSpeed = 20.0f;
	float										m_boostDushSpeed = 100.0f;
	float										m_bladeAttackSpeed = 200.0f;
	float										m_hitedSpeed = 15.0f;

	Math::Color color = { 0,1,0,1 };

	bool									    m_transAC = false;

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

	protected:

		const Math::Vector3 Direct(std::weak_ptr<Character>& owner, bool isCamera);

		bool m_isBoost = false;
		bool m_isMove = false;
		bool m_isRightAttack = false;
		bool m_isFlow = false;
		bool m_isGuard =false;

		std::string		m_animName;

		float m_speed = 0.0f;
		Math::Vector3 m_direction = Math::Vector3::Zero;

		struct Effect
		{
			std::string name;
			std::weak_ptr<KdEffekseerObject> wpEffect;
			Math::Matrix pNodeMat;
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
	};

	class ActionStandUp :public ActionStateBase
	{
	public:
		virtual ~ActionStandUp()  override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	};

	class ActionJump :public ActionStateBase
	{
	public:
		virtual ~ActionJump() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	};

	//
	class ActionStandShield : public ActionStateBase
	{
	public:
		virtual ~ActionStandShield() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	};

	class ActionJumpShield :public ActionStateBase
	{

		virtual ~ActionJumpShield() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
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

	//未
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

	class ActionShield :public ActionStateBase
	{
	public:
		virtual ~ActionShield() override {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void PostUpdate(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
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
};