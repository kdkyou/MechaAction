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
}

void DrawTerrain::DrawUnLit()
{

}

void DrawTerrain::SetModel(const std::string& path)
{
}

void DrawTerrain::Editor_ImGui()
{
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

	ImGui::DragFloat3((const char*)u8"エミッシブ", &m_emmisive.x, 0.1f, 0.0f, 10.0f);
}



void DrawTerrain::AnimationPlay(const std::string& path)
{

}