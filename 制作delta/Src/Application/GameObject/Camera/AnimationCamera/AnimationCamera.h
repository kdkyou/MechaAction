#pragma once

#include"../CameraBase.h"

class AnimationCamera :public CameraBase
{
public:

	void Init()override;
	void Update()override;
	void PostUpdate()override;

	void AddMatrix(const std::vector<Math::Matrix>& mats);

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

private:

	Math::Vector3
	

};