#include "UIManager.h"

//#include "UIBase.h"
#include "DrawUI/DrawUI.h"
#include "NumberUI/NumberUI.h"
#include "../Weapon/Gun/GunBase.h"

#include "../../Scene/SceneManager.h"


void UIManager::UIInit()
{

}

void UIManager::SceneUICreate()
{
	std::string str = "";

	auto sceneType = SceneManager::Instance().GetSceneType();
	switch (sceneType)
	{
	case SceneManager::SceneType::Title:
		str = "Asset/Data/TitleUI.Scene";
		break;
	case SceneManager::SceneType::Game:
		str = "Asset/Data/GameUI.Scene";
		break;
	default:
		break;
	}

	m_uiList.clear();

	Deserialize(str);
}

void UIManager::PreUpdate()
{
	auto it = m_uiList.begin();

	while (it != m_uiList.end())
	{
		if ((*it)->IsExpired())	// IsExpired() ・・・ 無効ならtrue
		{
			// 無効なオブジェクトをリストから削除
			it = m_uiList.erase(it);
		}
		else
		{
			++it;	// 次の要素へイテレータを進める
		}
	}

	for (auto& obj : m_uiList)
	{
		obj->PreUpdate();
	}
}

void UIManager::Update()
{
	if (m_hpUI.lock()) {
		m_hpUI.lock()->SetNumber(m_playerHP);
	}
	if (m_leftWeaponUI.lock())
	{
		m_leftWeaponUI.lock()->SetNumber(m_playerLeftWeaponAmmo);
	}
	if (m_rightWeaponUI.lock())
	{
		m_rightWeaponUI.lock()->SetNumber(m_playerRightWeaponAmmo);
	}
	
	for (auto& obj : m_uiList)
	{
		obj->Update();
	}

}

void UIManager::DrawSprite()
{
	for (auto& obj : m_uiList)
	{
		obj->DrawSprite();
	}
}

void UIManager::Editor_ImGui()
{
	nlohmann::json outJson;
	if (ImGui::BeginMenu((const char*)u8"シーンUI"))
	{
		std::string str = "";

		auto sceneType = SceneManager::Instance().GetSceneType();
		switch (sceneType)
		{
		case SceneManager::SceneType::Title:
			str = "Asset/Data/TitleUI.Scene";
			break;
		case SceneManager::SceneType::Game:
			str = "Asset/Data/GameUI.Scene";
			break;
		default:
			break;
		}

		if (ImGui::MenuItem((const char*)u8"保存"))
		{
			for (auto obj : m_uiList)
			{
				nlohmann::json json;
				obj->Serialize(json);
				outJson.push_back(json);
			}

			

			std::ofstream ofs(str);
			if (ofs.is_open())
			{
				ofs << outJson.dump();
			}
		}
		if (ImGui::MenuItem((const char*)u8"呼び出し"))
		{
			m_uiList.clear();

			Deserialize(str);
		}

		ImGui::EndMenu();
	}
	
	if (ImGui::BeginMenu((const char*)u8"UI作成"))
	{
		const char* alignStr[] = { "None","Draw", "Number" };
		int alignIndex = static_cast<int>(m_nowCreateType);
		if (ImGui::Combo("Create", &alignIndex, alignStr, IM_ARRAYSIZE(alignStr)))
		{
			m_nowCreateType = static_cast<CreateType>(alignIndex);
		}
		if (ImGui::Button((const char*)u8"作成"))
		{
			CreateUI();
		}
		ImGui::EndMenu();
	}


	for (auto& obj : m_uiList)
	{
		ImGui::PushID(obj.get());
		if (ImGui::CollapsingHeader(obj->GetName().c_str()))
		{
			obj->Editor_ImGui();
		}
		ImGui::PopID();
	}
}

void UIManager::CreateUI()
{
	std::shared_ptr<DrawUI> draw; 
	std::shared_ptr<NumberUI> number;
	switch (m_nowCreateType)
	{
	case UIManager::CreateType::No:
		break;
	case UIManager::CreateType::Draw:
		draw= std::make_shared<DrawUI>();
		draw->Init();
		AddUI(draw);
		break;
	case UIManager::CreateType::Number:
		number = std::make_shared<NumberUI>();
		number->Init();
		AddUI(number);
		break;
	default:
		break;
	}

}

void UIManager::AddUI(std::shared_ptr<KdGameObject> obj)
{
	m_uiList.push_back(obj); 
}

void UIManager::Deserialize(const std::string& path)
{
	std::ifstream ifs(path);
	if (ifs.is_open())
	{
		nlohmann::json j;
		ifs >> j;
		for (auto json : j)
		{
			std::string str;
			KdJsonUtility::GetValue(json, "Name", &str);
			if (!str.empty())
			{
				if (str == "DrawUI")
				{
					auto obj = std::make_shared<DrawUI>();
					obj->Init();
					obj->Deserialize(json);
					AddUI(obj);	
				}

				if (str == "NumberUI")
				{
					auto obj = std::make_shared<NumberUI>();
					obj->Init();
					obj->Deserialize(json);
					AddUI(obj);	

					auto& tag = obj->GetUIByTag();
					if (tag == "PlayerHP") {
						m_hpUI = obj;
					}
					else if (tag == "PlayerRightShoulderAmmo")
					{
						m_rightWeaponUI = obj;
					}
					else if (tag == "PlayerLeftShoulderAmmo")
					{
						m_leftWeaponUI = obj;
					}

				}
				
			}
		}
	}
	
}
