#include "AnimTerrain.h"
#include "DrawTerrain.h"

void AnimTerrain::Init()
{
	m_name = "AnimTerrain";
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
		m_spAnimator->AdvanceTime(m_spModel->WorkNodes(),m_animSpeed);
		m_spModel->CalcNodeMatrices();
	}
}

void AnimTerrain::DrawLit()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld,m_modelColor,m_emmisive);
}

void AnimTerrain::SetModel(const std::string& path)
{
	if (!m_spModel) {}
	m_spModel = std::make_shared<KdModelWork>();
	m_spModel->SetModelData(path);

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
				AnimationPlay(m_animPath);
			}
		}

		ImGui::DragFloat((const char*)u8"アニメーション速度", &m_animSpeed, 0.1f, 0.0f, 100.0f);
	}

	ImGui::ColorEdit4((const char*)u8"カラー", &m_modelColor.x);

	ImGui::DragFloat3((const char*)u8"エミッシブ", &m_emmisive.x,0.1f,0.0f,10.0f);

	static PointLight point;

	ImGui::DragFloat3((const char*)u8"ポイントpos", &point.Pos.x,0.01f,0.0f,100.0f);
	ImGui::DragFloat((const char*)u8"ポイント半径", &point.Radius,0.1f,0.0f,100.0f);
	ImGui::DragFloat3((const char*)u8"ポイントカラー", &point.Color.x,0.1f,0.0f,1.0f);

	if(ImGui::Button((const char*)u8"ポイントライト作成"))
	{
		point.IsBright = 1;
		KdShaderManager::Instance().WorkAmbientController().AddPointLight(point);
		m_points.push_back(point);
	}

}


void AnimTerrain::AnimationPlay(const std::string& path)
{
	m_spAnimator->SetAnimation(m_spModel->GetAnimation(path), 10.0f, false);
}

