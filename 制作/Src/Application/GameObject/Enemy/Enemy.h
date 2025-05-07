#pragma once
#include"../Character/CharacterBase.h"

#define NITY 90

class Enemy :public CharacterBase
{
public:

	void Init()override;

	void Update()override;
	void PostUpdate()override;

	void GenerateDepthMapFromLight() override;
	void DrawLit()			override;

private:

	std::shared_ptr<KdModelWork>				m_spModel = nullptr;
	std::shared_ptr<KdAnimator>				m_spAnimator = nullptr;

	class ActionStateBase
	{
	public:
		virtual ~ActionStateBase() {}

		virtual void Enter(Enemy& owner) {}
		virtual void Update(Enemy& owner) {}
		virtual void Exit(Enemy& owner) {}

	protected:
	};


	class Stand :public ActionStateBase
	{
	public :
		void Enter(Enemy& owner)override;
		void Update(Enemy& owner)override;
		void Exit(Enemy& owner)override;
	private:
	};

	class MoveForward :public ActionStateBase
	{
	public:
		void Enter(Enemy& owner)override;
		void Update(Enemy& owner)override;
		void Exit(Enemy& owner)override;
	private:
	};

	class MoveBack :public ActionStateBase
	{
	public:
		void Enter(Enemy& owner)override;
		void Update(Enemy& owner)override;
		void Exit(Enemy& owner)override;
	private:
	};

	class Attack:public  ActionStateBase
	{
	public:
		void Enter(Enemy& owner)override;
		void Update(Enemy& owner)override;
		void Exit(Enemy& owner)override;
	private:
	};

	class Hited : public ActionStateBase
	{
	public:
		void Enter(Enemy& owner)override;
		void Update(Enemy& owner)override;
		void Exit(Enemy& owner)override;
	private:
	};

	class Destoroy :public ActionStateBase
	{
	public:
		void Enter(Enemy& owner)override;
		void Update(Enemy& owner)override;
		void Exit(Enemy& owner)override;
	private:
	};

	void ChangeActionState(std::shared_ptr<ActionStateBase> nextAction);
	std::shared_ptr<ActionStateBase>		m_nowAction = nullptr;

};
