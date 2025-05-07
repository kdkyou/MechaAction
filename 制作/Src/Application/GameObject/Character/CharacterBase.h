#pragma once

class CameraBase;
class CharacterBase : public KdGameObject
{
public:
	CharacterBase() {}
	~CharacterBase()			override {}

	void SetCamera(const std::shared_ptr<CameraBase>& camera)
	{
		m_wpCamera = camera;
	}

	void RegistHitObject(const std::shared_ptr<KdGameObject>& object)
	{
		m_wpHitObjectList.push_back(object);
	}

	const std::weak_ptr<KdModelWork>GetModelWork()const;

protected:


	// キャラクターの回転行列を作成する
	void UpdateRotate(const Math::Vector3& srcMoveVec);
	// 衝突判定とそれに伴う座標の更新
	void UpdateCollision();



	std::weak_ptr<CameraBase>					m_wpCamera;
	std::vector<std::weak_ptr<KdGameObject>>	m_wpHitObjectList{};

	Math::Vector3								m_worldRot;

	Math::Matrix 								m_scale;

	float										m_Gravity = 0;

	bool										m_isGround = false;

	Math::Vector3 m_vMove = Math::Vector3::Zero;

	Math::Vector3 m_emissive = { 10,10,10 };

	//ステートパターン管理系!
protected:

	class ActionStateBase
	{
	public:
		virtual ~ActionStateBase() {}

		virtual void Enter(CharacterBase& owner) {}
		virtual void Update(CharacterBase& owner) {}
		virtual void Exit(CharacterBase& owner) {}

	protected:
	};

	virtual void ChangeActionState(std::shared_ptr<ActionStateBase> nextAction){}
	std::shared_ptr<ActionStateBase>		m_nowAction = nullptr;
};