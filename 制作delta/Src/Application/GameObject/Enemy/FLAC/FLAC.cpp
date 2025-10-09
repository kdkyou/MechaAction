#include "FLAC.h" 

#include "../../Weapon/Gun/Rifle/Rifle.h"
#include "../../../Scene/SceneManager.h"

void FLAC::Init()
{
	m_name = "FLAC";

	/*m_hp = 5800;
	
	auto rifle = std::make_shared<Rifle>();
	rifle->SetParent(m_wpThis.lock());
	rifle->SetAttackTrigger(WeaponBase::LeftHand);
	rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
	rifle->SetGunsParam("Asset/Models/Weapon/LinearRifle/LinearRifle.gltf", 0.2f, 2.0f, 0.0f, 1, 36, 250);
	rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", 5.0f, 56, 800, 200, 20.0f, 0.9f);
	rifle->SetBulletsTrailParam("Asset/Textures/GameObject/ClockHand.png", Math::Color(0.7f, 0.4f, 0.1f), 1.7f, 20);
	rifle->SetShotSoundPath("Asset/Sounds/SE/Weapon/Shot_Raifl.wav");
	SceneManager::Instance().AddObject(rifle);
	m_wpWeapons.push_back(rifle);*/

}

void FLAC::Update()
{
}

void FLAC::PostUpdate()
{
}

void FLAC::DrawLit()
{
	if (!m_spModelWork) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld, m_modelColor, m_emissiveColor);

}

void FLAC::OnHit()
{	

}

void FLAC::Editor_ImGui()
{
	CharacterBase::Editor_ImGui();
}

void FLAC::Deserialize(const nlohmann::json& jsonObj)
{
	CharacterBase::Deserialize(jsonObj);
}

void FLAC::Serialize(nlohmann::json& outJson) const
{
	CharacterBase::Serialize(outJson);

	
}
