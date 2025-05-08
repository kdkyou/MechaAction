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

private:

	std::shared_ptr<KdModelWork>				m_spModel = nullptr;
	std::shared_ptr<KdAnimator>				m_spAnimator = nullptr;

	std::weak_ptr<KdGameObject>				m_wpTarget;

	class ActionStateBase
	{
	public:
		virtual ~ActionStateBase() {}

		virtual void Enter(Enemy& owner,const std::shared_ptr<KdGameObject>& spObj) {}
		virtual void Update(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj) {}
		virtual void Exit(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj) {}

	protected:
	};


	class Stand :public ActionStateBase
	{
	public :
		void Enter(Enemy& owner, const std::shared_ptr<KdGameObject>& spObj)override;
		void Update(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Exit(Enemy& owner, const std::shared_ptr<KdGameObject>& spObj)override;
	private:
	};

	class MoveForward :public ActionStateBase
	{
	public:
		void Enter(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Update(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Exit(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
	private:
	};

	class MoveBack :public ActionStateBase
	{
	public:
		void Enter(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Update(Enemy& owner, const std::shared_ptr<KdGameObject>& spObj)override;
		void Exit(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
	private:
	};

	class Attack:public  ActionStateBase
	{
	public:
		void Enter(Enemy& owner, const std::shared_ptr<KdGameObject>& spObj)override;
		void Update(Enemy& owner, const std::shared_ptr<KdGameObject>& spObj)override;
		void Exit(Enemy& owner, const std::shared_ptr<KdGameObject>& spObj)override;
	private:
	};

	class Hited : public ActionStateBase
	{
	public:
		void Enter(Enemy& owner, const std::shared_ptr<KdGameObject>& spObj)override;
		void Update(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Exit(Enemy& owner, const std::shared_ptr<KdGameObject>& spObj)override;
	private:
	};

	class Destoroy :public ActionStateBase
	{
	public:
		void Enter(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Update(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)override;
		void Exit(Enemy& owner, const std::shared_ptr<KdGameObject>& spObj)override;
	private:
	};

	void ChangeActionState(std::shared_ptr<ActionStateBase> nextAction);
	std::shared_ptr<ActionStateBase>		m_nowAction = nullptr;

};
