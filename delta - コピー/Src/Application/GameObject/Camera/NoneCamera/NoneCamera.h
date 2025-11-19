#pragma once

#include"../CameraBase.h"

class NoneCamera :public CameraBase
{
public:

	void Init()override;
	void Update()override;
	void PostUpdate()override;

	virtual void Editor_ImGui()override;

	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

private:

	float m_movePow = {3.0f};



};