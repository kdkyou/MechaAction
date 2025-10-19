#include "EnemyCreater.h"

#include "Enemy.h"
#include "Drone/Drone.h"
#include "MT/MT.h"
#include "FLAC/FLAC.h"
#include "Balt/Balt.h"

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
					auto obj = std::make_shared<Enemy>();
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
			}
		}
	}
}

void EnemyCreater::Editor_ImGui()
{
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


}
