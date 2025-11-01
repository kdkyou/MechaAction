#pragma once

#include "../../Character/CharacterBase.h"

class MT :public CharacterBase
{

public:

	void Init()override;

	void Update()override;
	void PostUpdate()override;

	void DrawLit()	override;

	void SetThis(const std::shared_ptr<MT>& spthis) { m_wpThis = spthis; }

	void OnHit()override;

	void Editor_ImGui()override;

	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;


private:



	void UpdateRotate(const Math::Vector3& srcMoveVec)override;

	void UpdateCollision()override;

	void UpdateMatrix();

	bool Search(bool areaOnly)override;

	std::weak_ptr<KdGameObject>				m_wpTarget;

	float									m_angle = 6.0f;

	bool									m_isForWard = false;

	float							m_overRap = 0.0f;


	// 視野角
	float									m_viewAngle = 0.0f;
	// 索敵範囲
	float									m_radius = 200.0f;
	// ミサイルかどうか
	bool									m_isBullet = false;



	std::weak_ptr<MT>				    m_wpThis;


	class ActionStateBase
	{
	public:
		virtual ~ActionStateBase() {}

		virtual void Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj) {}
		virtual void Update(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj) {}
		virtual void PostUpdate(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj) {}
		virtual void Exit(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj) {}

		void SetParam(float speed, const Math::Vector3& direct);

		enum TargetSide
		{
			Front,
			Right,
			Left,
			Back,
		};

	protected:

		void CreateEffect(std::weak_ptr<MT>& owner, const std::string& effectName, const std::string& nodeName);
		void EffectUpdate(std::weak_ptr<MT>& owner);
		void EffectExit();

		UINT Serch(const Math::Vector3& nowVec, const Math::Vector3& targetVec);
		void ChangeStateWithDistance(std::weak_ptr<MT>& owner, float targetLength);
		bool ChangeStateObstacle(std::weak_ptr<MT>& owner);

		void SetMoveDir(TargetSide side) { m_side = side; }

		// ステートの継続時間
		float							m_durationState = 0.0f;

		TargetSide						m_side = TargetSide::Front;

		Math::Vector3					m_direct = {};
		float							m_speed = 0.0f;

		float							m_sin = 0.0f;

		bool							m_anyFlg = false;

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
		~StandUp()override {}

		void Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
		void Update(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void PostUpdate(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
	private:
	};

	class Idle :public ActionStateBase
	{
	public:
		~Idle()override {}

		void Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
		void Update(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void PostUpdate(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
	private:
	};

	class MoveMent :public ActionStateBase
	{
	public:
		~MoveMent()override {}

		void Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
		void Update(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void PostUpdate(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
	private:
	};

	class Attack :public ActionStateBase
	{
	public:
		~Attack()override {}

		void Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
		void Update(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void PostUpdate(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
	private:
	};
	
	class Hited :public ActionStateBase
	{
	public:
		~Hited()override {}

		void Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
		void Update(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void PostUpdate(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
	private:
	};

	class Destroyed :public ActionStateBase
	{
	public:
		~Destroyed()override {}

		void Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
		void Update(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void PostUpdate(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
	private:
	};

	class Backed : public ActionStateBase
	{
	public:
		~Backed()override {}

		void Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
		void Update(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void PostUpdate(std::weak_ptr<MT>& owner, const  std::weak_ptr<KdGameObject>& obj)override;
		void Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)override;
	private:

	};


	void ChangeActionState(std::shared_ptr<ActionStateBase> nextAction);
	std::shared_ptr<ActionStateBase>		m_nowAction = nullptr;
	std::shared_ptr<ActionStateBase>		m_prevAction = nullptr;

};