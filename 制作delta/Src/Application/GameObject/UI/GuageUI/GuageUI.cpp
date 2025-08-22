#include "GuageUI.h"

void GuageUI::Init()
{
	m_name = "GuageUI";

	SetTexture("Asset/Textures/UI/Guage.png");

	m_drawWi = m_spTex->GetInfo().Width;
	m_drawHe = m_spTex->GetInfo().Height;

	m_rectWi = m_drawWi;
	m_rectHe = m_drawHe;

	m_rect = { 0, 0, m_rectWi, m_rectHe };

	m_fillDir = FillDirection::TopToBottom;
}

void GuageUI::Update()
{
	if (m_isIncDec)
	{
		m_duration += KdFPSController::GetInstance().GetDeltaTime();
	}
	else {
		m_duration -= KdFPSController::GetInstance().GetDeltaTime();
	}

	m_ratio = 1.0f / m_time * m_duration;
	if (m_ratio > 1.0f)
	{
		m_ratio = 1.0f;
	}
}

void GuageUI::DrawSprite()
{
	DrawGuage();
}

void GuageUI::Editor_ImGui()
{
	UIBase::Editor_ImGui();

	ImGui::DragInt("Current", &m_current, 1, 0, m_max);
	ImGui::DragInt("Max", &m_max, 1, 1, 999);
	ImGui::Checkbox("Horizontal", &m_isHorizontal);
	ImGui::Checkbox("Reverse", &m_isReverse);

	static const char* dirNames[] = { "Left->Right", "Right->Left", "Bottom->Top", "Top->Bottom" };
	int dir = static_cast<int>(m_fillDir);
	if (ImGui::Combo("Fill Dir", &dir, dirNames, IM_ARRAYSIZE(dirNames)))
	{
		m_fillDir = static_cast<FillDirection>(dir);
	}

	ImGui::SliderFloat("Ammo Ratio", &m_ratio, 0.0f, 1.0f);
	ImGui::SliderFloat("Time", &m_time, 0.0f, 10.0f);
	ImGui::Checkbox("IncDec", &m_isIncDec);
}

void GuageUI::Deserialize(const nlohmann::json& jsonObj)
{
	UIBase::Deserialize(jsonObj);
	KdJsonUtility::GetValue(jsonObj, "FillDirection", &m_fillDir);
	KdJsonUtility::GetValue(jsonObj, "Ratio", &m_ratio);
	KdJsonUtility::GetValue(jsonObj, "Time", &m_time);
	KdJsonUtility::GetValue(jsonObj, "IncDec", &m_isIncDec);
	KdJsonUtility::GetValue(jsonObj, "Auto", &m_isAuto);
}

void GuageUI::Serialize(nlohmann::json& outJson) const
{
	UIBase::Serialize(outJson);

	outJson["FillDirection"] = m_fillDir;
	outJson["Ratio"] = m_ratio;
	outJson["Time"] = m_time;
	outJson["IncDec"] = m_isIncDec;
	outJson["Auto"] = m_isAuto;
}



void GuageUI::DrawGuage()
{
	if (m_spTex == nullptr) { return; }

	if (m_max <= 0) { return; }

	m_rect = { m_rectX,m_rectY,m_rectWi,m_rectHe };

	Math::Rectangle uvRect = m_rect;
	Math::Vector2 drawPos = { m_pos.x, m_pos.y };
	int drawW = m_drawWi;
	int drawH = m_drawHe;

	switch (m_fillDir)
	{
	case FillDirection::LeftToRight:
		uvRect.width = static_cast<int>(m_rectWi * m_ratio);
		drawW = static_cast<int>(m_drawWi * m_ratio);
		break;

	case FillDirection::RightToLeft:
		uvRect.x = m_rect.x + static_cast<int>(m_rectWi * (1.0f - m_ratio));
		uvRect.width = static_cast<int>(m_rectWi * m_ratio);
		drawPos.x -= m_drawWi * (1.0f - m_ratio);
		drawW = static_cast<int>(m_drawWi * m_ratio);
		break;

	case FillDirection::BottomToTop:
		uvRect.y = m_rect.y + static_cast<int>(m_rectHe * (1.0f - m_ratio));
		uvRect.height = static_cast<int>(m_rectHe * m_ratio);
		drawPos.y += m_drawHe * (1.0f - m_ratio);
		drawH = static_cast<int>(m_drawHe * m_ratio);
		break;

	case FillDirection::TopToBottom:
		uvRect.height = static_cast<int>(m_rectHe * m_ratio);
		drawH = static_cast<int>(m_drawHe * m_ratio);
		break;
	}



	auto& sm = KdShaderManager::Instance();

	sm.ChangeBlendState(KdBlendState::Add);
	sm.m_spriteShader.DrawTex(m_spTex, drawPos.x, drawPos.y, drawW, drawH, &m_rect,&m_color,m_pivot);
	sm.ChangeBlendState(KdBlendState::Alpha);

}


