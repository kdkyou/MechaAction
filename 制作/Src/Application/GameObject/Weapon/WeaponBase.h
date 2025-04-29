#pragma once

class WeaponBase :public KdGameObject
{
public:
	WeaponBase(){}
	~WeaponBase() override{};

	void Update()override;
	virtual void SetParent(std::weak_ptr<KdGameObject>_parent){}

	void DrawLit()override;
	void GenerateDepthMapFromLight()override;

protected:
	
	std::shared_ptr<KdModelWork>	m_spModelWork = nullptr;

	std::shared_ptr<KdAnimator>		m_spAnimator = nullptr;


	Math::Matrix m_mLocalMat = Math::Matrix::Identity;
	Math::Matrix m_parentAttachMat = Math::Matrix::Identity;
};