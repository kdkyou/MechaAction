#pragma once

#include "DrawTerrain.h"

class ParentTerrain :public DrawTerrain
{
public:
	ParentTerrain() {}
	virtual ~ParentTerrain()		override {}

	void Init()				override;
	void Update()			override;
	void DrawUnLit()		override;	

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override
	{
		DrawTerrain::Deserialize(jsonObj);
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override
	{
		DrawTerrain::Serialize(outJson);
	}

	void SetParent(const std::shared_ptr<KdGameObject>& parent) {
		m_wpParent = parent;
	}

private:
	
	std::weak_ptr<KdGameObject>  m_wpParent;
	
};