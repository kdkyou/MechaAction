#pragma once

#include"../CameraBase.h"

class TrackingCamera : public CameraBase
{
public:
	TrackingCamera() {}
	~TrackingCamera()			override {}

	void Init()				override;
	void PostUpdate()		override;

	
	void Editor_ImGui()		override;

	void SetBasePos(Math::Vector3& _pos) { m_basePos = _pos; }

private:

	void CheckScreenPull();


	Math::Vector3 m_basePos = {3.0f, 14.5f, -30.0f};	//トラッキング時の始点
	Math::Vector3 m_target;		//注視点
	float m_speed = 5.0f;

	float m_unEnableChangeTime = 0.5f;

	bool m_isPull = false;

};