#pragma once

class TransAC :public KdGameObject
{
public:

	void Init()override;
	void Update()override;
	void PostUpdate()override;
	void DrawUnLit()		override;
	void DrawLit()		override;
	void DrawBright()		override;

	void SetTransACData(const std::string& modelPath, const std::string& animPath,float animprogress,const Math::Matrix& mat ,float dissolveProgress,float dissolveSpeed, const Math::Vector3 dissolveColor);

private:

	std::shared_ptr<KdModelWork> m_spModelWork = nullptr;
	std::shared_ptr<KdAnimator>  m_spAnimator = nullptr;

	float						 m_dissolveSpeed = 0.0f;
	float						 m_dissolveProgress = 0.0f;
	Math::Vector3				 m_dissolveColor = {};

};