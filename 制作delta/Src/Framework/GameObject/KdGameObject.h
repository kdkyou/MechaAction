#pragma once

#include "../../Framework/nlohmann/KdJsonUtility.h"

// ゲーム上に存在するすべてのオブジェクトの基底となるクラス
class KdGameObject : public std::enable_shared_from_this<KdGameObject>
{
public:

	// どのような描画を行うのかを設定するTypeID：Bitフラグで複数指定可能
	enum
	{
		eDrawTypeLit = 1 << 0,
		eDrawTypeUnLit = 1 << 1,
		eDrawTypeBright = 1 << 2,
		eDrawTypeUI = 1 << 3,
		eDrawTypeDepthOfShadow = 1 << 4,
	};

	enum ObjectTag
	{
		tNone,
		tPlayer,
		tEnemy,
		tPlayerAttack,
		tEnemyAttack,
		tTerrain,
		tUI,
	};

	KdGameObject() {}
	virtual ~KdGameObject() { Release(); }

	// 生成される全てに共通するパラメータに対する初期化のみ
	virtual void Init() {}

	virtual void PreUpdate() {}
	virtual void Update() {}
	virtual void PostUpdate() {}

	// それぞれの状況で描画する関数
	virtual void GenerateDepthMapFromLight() {}
	virtual void PreDraw() {}
	virtual void DrawLit() {}
	virtual void DrawUnLit() {}
	virtual void DrawBright() {}
	virtual void DrawSprite() {}

	virtual void DrawDebug();

	virtual void SetAsset(const std::string&) {}

	virtual void SetPos(const Math::Vector3& pos) { m_mWorld.Translation(pos); }
	virtual Math::Vector3 GetPos() const { return m_mWorld.Translation(); }

	// 拡大率を変更する関数
	void SetScale(float scalar);
	virtual void SetScale(const Math::Vector3& scale);
	virtual Math::Vector3 GetScale() const;

	const Math::Matrix& GetMatrix() const { return m_mWorld; }

	virtual bool IsExpired() const { return m_isExpired; }

	virtual bool IsVisible()	const { return false; }
	virtual bool IsRideable()	const { return false; }

	// 視錐台範囲内に入っているかどうか
	virtual bool CheckInScreen(const DirectX::BoundingFrustum&) const { return false; }

	// カメラからの距離を計算
	virtual void CalcDistSqrFromCamera(const Math::Vector3& camPos);

	float GetDistSqrFromCamera() const { return m_distSqrFromCamera; }

	UINT GetDrawType() const { return m_drawType; }

	bool Intersects(const KdCollider::SphereInfo& targetShape, std::list<KdCollider::CollisionResult>* pResults);
	bool Intersects(const KdCollider::BoxInfo& targetBox, std::list<KdCollider::CollisionResult>* pResults);
	bool Intersects(const KdCollider::RayInfo& targetShape, std::list<KdCollider::CollisionResult>* pResults);

	
	virtual void OnHit(){}
	virtual float GetParameter()const{ return m_parameter; }

	const ObjectTag GetTag()const { return m_tag; }
	void SetTag(const ObjectTag tag) { m_tag = tag; }

	const std::string& GetName() { return m_name; }

	virtual void Editor_ImGui(){
		ImGui::DragFloat3("Pos", &m_pos.x, 0.1f, -FLT_MAX, FLT_MAX);
		ImGui::DragFloat3("Scale", &m_scale.x, 0.1f, -FLT_MAX, FLT_MAX);
		ImGui::DragFloat3("Rotation", &m_rot.x, 0.1f, -FLT_MAX, FLT_MAX);

	}
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)
	{
		m_name = jsonObj["Name"];
		KdJsonUtility::GetArray(jsonObj,"Pos",&m_pos.x, 3);
		KdJsonUtility::GetArray(jsonObj,"Rot",&m_rot.x, 3);
		KdJsonUtility::GetArray(jsonObj,"Scale",&m_scale.x, 3);
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const
	{
		outJson["Name"] = m_name;
		outJson["Pos"]  = KdJsonUtility::CreateArray(&m_pos.x,3);
		outJson["Rot"]  = KdJsonUtility::CreateArray(&m_rot.x,3);
		outJson["Scale"]  = KdJsonUtility::CreateArray(&m_scale.x,3);
	}

	
protected:

	void Release() {}

	// 描画タイプ・何の描画を行うのかを決める / 最適な描画リスト作成用
	UINT m_drawType = 0;

	// カメラからの距離
	float m_distSqrFromCamera = 0;

	// 存在消滅フラグ
	bool m_isExpired = false;

	// 3D空間に存在する機能
	Math::Matrix	m_mWorld;

	ObjectTag m_tag = tNone;

	// 当たり判定クラス
	std::unique_ptr<KdCollider> m_pCollider = nullptr;

	// デバッグ情報クラス
	std::unique_ptr<KdDebugWireFrame> m_pDebugWire = nullptr;

	// 受け渡したい何らかの数値一つ(float型) 
	float m_parameter = 0.0f;

	std::string m_name = "";

	Math::Vector3 m_pos;
	Math::Vector3 m_scale = { 1,1,1 };
	Math::Vector3 m_rot;
	Math::Quaternion m_rotation = {};
};
