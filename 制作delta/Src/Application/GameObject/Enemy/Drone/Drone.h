#pragma once

#include "../../Character/CharacterBase.h"

class Drone :public CharacterBase
{
public:

	void Init()override;

	void Update()override;
	void PostUpdate()override;

	void DrawLit()			override;

	void SetTarget(const std::shared_ptr<KdGameObject>& target) { m_wpTarget = target; }

	void SetThis(const std::shared_ptr<Drone>& spthis) { m_wpThis = spthis; }

	void OnHit()override;


private:

	void UpdateRotate(const Math::Vector3& srcMoveVec)override;

	void UpdateCollision()override;

	void UpdateMatrix();

	bool Search(bool areaOnly)override;

	void Editor_ImGui()override;

	std::weak_ptr<KdGameObject>				m_wpTarget;

	float									m_angle = 6.0f;

	bool									m_isForWard = false;


	// 視野角
	float									m_viewAngle = 0.0f;
	// 索敵範囲
	float									m_radius = 200.0f;
	// ミサイルかどうか
	bool									m_isBullet = false;

	// 補正値
	Math::Vector3							m_currection = { 0.0f,5.0f,0.0f };

	
	std::weak_ptr<Drone>				    m_wpThis;


	class ActionStateBase
	{
	public:
		virtual ~ActionStateBase() {}

		virtual void Enter(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj) {}
		virtual void Update(std::weak_ptr<Drone>& owner, const  std::weak_ptr<KdGameObject>& obj) {}
		virtual void PostUpdate(std::weak_ptr<Drone>& owner, const  std::weak_ptr<KdGameObject>& obj) {}
		virtual void Exit(std::weak_ptr<Drone>& owner, const  std::weak_ptr<KdGameObject>& obj) {}

		void SetParam(float speed, const Math::Vector3& direct);

		enum TargetSide
		{
			Front,
			Right,
			Left,
			Back,
		};

	protected:


		void EffectUpdate(std::weak_ptr<Drone>& owner);
		void EffectExit();

		UINT Serch(const Math::Vector3& nowVec, const Math::Vector3& targetVec);
		void ChangeStateWithDistance(std::weak_ptr<Drone>& owner,float targetLength);

		// ステートの継続時間
		float m_durationState = 0.0f;

		

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

	
	class Idle :public ActionStateBase
	{
	public:
		~Idle()override {}

		void Enter(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)override;
		void Update(std::weak_ptr<Drone>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void PostUpdate(std::weak_ptr<Drone>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void Exit(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)override;
	private:
	};

	class MoveMent :public ActionStateBase
	{
	public:
		~MoveMent()override {}

		void Enter(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)override;
		void Update(std::weak_ptr<Drone>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void PostUpdate(std::weak_ptr<Drone>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void Exit(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)override;
	private:
	};

	class Attack :public ActionStateBase
	{
	public:
		~Attack()override {}

		void Enter(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)override;
		void Update(std::weak_ptr<Drone>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void PostUpdate(std::weak_ptr<Drone>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void Exit(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)override;
	private:
	};

	class Destroyed :public ActionStateBase
	{
	public:
		~Destroyed()override {}

		void Enter(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)override;
		void Update(std::weak_ptr<Drone>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void PostUpdate(std::weak_ptr<Drone>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void Exit(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)override;
	private:
	};


	void ChangeActionState(std::shared_ptr<ActionStateBase> nextAction);
	std::shared_ptr<ActionStateBase>		m_nowAction = nullptr;
	std::shared_ptr<ActionStateBase>		m_prevAction = nullptr;

};