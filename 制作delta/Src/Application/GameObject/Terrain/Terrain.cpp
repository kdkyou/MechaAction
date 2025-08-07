#include "Terrain.h"

void Terrain::Init()
{
	m_name = "GroundTerrain";

}

void Terrain::Update()
{
	auto angle = KdToRadians * m_rot;

	m_mWorld = Math::Matrix::CreateScale(m_scale) * Math::Matrix::CreateFromYawPitchRoll(angle) * Math::Matrix::CreateTranslation(m_pos);

}

void Terrain::DrawLit()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void Terrain::SetModel(const std::string& path)
{
	m_spModel = KdAssets::Instance().m_modeldatas.GetData(path);

	m_modelPath = path;

	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape("Ground", m_spModel, KdCollider::TypeGround);
}

void Terrain::Editor_ImGui()
{
	KdGameObject::Editor_ImGui();

	ImGui::Text((const char*)"アセットパス:%s", m_modelPath);

	if (ImGui::Button((const char*)u8"モデルのロード"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			SetModel(filepath);
		}
	}

}

