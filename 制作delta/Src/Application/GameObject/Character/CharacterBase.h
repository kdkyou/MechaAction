#pragma once

class CameraBase;
class CharacterBase : public KdGameObject
{
public:
	CharacterBase() {}
	~CharacterBase()			override {}

	void GenerateDepthMapFromLight()override;

	void SetCamera(const std::shared_ptr<CameraBase>& camera)
	{
		m_wpCamera = camera;
	}

	void RegistHitObject(const std::shared_ptr<KdGameObject>& object)
	{
		m_wpHitObjectList.push_back(object);
	}

	const std::weak_ptr<KdModelWork>GetModelWork()const {
		return m_spModelWork;
	}

	const Math::Matrix& GetCorrectionMatrix() const{ return m_correctionMat * m_mWorld; }

	// 右手行動状態かどうかを調べる
	const bool IsRightAttack()const { return m_isRightAttack; }
	void ChangeEnableRightAttack(bool attack) { m_isRightAttack = attack; }

	// 左手行動状態かどうかを調べる
	const bool IsLeftAttack()const { return m_isLeftAttack; }
	void ChangeEnableLeftAttack(bool attack) { m_isLeftAttack = attack; }

	// 右肩行動状態かどうかを調べる
	const bool IsRightShoudlerAttack()const { return m_isRightShoulderAttack; }
	void ChangeEnableRightShoulderAttack(bool attack) { m_isRightShoulderAttack = attack; }

	// 左肩行動状態かどうかを調べる
	const bool IsLeftShoulderAttack()const { return m_isLeftShoulderAttack; }
	void ChangeEnableLeftShoulderAttack(bool attack) { m_isLeftShoulderAttack = attack; }

	const bool IsHit() const{ return m_isHit; }
	void HitDamage(float damage);
	void SetHitEnable(bool hit) { m_isHit = hit; }


protected:

	virtual void UpdateCollision(){}

	bool SearchDetect(const Math::Vector3& hitPos, const Math::Matrix& myPos, float viewRange);


	std::shared_ptr<KdModelWork>				m_spModelWork;
	std::shared_ptr<KdModelData>				m_spModelData;
	std::shared_ptr<KdAnimator>					m_spAnimator = nullptr;

	std::weak_ptr<CameraBase>					m_wpCamera;
	std::vector<std::weak_ptr<KdGameObject>>	m_wpHitObjectList{};

	Math::Vector3								m_worldRot;

	Math::Matrix 								m_scale;

	Math::Matrix								m_correctionMat = Math::Matrix::Identity;

	float										m_gravity = 0;

	bool										m_isGround = false;

	bool										m_isRightAttack = false;
	bool										m_isLeftAttack = false;
	bool										m_isRightShoulderAttack = false;
	bool										m_isLeftShoulderAttack = false;

	Math::Vector3 m_vMove = Math::Vector3::Zero;

	// パラメータ関係
	float										m_hp = 0;
	bool										m_isHit = false;

};