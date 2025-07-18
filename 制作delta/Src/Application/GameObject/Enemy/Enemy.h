#pragma once

#include"../Character/CharacterBase.h"


class Enemy :public CharacterBase
{
public:

	enum AnotherStateType
	{
		tStart,
		tStandUp,
		tStand,
		tStandAttack,
		tBoost,
		tBoostStop,
		tMoveForward,
		tRotateRight,
		tRotateLeft,
		tMoveBack,
		tFrontAttack,
		tRightAttack,
		tLeftAttack,
		tBackAttack,

	};

	void Init()override;

	void Update()override;
	void PostUpdate()override;

	void DrawLit()			override;

	void SetTarget(const std::shared_ptr<KdGameObject>& target) { m_wpTarget = target; }

	void SetThis(const std::shared_ptr<Enemy>& spthis) { m_wpThis = spthis; }


private:

	void UpdateRotate(const Math::Vector3& srcMoveVec)override;

	void UpdateCollision()override;

	bool Search(bool areaOnly);

	void Editor_ImGui()override;
	
	std::weak_ptr<KdGameObject>				m_wpTarget;

	float									m_angle = 6.0f;
	
	bool									m_isForWard = false;

	//			追いかける範囲　x = Near　y = Far
	Math::Vector2							m_dist = {10.0f,180.0f};
	// 視野角
	float									m_viewAngle = 0.0f;
	// 索敵範囲
	float									m_radius = 200.0f;
	// ミサイルかどうか
	bool									m_isBullet = false;

	// 補正値
	Math::Vector3							m_currection = { 0.0f,5.0f,0.0f };




	std::weak_ptr<Enemy>				    m_wpThis;


	class ActionStateBase
	{
	public:
		virtual ~ActionStateBase() {}

		virtual void Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj){}
		virtual void Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj){}
		virtual void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj){}
		virtual void Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj){}

		void SetParam(float speed,const Math::Vector3& direct);

		enum TargetSide
		{
			Front,
			Right,
			Left,
			Back,
		};

	protected:


		void EffectUpdate(std::weak_ptr<Enemy>& owner);
		void EffectExit();

		UINT Serch(const Math::Vector3& nowVec, const Math::Vector3& targetVec);
		void ChangeStateWithPrev(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj);
		void ChangeStateWithDisttance(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj);

		// ステートの継続時間
		float m_durationState = 0.0f;

		// ステートタイプ
		AnotherStateType m_type;

		Math::Vector3 m_direct = {};
		float					m_speed = 0.0f;

		struct Effect
		{
			std::string name;
			std::weak_ptr<KdEffekseerObject> wpEffect;
			Effekseer::Handle handle = 0;
			Math::Matrix pNodeMat = Math::Matrix::Identity;
		};

		std::list<std::shared_ptr<Effect>> m_spEffects;
	};

	class Start :public ActionStateBase
	{
	public:
		~Start()override {}

		void Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};


	class StandUp :public ActionStateBase
	{
	public:
		~StandUp()override {}

		void Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	class Stand :public ActionStateBase
	{
	public:
		~Stand()override{}

		void Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	class Alert :public ActionStateBase
	{
	public:
		~Alert()override {}

		void Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	class Boost : public ActionStateBase
	{
	public :
		~Boost()override{}

		void Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	class BoostStop : public ActionStateBase
	{
	public:
		~BoostStop()override {}

		void Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	class MoveForward :public ActionStateBase
	{
	public:
		~MoveForward()override{}

		void Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	class MoveBack :public ActionStateBase
	{
	public:
		~MoveBack()override{}

		void Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	class MoveRightRotate :public  ActionStateBase
	{
	public:
	~MoveRightRotate()override{}

		void Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	class MoveLeftRotate :public  ActionStateBase
	{
	public:
		~MoveLeftRotate()override {}

		void Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	class AttackStand :public ActionStateBase
	{
	public:
		~AttackStand()override {}

		void Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
	};

	class AttackForWard :public ActionStateBase
	{
	public :
		~AttackForWard()override {}

		void Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
	};

	class AttackBack :public  ActionStateBase
	{
	public:
		~AttackBack()override {}

		void Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	class AttackRight :public  ActionStateBase
	{
	public:
		~AttackRight()override {}

		void Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	class AttackLeft :public ActionStateBase
	{
	public:
		~AttackLeft()override {}

		void Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
	};

	class Hited :public ActionStateBase
	{
	public:
		~Hited()override {}

		void Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	class Destoroy :public ActionStateBase
	{
	public:

		~Destoroy()override {}

		void Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
	private:
	};

	

	void ChangeActionState(std::shared_ptr<ActionStateBase> nextAction);
	std::shared_ptr<ActionStateBase>		m_nowAction = nullptr;
	std::shared_ptr<ActionStateBase>		m_prevAction = nullptr;


};
