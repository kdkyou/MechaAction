#pragma once

#include"../WeaponBase.h"

class Character;

class Sowrd :public WeaponBase
{
public:
	void Init()override;
	void Update()override;

	void DrawUnLit()override;

	void SetModel(const std::string& path)override;

	void OnHit()override;

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;
	
	virtual bool Intersects(const KdCollider::BoxInfo& targetBox, std::list<KdCollider::CollisionResult>* pResults)override;

private:

	
	std::shared_ptr<KdTrailPolygon> m_spTrail;
	std::shared_ptr<KdTrailPolygon> m_spTrail2;

	Math::Matrix					m_startMat;
	Math::Matrix					m_endMat;

};