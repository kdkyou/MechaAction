#pragma once
#include "../CameraBase.h"

class TPSCamera : public CameraBase
{
public:
	TPSCamera()							{}
	~TPSCamera()			override	{}

	void Editor_ImGui()override;

	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;
	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;


	void Init()				override;
	void PostUpdate()		override;
};