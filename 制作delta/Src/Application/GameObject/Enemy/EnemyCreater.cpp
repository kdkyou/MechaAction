#include "EnemyCreater.h"

#include "Enemy.h"
#include "Drone/Drone.h"
#include "MT/MT.h"
#include "FLAC/FLAC.h"
#include "Balt/Balt.h"
#include "Scarecrow/Scarecrow.h"


#include "../../Scene/SceneManager.h"

void EnemyCreater::EnemysCreate(const std::string& filePath)
{
	std::ifstream ifs(filePath);
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
				if (str == "Drone")
				{
					auto obj = std::make_shared<Drone>();
					obj->SetThis(obj);
					obj->SetThisBase(obj);
					obj->SetTag(KdGameObject::tEnemy);
					obj->Deserialize(json);
					obj->Init();
					SceneManager::Instance().AddEnemy(obj);
				}

				if (str == "MT")
				{
					auto obj = std::make_shared<MT>();
					obj->SetThis(obj);
					obj->SetTag(KdGameObject::tEnemy);
					obj->SetThisBase(obj);
					obj->Deserialize(json);
					obj->Init();
					SceneManager::Instance().AddEnemy(obj);
				}

				if (str == "Enemy")
				{
					auto obj = std::make_shared<Enemy>();
					obj->SetThis(obj);
					obj->SetThisBase(obj);
					obj->SetTag(KdGameObject::tEnemy);
					obj->Deserialize(json);
					obj->Init();
					SceneManager::Instance().AddEnemy(obj);
				}

				if (str == "Balt")
				{
					auto obj = std::make_shared<Balt>();
					obj->SetThis(obj);
					obj->SetThisBase(obj);
					obj->SetTag(KdGameObject::tEnemy);
					obj->Deserialize(json);
					obj->Init();
					SceneManager::Instance().AddEnemy(obj);
				}

				if (str == "FLAC")
				{
					auto obj = std::make_shared<FLAC>();
					obj->SetThis(obj);
					obj->SetThisBase(obj);
					obj->SetTag(KdGameObject::tEnemy);
					obj->Deserialize(json);
					obj->Init();
					SceneManager::Instance().AddEnemy(obj);
				}

				if (str == "Scarecrow")
				{
					auto obj = std::make_shared<Scarecrow>();
					obj->SetThis(obj);
					obj->SetThisBase(obj);
					obj->SetTag(KdGameObject::tEnemy);
					obj->Deserialize(json);
					obj->Init();
					SceneManager::Instance().AddEnemy(obj);

				}
			}
		}
	}
}

void EnemyCreater::Editor_ImGui()
{

	std::string str = "";

	auto sceneType = SceneManager::Instance().GetSceneType();
	switch (sceneType)
	{
	case SceneManager::SceneType::Title:
		str = "Asset/Data/Enemy/Title.enemy";
		break;
	case SceneManager::SceneType::TitleMovie:
		str = "Asset/Data/Enemy/TitleMovie.enemy";
		break;
	case SceneManager::SceneType::Game:
		str = "Asset/Data/Enemy/Game.enemy";
		break;
	case SceneManager::SceneType::Retry:
		str = "Asset/Data/Enemy/Retry.enemy";
		break;
	case SceneManager::SceneType::Training:
		str = "Asset/Data/Enemy/Training.enemy";
		break;
	default:
		break;
	}

	if (ImGui::Button((const char*)u8"保存"))
	{
		nlohmann::json outJson;

		for (auto obj : SceneManager::Instance().GetEnemyList())
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

	if (ImGui::Button((const char*)u8"エネミー読み込み"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			EnemysCreate(filepath);
		}
	}


	if(ImGui::Button((const char*)u8"Drone"))
	{
		auto obj = std::make_shared<Drone>();
		obj->SetThis(obj);
		obj->SetThisBase(obj);
		obj->Init();
		obj->SetTag(KdGameObject::tEnemy);
		SceneManager::Instance().AddEnemy(obj);
	}

	if (ImGui::Button((const char*)u8"MT"))
	{
		auto obj = std::make_shared<MT>();
		obj->SetThis(obj);
		obj->SetThisBase(obj);
		obj->Init();
		obj->SetTag(KdGameObject::tEnemy);
		SceneManager::Instance().AddEnemy(obj);
	}
	
	if (ImGui::Button((const char*)u8"Enemy"))
	{
		auto obj = std::make_shared<Enemy>();
		obj->SetThis(obj);
		obj->SetThisBase(obj);
		obj->Init();
		obj->SetTag(KdGameObject::tEnemy);
		SceneManager::Instance().AddEnemy(obj);
	}
	
	if (ImGui::Button((const char*)u8"Balt"))
	{
		auto obj = std::make_shared<Balt>();
		obj->SetThis(obj);
		obj->SetThisBase(obj);
		obj->Init();
		obj->SetTag(KdGameObject::tEnemy);
		SceneManager::Instance().AddEnemy(obj);
	}
	
	if (ImGui::Button((const char*)u8"FLAC"))
	{
		auto obj = std::make_shared<FLAC>();
		obj->SetThis(obj);
		obj->SetThisBase(obj);
		obj->Init();
		obj->SetTag(KdGameObject::tEnemy);
		SceneManager::Instance().AddEnemy(obj);
	}

	if (ImGui::Button((const char*)u8"Scarecrow"))
	{
		auto obj = std::make_shared<Scarecrow>();
		obj->SetThis(obj);
		obj->SetThisBase(obj);
		obj->Init();
		obj->SetTag(KdGameObject::tEnemy);
		SceneManager::Instance().AddEnemy(obj);

	}

}

void EnemyCreater::EnemyInit()
{

}
