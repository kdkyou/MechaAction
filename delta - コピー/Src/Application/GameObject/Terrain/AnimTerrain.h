#pragma once

#include "DrawTerrain.h"

class AnimTerrain : public DrawTerrain
{
public:
	AnimTerrain() {}
	virtual ~AnimTerrain()		override {}

	void Init()				override;
	void Update()			override;
	void PostUpdate()		override;
	void DrawLit()			override;
	void DrawBright()		override;

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

private:


};