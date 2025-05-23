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
	void DrawBright()override;

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

	//WASDの入力があったかどうか
	const bool IsMove();

	//左SHIFTの入力があったか
	const bool IsBoost();

	//左クリック入力
	const bool IsLeftAttack();

	//スペース入力
	const bool IsFlow();

	bool Move(float speed,const Math::Vector3& dir,const KdCollider::Type type ,bool ray = true,bool camera=true );

	// キャラクターの回転行列を作成する
	void UpdateRotate(const Math::Vector3& srcMoveVec);
	// 衝突判定とそれに伴う座標の更新
	void UpdateCollision();

	bool  RayCast(const Math::Vector3&startPos,const Math::Vector3& vec,const float length,const KdCollider::Type& type,Math::Vector3& resultPos);

	std::shared_ptr<KdModelWork>				m_spModel = nullptr;
	std::shared_ptr<KdAnimator>					m_spAnimator = nullptr;

	std::weak_ptr<CameraBase>					m_wpCamera;
	std::vector<std::weak_ptr<KdGameObject>>	m_wpHitObjectList{};

	Math::Vector3								m_worldRot;

	float										m_clampSize = 10.0f;

	Math::Matrix 								m_scale;

	float										m_Gravity = 0;

	bool										m_isGround = false;

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
		virtual void Exit(std::weak_ptr<Character>& owner) {}

		void Checkkey(std::weak_ptr<Character>& owner);

	protected:

		const Math::Vector3 Direct(std::weak_ptr<Character>& owner,bool isCamera);

		bool m_isBoost = false;
		bool m_isMove = false;
		bool m_isLeftAttack = false;
		bool m_isFlow = false;

		float m_speed = 0.0f;
		Math::Vector3 m_direction = Math::Vector3::Zero;
	};

	class ActionIdle :public ActionStateBase
	{
	public:
		virtual ~ActionIdle() {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	};

	class ActionStandUp :public ActionStateBase
	{
	public:
		virtual ~ActionStandUp() {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	};

	class ActionJump :public ActionStateBase
	{
	public:
		virtual ~ActionJump() {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	};

	class ActionMove :public ActionStateBase
	{
	public:
		virtual ~ActionMove() {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	};

	class ActionBoost :public ActionStateBase
	{
	public:
		virtual ~ActionBoost(){}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;

	private:

		Math::Vector3 m_direction=Math::Vector3::Zero;
	};

	class ActionBoostNow :public ActionStateBase
	{
	public:
		virtual ~ActionBoostNow() {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionBoostEnd :public ActionStateBase
	{
	public:
		virtual ~ActionBoostEnd() {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionBoostDush :public ActionStateBase
	{
	public:
		virtual ~ActionBoostDush() {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionRightAttack :public ActionStateBase
	{
	public:
		virtual ~ActionRightAttack() {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionRightAttackAf :public ActionStateBase
	{
	public:
		virtual ~ActionRightAttackAf() {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	class ActionHited :public ActionStateBase
	{
	public:
		virtual ~ActionHited() {}

		void Enter(std::weak_ptr<Character>& owner) override;
		void Update(std::weak_ptr<Character>& owner)override;
		void Exit(std::weak_ptr<Character>& owner)override;
	private:
	};

	void ChangeActionState(std::shared_ptr<ActionStateBase> nextAction);
	std::shared_ptr<ActionStateBase>		m_nowAction = nullptr;
};