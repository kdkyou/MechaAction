#pragma once

#include "../CameraBase.h"

class LookAtCamera : public CameraBase
{
public:

	void Init()override;

	void PostUpdate()override;


	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

private:

	void Look();
};