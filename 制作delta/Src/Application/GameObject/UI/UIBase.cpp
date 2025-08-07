#include "UIBase.h"

void UIBase::Init()
{
}

void UIBase::DrawSprite()
{
	if (m_spTex)
	{

		KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex,m_pos.x,m_pos.y, m_drawWi, m_drawHe,&m_rect);
	}
}

void UIBase::Editor_ImGui()
{
	KdGameObject::Editor_ImGui();
	char buf[64];
	strncpy(buf, m_uiTag.c_str(), sizeof(buf));
	if (ImGui::InputText("Tag", buf, IM_ARRAYSIZE(buf)))
	{
		m_uiTag = buf;
	}

	ImGui::DragInt((const char*)u8"描画範囲：X", &m_drawWi, 1, 0);
	ImGui::DragInt((const char*)u8"描画範囲：Y", &m_drawHe, 1, 0);
	ImGui::DragInt((const char*)u8"切り取り位置:X", &m_rectX, 1.0);
	ImGui::DragInt((const char*)u8"切り取り位置:Y", &m_rectY, 1.0);
	ImGui::DragInt((const char*)u8"切り取り範囲:X", &m_rectWi, 1.0);
	ImGui::DragInt((const char*)u8"切り取り範囲:Y", &m_rectHe, 1.0);


	m_rect = { m_rectX,m_rectY,m_rectWi,m_rectHe };


if (ImGui::Button((const char*)u8"テクスチャのロード"))
{
	std::string filepath;
	if (EditorData::GetInstance().OpenFileDialog(filepath))
	{
		SetTexture(filepath);
	}
}

}

void UIBase::SetTexture(const std::string& path)
{
	if (!m_spTex)
	{
		m_spTex = KdAssets::Instance().m_textures.GetData(path);

		m_fileName = path;

		m_drawWi = m_spTex->GetWidth();
		m_drawHe = m_spTex->GetHeight();

	

		m_rectWi = m_spTex->GetWidth();
		m_rectHe = m_spTex->GetHeight();
	}
}