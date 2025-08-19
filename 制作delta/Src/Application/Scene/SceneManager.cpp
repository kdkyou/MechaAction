#include "SceneManager.h"

#include "../GameObject/Character/CharacterBase.h"

#include "BaseScene/BaseScene.h"
#include "TitleScene/TitleScene.h"
#include "GameScene/GameScene.h"


void SceneManager::PreUpdate()
{
	// シーン切替
	if (m_currentSceneType != m_nextSceneType)
	{
		ChangeScene(m_nextSceneType);
	}

	m_currentScene->PreUpdate();
}

void SceneManager::Update()
{
	m_currentScene->Update();
}

void SceneManager::PostUpdate()
{
	m_currentScene->PostUpdate();
}

void SceneManager::PreDraw()
{
	m_currentScene->PreDraw();
}

void SceneManager::Draw()
{
	m_currentScene->Draw();
}

void SceneManager::DrawSprite()
{
	m_currentScene->DrawSprite();
}

void SceneManager::DrawDebug()
{
	m_currentScene->DrawDebug();
}

const std::list<std::shared_ptr<KdGameObject>>& SceneManager::GetObjList()
{
	return m_currentScene->GetObjList();
}

const std::list<std::shared_ptr<CharacterBase>>& SceneManager::GetPlayerList()
{
	return m_currentScene->GetPlayerList();
}

const std::list<std::shared_ptr<CharacterBase>>& SceneManager::GetEnemyList()
{
	return m_currentScene->GetEnemyList();
}

const std::list<std::shared_ptr<KdGameObject>>& SceneManager::GetTerrainList()
{
	return m_currentScene->GetTerrainList();
}

void SceneManager::AddObject(const std::shared_ptr<KdGameObject>& obj)
{
	m_currentScene->AddObject(obj);
}

void SceneManager::Edit_ImGui()
{

	static std::string fileName = "";

	ImGui::InputText((const char*)u8"ファイル名", &fileName);

	nlohmann::json outJson;
	if (ImGui::Button((const char*)u8"シーン保存"))
	{
		for (auto obj : m_currentScene->GetObjList())
		{
			nlohmann::json json;
			obj->Serialize(json);
			outJson.push_back(json);
		}

		std::string str = "";
		switch (m_currentSceneType)
		{
		case SceneManager::SceneType::Title:
			str = "Asset/Data/Title.Scene";
			break;
		case SceneManager::SceneType::Game:
			str = "Asset/Data/Game.Scene";
			break;
		default:
			break;
		}

		if (fileName != "")
		{
			str ="Asset/Data/" + fileName;
		}



		std::ofstream ofs(str);
		if (ofs.is_open())
		{
			ofs << outJson.dump();
		}
	}

	m_currentScene->Edit_ImGui();
}


void SceneManager::ChangeScene(SceneType sceneType)
{
	// 次のシーンを作成し、現在のシーンにする
	// 現在のシーン情報を更新
	m_currentSceneType = sceneType;
	
	std::shared_ptr<GameScene> scene;
	switch (sceneType)
	{
	case SceneType::Title:
		m_currentScene = std::make_shared<TitleScene>();
		break;
	case SceneType::Game:
		scene =  std::make_shared<GameScene>();
		m_currentScene = scene;
		break;
	}

	
}
