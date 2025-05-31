#pragma once

class Enemy :public KdGameObject
{
public:

	void Init()override;

	void Update()override;
	void PostUpdate()override;

	void GenerateDepthMapFromLight() override;
	void DrawLit()			override;

	void SetTarget(const std::shared_ptr<KdGameObject>& target) { m_wpTarget = target; }

	void SetThis(const std::shared_ptr<Enemy>& spthis) { m_wpThis = spthis; }

private:

	std::shared_ptr<KdModelWork>				m_spModel = nullptr;
	std::shared_ptr<KdAnimator>				m_spAnimator = nullptr;

	std::weak_ptr<KdGameObject>				m_wpTarget;

	float									m_angle = 6.0f;
	Math::Vector3 m_worldRot = Math::Vector3::Zero;

	bool									m_isForWard = false;

	//			追いかける範囲　x = Near　y = Far
	Math::Vector2									m_dist = {10.0f,30.0f};

	std::weak_ptr<Enemy>                            m_wpThis;


	class ActionStateBase
	{
	public:
		virtual ~ActionStateBase() {}

		virtual void Enter(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj) {}
		virtual void Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj) {}
		virtual void Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj) {}

	protected:
	};


	class Stand :public ActionStateBase
	{
	public:
		void Enter(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)override;
	private:
	};

	class MoveForward :public ActionStateBase
	{
	public:
		void Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
	private:
	};

	class MoveBack :public ActionStateBase
	{
	public:
		void Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
	private:
	};

	class Attack :public  ActionStateBase
	{
	public:
		void Enter(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)override;
	private:
	};

	class Hited :public ActionStateBase
	{
	public:
		void Enter(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)override;
	private:
	};

	class Destoroy :public ActionStateBase
	{
	public:
		void Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Exit(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)override;
	private:
	};

	

	void ChangeActionState(std::shared_ptr<ActionStateBase> nextAction);
	std::shared_ptr<ActionStateBase>		m_nowAction = nullptr;

};
