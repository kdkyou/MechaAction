#pragma once
#include"../CameraBase.h"

class LockCamera : public CameraBase
{
public:
	LockCamera() {}
	~LockCamera()			override {}

	void Init()				override;
	void Update()			override;
	void PostUpdate()		override;

	void DrawUnLit()		override;
	void DrawSprite()		override;

private:

	void Lock();

	std::shared_ptr<KdSquarePolygon> m_spPolygon;
	std::shared_ptr<KdTexture>		 m_spTex;
	Math::Vector3					 m_lockPos = {};

	Math::Vector2					 m_speedRatio = { 0.5f,2.0f };

	float							 m_texAlpha = 1.0f;

	bool							 m_isReduce = false;

	float							 m_texScale = 1.0f;
	float							 m_durationScale = 0.0f;

};