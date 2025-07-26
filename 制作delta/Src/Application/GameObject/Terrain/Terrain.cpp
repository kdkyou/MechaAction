#include "Terrain.h"

void Terrain::Init()
{
	m_name = "GroundTerrain";

}

void Terrain::Update()
{
}

void Terrain::DrawLit()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void Terrain::SetModel(const std::string& path)
{
	if (!m_spModel) {}
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


	if (ImGui::Button((const char*)u8"モデルのロード"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			SetModel(filepath);
		}
	}

}

