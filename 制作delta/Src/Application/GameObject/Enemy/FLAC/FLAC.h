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

	enum FLACStateType
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

		// ステートの継続時間
		float m_durationState = 0.0f;

		// ステートタイプ
		FLACStateType m_type = FLACStateType::Start;

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


};