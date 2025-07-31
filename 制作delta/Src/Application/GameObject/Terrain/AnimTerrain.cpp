#include "AnimTerrain.h"

void AnimTerrain::Init()
{
	m_name = "GroundTerrain";
}

void AnimTerrain::Update()
{
	auto angle = KdToRadians * m_rot;

	m_mWorld = Math::Matrix::CreateScale(m_scale) * Math::Matrix::CreateFromYawPitchRoll(angle) * Math::Matrix::CreateTranslation(m_pos);

}

void AnimTerrain::PostUpdate()
{
	if (m_spModel)
	{
		m_spAnimator->AdvanceTime(m_spModel->WorkNodes());
		m_spModel->CalcNodeMatrices();
	}
}

void AnimTerrain::DrawLit()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void AnimTerrain::SetModel(const std::string& path)
{
	if (!m_spModel) {}
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData(KdAssets::Instance().m_modeldatas.GetData(path));

	m_modelPath = path;

	m_spAnimator = std::make_shared<KdAnimator>();
	m_spAnimator->SetAnimation(m_spModel->GetAnimation("Open"), 10.0f, false);

	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape("Ground", m_spModel, KdCollider::TypeGround);
}

void AnimTerrain::Editor_ImGui()
{
	KdGameObject::Editor_ImGui();


	if (ImGui::Button((const char*)u8"モデルのロード"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			SetModel(filepath);
		}
	}

}

