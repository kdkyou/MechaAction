#include "NumberUI.h"

void NumberUI::Init()
{
	m_name = "NumberUI";

	/*SetTexture("Asset/Textures/UI/Number3.png");
	m_drawWi = kDigitWidth;
	m_drawHe = kDigitHeight;
	m_rect = { 0,0,kDigitWidth,kDigitHeight };*/

}

void NumberUI::Update()
{
}

void NumberUI::PostUpdate()
{
}

void NumberUI::DrawSprite()
{
	DrawNumber();
}

void NumberUI::Editor_ImGui()
{
	UIBase::Editor_ImGui();

	ImGui::DragInt("Number", &m_number);
	ImGui::Checkbox("Padding", &m_isPadding);
	ImGui::DragInt("Max Digit", &m_maxDigit, 1, 1, 10);
	ImGui::DragFloat("Spacing", &m_spacing, 1.0f, 0.0f, 100.0f);

	const char* alignStr[] = { "Left", "Right","Center" };
	int alignIndex = static_cast<int>(m_align);
	if (ImGui::Combo("Align", &alignIndex, alignStr, IM_ARRAYSIZE(alignStr)))
	{
		m_align = static_cast<AlignType>(alignIndex);
	}
}

void NumberUI::Deserialize(const nlohmann::json& jsonObj)
{
	UIBase::Deserialize(jsonObj);

	KdJsonUtility::GetValue(jsonObj, "Padding", &m_isPadding);
	KdJsonUtility::GetValue(jsonObj, "Spacing", &m_spacing);
	KdJsonUtility::GetValue(jsonObj, "Align", &m_align);
	KdJsonUtility::GetValue(jsonObj, "MaxDigit", &m_maxDigit);
}

void NumberUI::Serialize(nlohmann::json& outJson) const
{
	UIBase::Serialize(outJson);

	outJson["Padding"]	= m_isPadding;
	outJson["Spacing"]	= m_spacing;
	outJson["Align"]	= m_align;
	outJson["MaxDigit"] = m_maxDigit;
}

void NumberUI::DrawNumber()
{
	std::string strNum = std::to_string(m_number);
	if (m_isPadding)
	{
		// 左パディング
		while (strNum.length() < m_maxDigit)
		{
			strNum.insert(0, "0");
		}
	}


	float totalWidth = strNum.size() * m_spacing;
	Math::Vector2 drawPos = { m_pos.x,m_pos.y };

	switch (m_align)
	{
	case NumberUI::AlignType::Left:
		break;
	case NumberUI::AlignType::Right:
		drawPos.x -= totalWidth;
		break;
	case NumberUI::AlignType::Center:
		drawPos.x -= totalWidth * 0.5f;
		break;
	default:
		break;
	}

	for (size_t i = 0; i < strNum.size(); ++i)
	{
		int digit = strNum[i] - '0';

		m_rect.x = digit * kDigitWidth;
		m_rect.width = m_rectWi;
		m_rect.height = m_rectHe;

		Math::Vector2 pos = drawPos + Math::Vector2(i * m_spacing, 0);

		//KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);
		KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, (int)pos.x, (int)pos.y, m_drawWi, m_drawHe, &m_rect,&m_color);
		//KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
	}
}
