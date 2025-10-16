#include "UIManager.h"

//#include "UIBase.h"
#include "DrawUI/DrawUI.h"
#include "NumberUI/NumberUI.h"
#include "GuageUI/GuageUI.h"
#include "BlinkUI/BlinkUI.h"
#include "MapUI/MapUI.h"


#include "../Weapon/Gun/GunBase.h"

#include "../../Scene/SceneManager.h"


void UIManager::UIInit()
{
	m_fade = std::make_shared<Fade>();
	m_fade->Init();
}

void UIManager::SceneUICreate()
{
	std::string str = "";

	auto sceneType = SceneManager::Instance().GetSceneType();
	switch (sceneType)
	{
	case SceneManager::SceneType::Title:
		str = "Asset/Data/UI/TitleUI.scene";
		break;
	case SceneManager::SceneType::Game:
		str = "Asset/Data/UI/GameUI.scene";
		break;
	default:
		break;
	}

	m_uiList.clear();

	Deserialize(str);
}

void UIManager::SceneUICreate(const std::string& path)
{
	if (path == "") { return; }

	m_uiList.clear();

	Deserialize(path);
}

void UIManager::SceneUIAdd(const std::string& path)
{
	if (path == "") { return; }
	Deserialize(path);
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

	if (m_fade)
	{
		m_fade->PreUpdate();
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
	if (m_leftWeaponOneUI.lock())
	{
		m_leftWeaponOneUI.lock()->SetNumber(m_playerLeftWeaponAmmoOne);
	}
	if (m_rightWeaponOneUI.lock())
	{
		m_rightWeaponOneUI.lock()->SetNumber(m_playerRightWeaponAmmoOne);
	}
	
	for (auto& obj : m_uiList)
	{
		obj->Update();
	}

	if (m_fade)
	{
		m_fade->Update();
	}

}

void UIManager::DrawSprite()
{
	for (auto& obj : m_uiList)
	{
		obj->DrawSprite();
	}

	if (m_fade)
	{
		m_fade->DrawSprite();
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
			str = "Asset/Data/UI/TitleUI.scene";
			break;
		case SceneManager::SceneType::Game:
			str = "Asset/Data/UI/GameUI.scene";
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
		if (ImGui::MenuItem((const char*)u8"シーン呼び出し"))
		{
			m_uiList.clear();

			Deserialize(str);

		}

		if (ImGui::MenuItem((const char*)u8"読み込み"))
		{
			m_uiList.clear();

			std::string filepath;
			if (EditorData::GetInstance().OpenFileDialog(filepath))
			{
				Deserialize(filepath);
			}
		}

		ImGui::EndMenu();
	}
	
	if (ImGui::BeginMenu((const char*)u8"UI作成"))
	{
		const char* alignStr[] = { "None","Draw", "Number","Guage","Blink","Map"};
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
	std::shared_ptr<GuageUI> guage;
	std::shared_ptr<BlinkUI> blink;
	std::shared_ptr<MapUI> map;
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
	case UIManager::CreateType::Guage:
		guage = std::make_shared<GuageUI>();
		guage->Init();
		AddUI(guage);
		break;
	case UIManager::CreateType::Blink:
		blink = std::make_shared<BlinkUI>();
		blink->Init();
		AddUI(blink);
		break;
	case UIManager::CreateType::Map:
		map = std::make_shared<MapUI>();
		map->Init();
		AddUI(map);
		break;
	default:
		break;
	}

}

void UIManager::AddUI(std::shared_ptr<KdGameObject> obj)
{
	m_uiList.push_back(obj); 
}

void UIManager::SetFade(Fade::FadeType type, float time, bool OutorIn)
{
	m_fade->SetFade(type, time, OutorIn);
}

const bool UIManager::IsFadeComplete() const
{
	return m_fade->IsCompleteFade();
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
					obj->Deserialize(json);
					obj->Init();
					AddUI(obj);	
				}

				if (str == "NumberUI")
				{
					auto obj = std::make_shared<NumberUI>();
					obj->Deserialize(json);
					obj->Init();
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
					else if (tag == "PlayerRightShoulderOnce")
					{
						m_rightWeaponOneUI = obj;
					}
					else if (tag == "PlayerLeftShoulderOnce")
					{
						m_leftWeaponOneUI = obj;
					}

				}

				if (str == "GuageUI")
				{
					auto obj = std::make_shared<GuageUI>();
					obj->Init();
					obj->Deserialize(json);
					AddUI(obj);
				}

				if (str == "BlinkUI")
				{
					auto obj = std::make_shared<BlinkUI>();
					obj->Init();
					obj->Deserialize(json);
					AddUI(obj);
				}

				if (str == "MapUI")
				{
					auto obj = std::make_shared<MapUI>();
					obj->Init();
					obj->Deserialize(json);
					AddUI(obj);
				}
				
			}
		}
	}
	
}
