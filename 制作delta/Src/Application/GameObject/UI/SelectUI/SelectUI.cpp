#include "SelectUI.h"

#include "../UIManager.h"
#include "../../../Scene/SceneManager.h"

void SelectUI::Init()
{
	m_name = "SelectUI";
	m_isChangeScene = false;
}

void SelectUI::Update()
{
	if (m_dataes.empty() == true) { return; }

	auto& key = KeyInput::GetInstance().GetKeyboardState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	if (m_type == MoveType::TopDown)
	{

		if (key.W || pad.IsLeftThumbStickUp()) {
			m_subscript -= 1;
		}
		
		if (key.S || pad.IsLeftThumbStickDown()) {
			m_subscript += 1;
		}
	}
	else if (m_type == MoveType::LeftRight)
	{
	 if (key.D || pad.IsLeftThumbStickRight()) {
		m_subscript += 1;
	}
	 
	 if (key.S || pad.IsLeftThumbStickLeft()) {
		 m_subscript -= 1;
	 }
	}

	if (m_subscript < 0)
	{
		m_subscript = 0;
	}
	else if (m_subscript >= m_dataes.size() )
	{
		m_subscript = m_dataes.size() - 1;
	}

	int num = 0;
	for (auto& obj : m_dataes)
	{
		if (num == m_subscript)
		{
			m_selectedPos = obj->pos;
			m_selectedScene = obj->sceneType;
			break;
		}
		num++;
	}

	if (key.Space||pad.IsAPressed()) {
		if (!m_isChangeScene)
		{
			m_isChangeScene = true;
		UIManager::GetInstance().SetFade(Fade::FadeIn, 0.2f, true);
		}
	}

	if (m_isChangeScene)
	{
		if (UIManager::GetInstance().IsFadeComplete())
		{
			auto type = static_cast<SceneManager::SceneType>(m_selectedScene);
			SceneManager::Instance().SetNextScene(type);
		}
	}


}

void SelectUI::PostUpdate()
{

}

void SelectUI::DrawSprite()
{
	if (m_spTex)
	{
		m_rect = { 0,0,m_rectWi,m_rectHe };
		KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, (int)m_pos.x + m_selectedPos.x, (int)m_pos.y + m_selectedPos.y, m_drawWi, m_drawHe, &m_rect, &m_color, m_pivot);
	}

	for (auto& obj : m_dataes)
	{
		if (obj->tex) {
			Math::Vector2 pos = { m_pos.x + obj->pos.x ,m_pos.y + obj->pos.y };
			Math::Rectangle rect = { 0,0,(long)obj->tex->GetWidth(), (long)obj->tex->GetHeight() };
			KdShaderManager::Instance().m_spriteShader.DrawTex
			(obj->tex, (int)pos.x, (int)pos.y,
				obj->tex->GetWidth(), obj->tex->GetHeight(), &rect, &obj->color, m_pivot);
		}
	}
}

void SelectUI::Editor_ImGui()
{
	UIBase::Editor_ImGui();

	static const char* dirNames[] = { "None", "TopDown", "LeftRight", "" };
	int dir = static_cast<int>(m_type);
	if (ImGui::Combo("MoveType", &dir, dirNames, IM_ARRAYSIZE(dirNames)))
	{
		m_type = static_cast<MoveType>(dir);
	}

	if(ImGui::Button((const char*)u8"ポイントの追加"))
	{
		auto data = std::make_shared<SelectData>();
		m_dataes.push_back(data);
	}

	for (auto& obj : m_dataes)
	{
		ImGui::PushID(obj.get());
		if (ImGui::CollapsingHeader(obj->path.c_str()))
		{
			if (ImGui::Button((const char*)u8"テクスチャのロード"))
			{
				std::string filepath;
				if (EditorData::GetInstance().OpenFileDialog(filepath))
				{
					obj->tex = KdAssets::Instance().m_textures.GetData(filepath);

					obj->path = filepath;
				}
			}
			ImGui::DragFloat2((const char*)u8"LocalPos", &obj->pos.x,0.1f);

			static const char* dirNames[] = { "Title", "TitleMovie", "Game", "Retry","Training"};
			int dir = obj->sceneType;
			if (ImGui::Combo("SceneType", &dir, dirNames, IM_ARRAYSIZE(dirNames)))
			{
				obj->sceneType = dir;
			}

		}
		ImGui::PopID();
	}

}

void SelectUI::Deserialize(const nlohmann::json& jsonObj)
{
	UIBase::Deserialize(jsonObj);

	KdJsonUtility::GetValue(jsonObj, "MoveType", &m_type);

	if (jsonObj.contains("SelectPoints"))
	{
		for (auto& data : jsonObj["SelectPoints"])
		{
			auto obj= std::make_shared<SelectData>();
			KdJsonUtility::GetValue(data, "Path", &obj->path);
			if (obj->path != "") {
				obj->tex = KdAssets::Instance().m_textures.GetData(obj->path);
			}
			KdJsonUtility::GetValue(data, "SceneType", &obj->sceneType);
			KdJsonUtility::GetArray(data, "Pos", &obj->pos.x,2);

			m_dataes.push_back(obj);
		}
	}
}

void SelectUI::Serialize(nlohmann::json& outJson) const
{
	UIBase::Serialize(outJson);
	outJson["MoveType"] = m_aliveTime;

	nlohmann::json weaponsArray = nlohmann::json::array();

	for (auto& obj : m_dataes)
	{
		if (obj)
		{
			nlohmann::json weaponJson;
			weaponJson["Path"] = obj->path;
			weaponJson["Pos"] = KdJsonUtility::CreateArray(&obj->pos.x,2);
			weaponJson["SceneType"] = obj->sceneType;
			weaponsArray.push_back(weaponJson);
		}
	}

	outJson["SelectPoints"] = weaponsArray;

}
