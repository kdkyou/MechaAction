#pragma once

#include"../WeaponBase.h"

class Shield :public WeaponBase
{
public:
	void Init()override;
	void Update()override;
	void PostUpdate()override;

	
	void DrawUnLit()override;

	void SetModel(const std::string& path)override;

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

	void OnHit()override;

private:

	void CreateShield();

	void ClearShield();
	
	const std::string m_pointName[5] = { "P1","P2", "P3", "P4", "Center" };


	std::shared_ptr<ShieldPolygon> m_spShieldPoly = nullptr;
};