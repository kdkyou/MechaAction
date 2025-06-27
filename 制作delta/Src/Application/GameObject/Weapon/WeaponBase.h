#pragma once



class WeaponBase :public KdGameObject
{
public:
	WeaponBase(){}
	~WeaponBase() override{};

	void SetParent(std::weak_ptr<KdGameObject>_parent) { m_wpParent = _parent; }

	void SetAttachModel(const std::string& modelPath, const std::string& AttachPath);

	void DrawLit()override;
	void GenerateDepthMapFromLight()override;

protected:
	
	std::weak_ptr<KdGameObject>    m_wpParent;

	std::shared_ptr<KdModelWork>	m_spModelWork = nullptr;

	std::shared_ptr<KdAnimator>		m_spAnimator = nullptr;


	Math::Matrix m_mLocalMat = Math::Matrix::Identity;
	Math::Matrix m_parentAttachMat = Math::Matrix::Identity;

	std::shared_ptr<KdModelWork::Node>	m_pNode;

	std::string m_attachPath = "";
};