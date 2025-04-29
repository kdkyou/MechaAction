#pragma once

class TrackingCamera : public CameraBase
{
public:
	TrackingCamera() {}
	~TrackingCamera()			override {}

	void Init()				override;
	void PostUpdate()		override;

	void SetBasePos(Math::Vector3& _pos) { m_basePos = _pos; }

private:


	Math::Vector3 m_basePos;	//トラッキング時の始点
	Math::Vector3 m_pos;		//カメラの座標
	Math::Vector3 m_target;		//注視点
	float m_speed=10.0f;

};