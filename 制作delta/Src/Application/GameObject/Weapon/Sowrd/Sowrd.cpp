#include"Sowrd.h"

#include "../../Camera/CameraManager.h"

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

void Sowrd::OnHit()
{
	if (m_attackNum > 0)
	{
		auto pos = m_mWorld.Translation() + m_endMat.Translation();
	//	KdEffekseerManager::GetInstance().Play("Slash.efkefc", pos,1.0f,3.0f,false);
		KdAudioManager::Instance().Play("Asset/Sounds/Sound/sword_hit.wav", false);
		m_parameter = m_damage;
		m_attackNum--;
		m_pCollider->SetEnableAll(false);

		CameraManager::Instance().SetNextType(CameraManager::Hit);
	}
}

void Sowrd::Editor_ImGui()
{
	KdGameObject::Editor_ImGui();
}

void Sowrd::Deserialize(const nlohmann::json& jsonObj)
{
	KdGameObject::Deserialize(jsonObj);
}

void Sowrd::Serialize(nlohmann::json& outJson) const
{
	KdGameObject::Serialize(outJson);
}

void Sowrd::Init()
{
	if (!m_spTrail)
	{
		m_spTrail = std::make_shared<KdTrailPolygon>();
		m_spTrail->SetMaterial(KdAssets::Instance().m_textures.LoadData("Asset/Textures/GameObject/Thunder1.png"));
		m_spTrail->SetColor(Math::Color{ 0.2f,0.3f,2.0f });
		m_spTrail->SetPattern(KdTrailPolygon::Trail_Pattern::eBillboard);
		m_spTrail->SetWidth(2.3f);
		m_spTrail->SetLength(10);
		m_spTrail->ClearPoints();
	}
	
	if (!m_spTrail2)
	{
		m_spTrail2 = std::make_shared<KdTrailPolygon>();
		m_spTrail2->SetMaterial(KdAssets::Instance().m_textures.LoadData("Asset/Textures/GameObject/Slash.png"));
		m_spTrail2->SetColor(Math::Color{ 0.3f,0.3f,2.0f });
		m_spTrail2->SetPattern(KdTrailPolygon::Trail_Pattern::eBillboard);
		m_spTrail2->SetWidth(5.3f);
		m_spTrail2->SetLength(20);
		m_spTrail2->ClearPoints();
	}

	m_pCollider = std::make_unique<KdCollider>();

	DirectX::BoundingBox box;
	box.Center = { 0.0f,5.0f,9.0f };
	box.Extents = { 10.0f, 9.0f, 10.0f };

	m_pCollider->RegisterCollisionShape("Sowrd", box, KdCollider::TypeDamage);

	m_pCollider->SetEnableAll(false);

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_name = "Sword";
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
			m_mParentAttach = _pNode->m_worldTransform;
		}

		_parentMat = _spParent->GetMatrix();

	}


	m_mWorld = m_mParentAttach * _parentMat;

	if (_spParent->IsRightAttack())
	{
		if (m_isOnece == true)
		{
			m_spTrail->SetEnable(true);
			m_spTrail2->SetEnable(true);
			m_isOnece = false;
			KdAudioManager::Instance().Play("Asset/Sounds/Sound/sword_swing.wav", false);

		}

		
		if (m_attackNum > 0)
		{
			m_pCollider->SetEnableAll(true);
		}

		if (!_spParent->IsEnableAttack())
		{
			m_pCollider->SetEnableAll(false);
		}



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

		m_spTrail2->AddPoint(m_endMat * m_mWorld);
	}

	else {
		m_spTrail->ClearPoints();
		m_spTrail->SetEnable(false);

		m_attackNum = m_maxAttackNum;

		m_spTrail2->ClearPoints();
		m_spTrail2->SetEnable(false);

		m_pCollider->SetEnableAll(false);

		
		m_isOnece = true;
	}

	auto mat =  Math::Matrix::CreateTranslation({ 0.0f,5.0f,9.0f })*m_mWorld;
	//m_pDebugWire->AddDebugBox(mat, Math::Vector3(10.0f, 9.0f, 10.0f));

	WeaponBase::Update();
}

void Sowrd::DrawUnLit()
{

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	{
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spTrail);
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spTrail2);
	}

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
}


