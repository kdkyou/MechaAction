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

private:

	//WASDの入力があったかどうか
	const bool IsMove();

	//左SHIFTの入力があったか
	const bool IsBoost();

	//左クリック入力
	const bool IsLeftAttack();

	// キャラクターの回転行列を作成する
	void UpdateRotate(const Math::Vector3& srcMoveVec);
	// 衝突判定とそれに伴う座標の更新
	void UpdateCollision();

	bool  RayCast(const Math::Vector3&startPos,const Math::Vector3& vec,const float length=1.0f);

	std::shared_ptr<KdModelWork>				m_spModel = nullptr;
	std::shared_ptr<KdAnimator>					m_spAnimator = nullptr;

	std::weak_ptr<CameraBase>					m_wpCamera;
	std::vector<std::weak_ptr<KdGameObject>>	m_wpHitObjectList{};

	Math::Vector3								m_worldRot;

	Math::Matrix 								m_scale;

	float										m_Gravity = 0;

	bool										m_isGround = false;

	Math::Vector3 m_vMove = Math::Vector3::Zero;

	Math::Vector3 m_emissive = { 10,10,10 };

	//ステートパターン管理系!
private:

	class ActionStateBase
	{
	public:
		virtual ~ActionStateBase() {}

		virtual void Enter(Character& owner) {}
		virtual void Update(Character& owner) {}
		virtual void Exit(Character& owner) {}

		void Checkkey(Character& owner);

	protected:
		bool m_isBoost = false;
		bool m_isMove = false;
		bool m_isLeftAttack = false;
	};

	class ActionIdle :public ActionStateBase
	{
	public:
		virtual ~ActionIdle() {}

		void Enter(Character& owner) override;
		void Update(Character& owner)override;
		void Exit(Character& owner)override;
	};

	class ActionStandUp :public ActionStateBase
	{
	public:
		virtual ~ActionStandUp() {}

		void Enter(Character& owner) override;
		void Update(Character& owner)override;
		void Exit(Character& owner)override;
	};

	class ActionJump :public ActionStateBase
	{
	public:
		virtual ~ActionJump() {}

		void Enter(Character& owner) override;
		void Update(Character& owner)override;
		void Exit(Character& owner)override;
	};

	class ActionMove :public ActionStateBase
	{
	public:
		virtual ~ActionMove() {}

		void Enter(Character& owner) override;
		void Update(Character& owner)override;
		void Exit(Character& owner)override;
	};

	class ActionBoost :public ActionStateBase
	{
	public:
		virtual ~ActionBoost(){}

		void Enter(Character& owner) override;
		void Update(Character& owner)override;
		void Exit(Character& owner)override;

	private:

		Math::Vector3 m_direction=Math::Vector3::Zero;
	};

	class ActionBoostNow :public ActionStateBase
	{
	public:
		virtual ~ActionBoostNow() {}

		void Enter(Character& owner) override;
		void Update(Character& owner)override;
		void Exit(Character& owner)override;
	};

	class ActionBoostEnd :public ActionStateBase
	{
	public:
		virtual ~ActionBoostEnd() {}

		void Enter(Character& owner) override;
		void Update(Character& owner)override;
		void Exit(Character& owner)override;
	};

	class ActionBoostDush :public ActionStateBase
	{
	public:
		virtual ~ActionBoostDush() {}

		void Enter(Character& owner) override;
		void Update(Character& owner)override;
		void Exit(Character& owner)override;
	};

	class ActionLeftAttack :public ActionStateBase
	{
	public:
		virtual ~ActionLeftAttack() {}

		void Enter(Character& owner) override;
		void Update(Character& owner)override;
		void Exit(Character& owner)override;
	};

	class ActionLeftAttackAf :public ActionStateBase
	{
	public:
		virtual ~ActionLeftAttackAf() {}

		void Enter(Character& owner) override;
		void Update(Character& owner)override;
		void Exit(Character& owner)override;
	};



	void ChangeActionState(std::shared_ptr<ActionStateBase> nextAction);
	std::shared_ptr<ActionStateBase>		m_nowAction = nullptr;
};