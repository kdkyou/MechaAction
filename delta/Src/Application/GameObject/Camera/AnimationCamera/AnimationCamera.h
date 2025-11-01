#pragma once

#include"../CameraBase.h"

class AnimationCamera :public CameraBase
{
public:

	enum CameraMoveType {
		Concurrent,
		RotToPos,
		PosToRot,
	};

	void Init()override;
	void Update()override;
	void PostUpdate()override;

	virtual void Editor_ImGui() override;
	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override;

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override;

private:

	Math::Vector3 m_firstPos;
	Math::Vector3 m_firstRot;

	Math::Vector3 m_SecondPos;
	Math::Vector3 m_SecondRot;
	
	float m_duration = 0.0f;
	float m_animTime = 0.0f;

	bool m_flg = false;

	CameraMoveType m_type = CameraMoveType::Concurrent;
};