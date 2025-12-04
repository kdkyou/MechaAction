#pragma once

#include"../../Character/CharacterBase.h"

class Scarecrow:public CharacterBase
{
public:

	void Init()override;

	void Update()override;
	void PostUpdate()override;

	void DrawLit()			override;


	void SetThis(const std::shared_ptr<Scarecrow>& spthis) { m_wpThis = spthis; }

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

	float									m_overRap = 0.0f;
	float									m_duration = 1.0f;


	// 視野角
	float									m_viewAngle = 0.0f;
	// 索敵範囲
	float									m_radius = 200.0f;
	// ミサイルかどうか
	bool									m_isBullet = false;



	std::weak_ptr<Scarecrow>				    m_wpThis;

};