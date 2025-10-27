#pragma once

class CameraBase;
class WeaponBase;
class CharacterBase : public KdGameObject
{
public:
	CharacterBase() {}
	~CharacterBase()			override {}

	void GenerateDepthMapFromLight()override;

	void DrawMarker()override;
	
	void SetCamera(const std::shared_ptr<CameraBase>& camera)
	{
		m_wpCamera = camera;
	}

	void RegistHitObject(const std::shared_ptr<KdGameObject>& object)
	{
		m_wpHitObjectList.push_back(object);
	}

	const std::weak_ptr<KdModelWork> GetModelWork()const {
		return m_spModelWork;
	}

	const Math::Matrix& GetCorrectionMatrix() const{ return m_correctionMat; }

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

	const bool IsEnableAttack()const { return m_isEnableAttackCollision; }
	void ChangeEnableAttack(bool attack) { m_isEnableAttackCollision = attack; }

	const bool IsHit() const{ return m_isHit; }
	void HitDamage(float damage);
	void SetHitEnable(bool hit) { m_isHit = hit; }
	const Math::Vector3& GetHitDir()const { return m_hitDir; }

	void SetParam(const float hp) { m_hp = hp; }

	virtual void SetModelWork(const std::string& path);

	void SetTarget(const std::shared_ptr<KdGameObject>& target) { m_wpTarget = target; }
	const std::weak_ptr<KdGameObject>& GetTarget()const { return m_wpTarget; }
	
	void SetCharacterTarget(const std::shared_ptr<CharacterBase>& target) { m_wpCharacterTarget = target; }
	const std::weak_ptr<CharacterBase>& GetCharacterTarget()const { return m_wpCharacterTarget; }

	void SetThisBase(const std::shared_ptr<CharacterBase>& target) { m_wpBase = target; }
	const std::weak_ptr<CharacterBase>& GetThisBase()const { return m_wpBase; }


	const Math::Vector2& GetDist()const { return m_dist; }

	const bool IsDestroy()const { return m_isDestroy; }

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

protected:

	void SetWeapon();

	virtual void UpdateCollision(){}

	//				移動量　　方向		当たり判定するタイプ（基本地面） 判定のみか キャラが回転するか 座標補正なし　レイに補正をかけるか
	bool Move(float speed, const Math::Vector3& dir, const KdCollider::Type type, bool ray = false, bool rotate = true, bool direct = false, bool step = false);
	// 
	bool MoveSwept(float speed, const Math::Vector3& dir, const KdCollider::Type type, bool ray = false, bool rotate = true, bool direct = false, bool step = false);

	bool  RayCast(const Math::Vector3& startPos, const Math::Vector3& vec, const float length, const KdCollider::Type& type, Math::Vector3& resultPos);
	bool  Gravity(const Math::Vector3& startPos, const Math::Vector3& vec, const float length);

	bool  SphereCast(const Math::Vector3& pos, const float radius, const KdCollider::Type& type, Math::Vector3& resultPos);

	bool CheckSweptSphere(const Math::Vector3& oldPos, const Math::Vector3& newPos, float radius, KdCollider::CollisionResult& out);

	bool SearchDetect(const Math::Vector3& hitPos, const Math::Matrix& myMat, float viewRange);

	bool SeaarchObstacle(const Math::Vector3& pos, const Math::Vector3& vec,const float length);

	virtual void UpdateRotate(const Math::Vector3& srcMoveVec) { srcMoveVec; }

	void BoostRotate(const Math::Vector3& vec);

	virtual bool Search(bool areaOnly) {
		areaOnly;
		return false; }
	bool SearchPlayer();

	const bool Burn();

	void WeaponCreate(std::weak_ptr<CharacterBase>& parent);

	std::string									m_modelPath;
	std::shared_ptr<KdModelWork>				m_spModelWork;
	std::shared_ptr<KdModelData>				m_spModelData;
	std::shared_ptr<KdModelData>				m_spMrkModel;
	std::shared_ptr<KdAnimator>					m_spAnimator = nullptr;

	std::weak_ptr<CameraBase>					m_wpCamera;
	std::vector<std::weak_ptr<KdGameObject>>	m_wpHitObjectList{};

	std::weak_ptr<CharacterBase>				m_wpCharacterTarget;
	std::weak_ptr<CharacterBase>				m_wpBase;
	std::weak_ptr<KdGameObject>					m_wpTarget;
	std::vector<std::weak_ptr<WeaponBase>>	m_wpWeapons;

	
	Math::Matrix 								m_mScale;

	Math::Matrix								m_correctionMat = Math::Matrix::Identity;

	// 補正値
	Math::Vector3                               m_correction = {};

	float										m_gravity = 0;
//	const float									m_gravityPow = 9.81f;
	const float									m_gravityPow = 19.62f;
//	const float									m_gravityPow = 29.43f;

	float										m_clampSize = 10.0f;

	bool										m_isGround = false;

	bool										m_isRightAttack = false;
	bool										m_isLeftAttack = false;
	bool										m_isRightShoulderAttack = false;
	bool										m_isLeftShoulderAttack = false;
	bool										m_isEnableAttackCollision = false;

	bool										m_isDestroy = false;

	std::string									m_burnPath;

	Math::Vector3							m_boxExtents;

	Math::Vector3 m_vMove = Math::Vector3::Zero;

	// パラメータ関係
	float										m_hp = 0;
	float										m_maxHp = 0;
	bool										m_isHit = false;
	Math::Vector3								m_hitDir = {};
	float										m_nockBackDamage = 0.0f;

	//			追いかける範囲　x = Near　y = Far
	Math::Vector2								m_dist = { 10.0f,180.0f };
	float										m_viewRange = 0.0f;


	// モデルカラー
	Math::Color								m_modelColor = {1,1,1,1};
	// エミッシブ
	Math::Vector3								m_emissiveColor = {10,10,10};

	// リムライト
	Math::Vector3 m_limColor = { 0.19f,0.09f,0.09f };
	float m_limPow =3.5f;
	bool m_limEnable = false;
	
	// トランスAC
	bool									    m_transAC = false;
	Math::Vector3 m_TACColor = { 0.19f,0.09f,0.09f };
	float m_TACProg =0.3f;
	float m_TACspeed =1.0f;

	// サウンドインスタンス管理
	std::unordered_map<std::string,std::weak_ptr<KdSoundInstance>> m_sounds;


	KdRandomGenerator						m_rand;
};