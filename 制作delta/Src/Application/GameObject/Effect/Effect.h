#pragma once

class Effect :public KdGameObject
{
public:

	void Init()override;

	void SetTexture(const std::string& path);

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

protected:

	// 生存時間 
	float m_durationAlive = 0.0f;

	// 板ポリ本体
	std::shared_ptr<KdSquarePolygon> m_spPoly;

	std::shared_ptr<KdTexture> m_spTex;
};