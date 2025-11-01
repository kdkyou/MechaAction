#include "Terrain.h"

void Terrain::Init()
{
	m_name = "Terrain";

}

void Terrain::Update()
{
	auto angle = KdToRadians * m_rot;

	m_mWorld = Math::Matrix::CreateScale(m_scale) * Math::Matrix::CreateFromYawPitchRoll(angle) * Math::Matrix::CreateTranslation(m_pos);

}

void Terrain::GenerateDepthMapFromLight()
{
	if (!m_spModel) return;
	if (!m_isGenerate)return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void Terrain::DrawLit()
{
	if (!m_spModel) return;
	if (!m_isLit)return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void Terrain::DrawUnLit()
{
	if (!m_spModel) return;
	if (!m_isUnLit)return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void Terrain::DrawBright()
{
	if (!m_spModel) return;
	if (!m_isBright)return;

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

	//ImGui::Text((const char*)"アセットパス:%s", m_modelPath);

	if (ImGui::Button((const char*)u8"モデルのロード"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			SetModel(filepath);
		}
	}

	ImGui::Checkbox((const char*)u8"Generate", &m_isGenerate);
	ImGui::Checkbox((const char*)u8"Lit", &m_isLit);
	ImGui::Checkbox((const char*)u8"UnLit", &m_isUnLit);
	ImGui::Checkbox((const char*)u8"Bright", &m_isBright);

}

