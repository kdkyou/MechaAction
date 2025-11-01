#pragma once

#include "../Effect.h"

class ScaleEffect :public Effect
{
public:
	ScaleEffect(){}
	~ScaleEffect()override{}

	void Init()override;

	void Update()override;
	void PostUpdate()override;
	void DrawUnLit()override;

	void SetMatrix(const Math::Matrix& mat) { m_mWorld = mat; }
	void SetRotation(const Math::Vector3& rot) { m_rot = rot; }
	void SetPolygonParam(const std::string& texPath, float scale, float alive);

private:

	float m_scale = 3.0f;
	float m_aliveTime = 0.0f;

};