#pragma once

#include"../CameraBase.h"

class HitCamera :public CameraBase
{
public:

	enum ShakeType
	{
		NoneShake,
		UpDown,
		LefRi,
		Intensely,
	};

	void Init()override;
	void PostUpdate()		override;
	
	void Editor_ImGui()		override;

	void SetShakeType(ShakeType _type) { m_shakeType = _type; }

private:

	ShakeType m_shakeType =Intensely;

	Math::Vector3 m_pos = {};

	Math::Vector3 m_movePow = {};

	float m_duration = 0.0f;
	const float m_limitDuration = 0.2f;

	KdRandomGenerator randam;

};