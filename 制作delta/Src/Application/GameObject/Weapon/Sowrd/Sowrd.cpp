#include"Sowrd.h"

void Sowrd::SetModel(const std::string& path)
{
	if (!m_spModelWork)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData(path);

		const KdModelWork::Node* _pNode = m_spModelWork->FindWorkNode("StartPoint");
		if (_pNode)
		{

			m_startMat = _pNode->m_worldTransform;
		}

		_pNode = m_spModelWork->FindWorkNode("EndPoint");
		if (_pNode)
		{

			m_endMat = _pNode->m_worldTransform;
		}

	}
}

void Sowrd::Init()
{
	if (!m_spTrail)
	{
		m_spTrail = std::make_shared<KdTrailPolygon>();
		m_spTrail->SetMaterial(KdAssets::Instance().m_textures.LoadData("Asset/Textures/GameObject/Thunder1.png"));
		m_spTrail->SetColor(Math::Color{ 0.2f,0.3f,2.0f });
		m_spTrail->SetPattern(KdTrailPolygon::Trail_Pattern::eBillboard);
		m_spTrail->SetWidth(0.3f);
		m_spTrail->SetLength(10);
		m_spTrail->ClearPoints();
	}


}

void Sowrd::Update()
{
	//親(プレイヤー)の行列を取得
	const std::shared_ptr<const CharacterBase> _spParent = m_wpParent.lock();
	Math::Matrix _rotMat = Math::Matrix::Identity;
	Math::Matrix _parentMat = Math::Matrix::Identity;

	if (_spParent)
	{
		const KdModelWork::Node* _pNode = _spParent->GetModelWork().lock()->FindWorkNode(m_attachPath);
		if (_pNode)
		{
			m_parentAttachMat = _pNode->m_worldTransform;
		}

		_parentMat = _spParent->GetMatrix();

	}


	m_mWorld = m_parentAttachMat * _parentMat;

	if (_spParent->IsRightAttack())
	{
		m_spTrail->SetEnable(true);


		static bool flg = false;
		if (flg == false)
		{
			m_spTrail->AddPoint(m_startMat * m_mWorld);
			flg = true;
		}
		else
		{
			m_spTrail->AddPoint(m_endMat * m_mWorld);
			flg = false;
		}
	}

	else {
		m_spTrail->ClearPoints();
		m_spTrail->SetEnable(false);
	}

	WeaponBase::Update();
}

void Sowrd::DrawUnLit()
{

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	{
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spTrail);
	}

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
}


