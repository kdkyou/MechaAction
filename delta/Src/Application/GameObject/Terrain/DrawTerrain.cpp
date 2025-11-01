#include "DrawTerrain.h"

void DrawTerrain::Init()
{
}

void DrawTerrain::Update()
{
}

void DrawTerrain::PostUpdate()
{
}

void DrawTerrain::DrawLit()
{
	if (!m_spModel) return;

	for (auto& point : m_points)
	{
		KdShaderManager::Instance().WorkAmbientController().AddPointLight(point);
	}

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld, m_modelColor, m_emmisive);
}

void DrawTerrain::SetModel(const std::string& path)
{
	if (!m_spModel) {}
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData(path);

	m_modelPath = path;



	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape("Ground", m_spModel, KdCollider::TypeGround);
}

void DrawTerrain::CreateAnimator()
{
	m_spAnimator = std::make_shared<KdAnimator>();
}

void DrawTerrain::Editor_ImGui()
{
	KdGameObject::Editor_ImGui();

	ImGui::Text((const char*)m_modelPath.c_str());

	if (ImGui::Button((const char*)u8"モデルのロード"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			SetModel(filepath);
		}
	}

	if (m_spModel)
	{

		ImGui::InputText((const char*)u8"アニメーションパス", &m_animPath);

		if (m_animPath != "")
		{
			if (ImGui::Button((const char*)u8"アニメーション再生"))
			{
				CreateAnimator();
				AnimationPlay(m_animPath);
			}
		}

		ImGui::DragFloat((const char*)u8"アニメーション速度", &m_animSpeed, 0.1f, 0.0f, 100.0f);
	}

	ImGui::ColorEdit4((const char*)u8"カラー", &m_modelColor.x);

	ImGui::DragFloat3((const char*)u8"エミッシブ", &m_emmisive.x, 0.1f, 0.0f, 100.0f);

	static PointLight point;

	ImGui::DragFloat3((const char*)u8"ポイントpos", &point.Pos.x, 0.01f, -100.0f, 100.0f);
	ImGui::DragFloat((const char*)u8"ポイント半径", &point.Radius, 0.1f, -100.0f, 100.0f);
	ImGui::DragFloat3((const char*)u8"ポイントカラー", &point.Color.x, 0.1f, 0.0f, 1.0f);

	if (ImGui::Button((const char*)u8"ポイントライト作成"))
	{
		point.IsBright = true;
		KdShaderManager::Instance().WorkAmbientController().AddPointLight(point);
		m_points.push_back(point);
	}
}



void DrawTerrain::AnimationPlay(const std::string& path)
{
	m_spAnimator->SetAnimation(m_spModel->GetAnimation(path), 10.0f, false);
}