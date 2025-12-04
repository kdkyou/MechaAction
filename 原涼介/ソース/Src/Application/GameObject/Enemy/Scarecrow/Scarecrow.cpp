#include "Scarecrow.h"

#include "../../../Scene/SceneManager.h"

void Scarecrow::Init()
{
	m_name = "Scarecrow";

	m_correctionMat = Math::Matrix::CreateTranslation(m_correction);

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape("Enemy", m_spModelWork, KdCollider::TypeDamage);

}

void Scarecrow::Update()
{
	if (m_hp <= 0.0f)
	{
		m_isDestroy = true;
	}
	if (m_isDestroy) {
		m_duration -= KdFPSController::GetInstance().GetDeltaTime();
		if (m_duration <= 0.0f)
		{
			m_isExpired = true;
		}
	}
}

void Scarecrow::PostUpdate()
{
	UpdateCollision();
}

void Scarecrow::DrawLit()
{
	if (!m_spModelWork) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld, m_modelColor, m_emissiveColor);
}

void Scarecrow::OnHit()
{
	if (m_hp <= 0.0f)
	{
		m_parameter = 0;
		m_isDestroy = true;
	}
}

void Scarecrow::Editor_ImGui()
{
	CharacterBase::Editor_ImGui();
}

void Scarecrow::Deserialize(const nlohmann::json& jsonObj)
{
	CharacterBase::Deserialize(jsonObj);
}

void Scarecrow::Serialize(nlohmann::json& outJson) const
{
	CharacterBase::Serialize(outJson);
}

void Scarecrow::UpdateRotate(const Math::Vector3& srcMoveVec)
{

}

void Scarecrow::UpdateCollision()
{
	DirectX::BoundingOrientedBox box;

	box.Center = GetPos() + Math::Vector3(0.0f, 6.0f, 0.0f);
	box.Extents = { 2.0f,5.0f,2.0f };
	UINT type = KdCollider::TypeDamage;
	KdCollider::BoxInfo boxInfo(type, box);

	auto translation = m_mWorld.Translation();

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (obj->Intersects(boxInfo, nullptr))
		{
			if (obj->GetTag() == tPlayerAttack)
			{
				obj->OnHit();
				m_hitDir = GetPos() - obj->GetMatrix().Translation();
				m_hitDir.Normalize();
				HitDamage(obj->GetParameter());
				m_parameter = obj->GetParameter();
				OnHit();
			}
		}
	}
}

bool Scarecrow::Search(bool areaOnly)
{
	return false;
}
