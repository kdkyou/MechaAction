#include "Shield.h"

void Shield::SetModel(const std::string& path)
{
	if (!m_spModelWork)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData(path);
	}

	if (!m_spAnimator)
	{
		m_spAnimator = std::make_shared<KdAnimator>();
		m_spAnimator->SetAnimation(m_spModelWork->GetAnimation("Close"), 10.0f,false);
	}

}

void Shield::Editor_ImGui()
{
	WeaponBase::Editor_ImGui();
}

void Shield::Deserialize(const nlohmann::json& jsonObj)
{
	WeaponBase::Deserialize(jsonObj);
}

void Shield::Serialize(nlohmann::json& outJson) const
{
	WeaponBase::Serialize(outJson);
}

void Shield::OnHit()
{
	KdAudioManager::Instance().Play("Asset/Sounds/SE/Weapon/Guard.wav")
		->SetVolume(KdAudioManager::Instance().GetSEVolume());
}

void Shield::CreateShield()
{
	if (!m_spModelWork) { return; }

	std::vector<Math::Vector3> positions;
	std::vector<Math::Vector2> uvs;

	for (int i = 0; i < 5; i++)
	{
		const auto& pNode = m_spModelWork->FindWorkNode(m_pointName[i]);
		if (pNode)
		{
			positions.push_back(pNode->m_worldTransform.Translation());

			if (i < 4)
			{
				uvs.push_back(Math::Vector2((float)(i % 2) , i / 2.0f));
			}
			else {
				uvs.push_back(Math::Vector2(0.5f, 0.5f));
				
			}
		}

	}

	if (positions.size() == 5)
	{
		m_spShieldPoly->SetVertices(positions, uvs);
	}
}

void Shield::ClearShield()
{
	m_spShieldPoly->ClearVertices();
}

void Shield::Init()
{
	
	Math::Vector3 pos = { 0.0f,5.0f,3.0f };
	SetPos(pos);

	m_name = "Shield";

	m_spShieldPoly = std::make_shared<ShieldPolygon>();
	auto material = std::make_shared<KdMaterial>();
	auto baseTex = KdAssets::Instance().m_textures.GetData("Asset/Textures/GameObject/wave.png");
	auto nmlTex = KdAssets::Instance().m_textures.GetData("Asset/Textures/GameObject/wave_nml.png");
	material->SetTextures(baseTex, nullptr, nullptr, nmlTex);
	m_spShieldPoly->SetMaterial(material);

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape("Shield", m_spShieldPoly, KdCollider::TypeGround+KdCollider::TypeDamage);
	m_pCollider->SetEnable("Shield",false);

}

void Shield::Update()
{
	//親(プレイヤー)の行列を取得
	const std::shared_ptr<const CharacterBase> _spParent = m_wpParent.lock();
	Math::Matrix _rotMat = Math::Matrix::Identity;
	Math::Matrix _parentMat = Math::Matrix::Identity;

	auto parent = m_wpParent.lock();
	if (parent)
	{
		if (m_attachPath != "")
		{
			const KdModelWork::Node* _pNode = _spParent->GetModelWork().lock()->FindWorkNode("LeftWeapon");
			if (_pNode)
			{
				m_mParentAttach = _pNode->m_worldTransform;
			}

			_parentMat = _spParent->GetMatrix();
		}
	}

	if (_spParent)
	{
		// 
		if (_spParent->IsLeftAttack()) {
			if (m_animChanged == false)
			{

				m_animChanged = true; 
				m_spAnimator->SetAnimation(m_spModelWork->GetAnimation("Open"), 30.0f, false);
				m_pCollider->SetEnable("Shield",true);
			}
		}
		else
		{
			if (m_animChanged == true)
			{

				m_animChanged = false; 
				m_spAnimator->SetAnimation(m_spModelWork->GetAnimation("Close"), 30.0f, false);
				m_pCollider->SetEnable("Shield",false);
			}

		}
	}

	if (m_animChanged == true)
	{
		CreateShield();
	}
	else
	{
		ClearShield();
	}

	m_mWorld = m_mParentAttach * _parentMat;

}

void Shield::PostUpdate()
{
	m_spAnimator->AdvanceTime(m_spModelWork->WorkNodes(), 60.0f);
}

void Shield::DrawUnLit()
{
	if (m_spShieldPoly == nullptr) { return; }
	KdShaderManager::Instance().m_StandardShader.SetWaveEnable(true);
	float delta = KdFPSController::GetInstance().GetDeltaTime();
	KdShaderManager::Instance().m_StandardShader.SetWaveTime(delta);

	
	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add); // 加算で光らせる
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spShieldPoly,m_mWorld);
	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha); // 元に戻す
	KdShaderManager::Instance().m_StandardShader.SetWaveEnable(false);
}
