#pragma once

#include "../../Character/CharacterBase.h"

class FLAC : public CharacterBase
{
public:

	void Init()override;
	void Update()override;
	void PostUpdate()override;

	void DrawLit()			override;

	void SetThis(const std::shared_ptr<FLAC>& spthis) { m_wpThis = spthis; }

	void OnHit()override;

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

private:

	void UpdateRotate(const Math::Vector3& srcMoveVec)override;


	enum class FLACStateType
	{
		Start,
		Stand,
		Fly,
		FrontMove,
		FrontMoveAttack,
		BackMove,
		BackMoveAttack,
		LeftMove,
		LeftMoveAttack,
		RightMove,
		RightMoveAttack,
		Hited,
		Avoid,
		Destroyed,
	};


	std::weak_ptr<FLAC>				    m_wpThis;

	bool CheckLengthBulPlay();

	const Math::Matrix& UpdateMatrix();

	const FLACStateType GetPrevState()const { return m_prevAction->GetType(); }

	class ActionStateBase
	{
	public:
		virtual ~ActionStateBase() {}

		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj) {}
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) {}
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) {}
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) {}

		void SetParam(float speed, const Math::Vector3& direct);

		enum TargetSide
		{
			Front,
			Right,
			Left,
			Back,
		};

		const FLACStateType& GetType()const { return m_type; }

	protected:

		void CreateEffect(std::weak_ptr<FLAC>& owner, const std::string& effectName, const std::string& nodeName);
		void EffectUpdate(std::weak_ptr<FLAC>& owner);
		void EffectExit();

		UINT Serch(const Math::Vector3& nowVec, const Math::Vector3& targetVec);
		void ChangeStateWithPrev(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj);
		void ChangeStateWithDistance(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj);

		void CheckAttackLevel(std::weak_ptr<FLAC>& owner);
		void AttackOff(std::weak_ptr<FLAC>& owner);

		// ステートの継続時間
		float m_durationState = 0.0f;

		// ステートタイプ
		FLACStateType m_type = FLACStateType::Start;
		TargetSide	m_avoidSide = TargetSide::Front;

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

	class StandUp :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};

	class Stand :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};

	class Fly :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};

	class Boost :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};

	class FrontMove :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};

	class FrontMoveAttack :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};

	class BackMove :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};
	
	class BackMoveAttack :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};
	
	class LeftMove :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};

	class LeftMoveAttack :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};
	
		class RightMove :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};

	class RightMoveAttack :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};

	class Hited :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};
	
		class Avoid :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};

	class Destroyed :public ActionStateBase
	{
	public:
		virtual void Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)override;
		virtual void Update(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj)override;
		virtual void PostUpdate(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;
		virtual void Exit(std::weak_ptr<FLAC>& owner, const  std::weak_ptr<KdGameObject>& spObj) override;

	private:

	};

	void ChangeActionState(std::shared_ptr<ActionStateBase> nextAction);
	std::shared_ptr<ActionStateBase>		m_nowAction = nullptr;
	std::shared_ptr<ActionStateBase>		m_prevAction = nullptr;

};