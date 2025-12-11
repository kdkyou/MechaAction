#include "BaseScene.h"

#include "../../GameObject/Camera/CameraManager.h"
#include "../../GameObject/Character/CharacterBase.h"
#include "../../GameObject/UI/UIManager.h"

#include "../../GameObject/Enemy/EnemyCreater.h"

void BaseScene::PreUpdate()
{
	// Updateの前の更新処理
	// オブジェクトリストの整理 ・・・ 無効なオブジェクトを削除
	auto it = m_objList.begin();

	while (it != m_objList.end())
	{
		if ((*it)->IsExpired())	// IsExpired() ・・・ 無効ならtrue
		{
			// 無効なオブジェクトをリストから削除
			it = m_objList.erase(it);
		}
		else
		{
			++it;	// 次の要素へイテレータを進める
		}
	}

	auto itr = m_enemyList.begin();

	while (itr != m_enemyList.end())
	{
		if ((*itr)->IsExpired())	// IsExpired() ・・・ 無効ならtrue
		{
			// 無効なオブジェクトをリストから削除
			itr = m_enemyList.erase(itr);
		}
		else
		{
			++itr;	// 次の要素へイテレータを進める
		}
	}

	// ↑の後には有効なオブジェクトだけのリストになっている

	for (auto& obj : m_playerList)
	{
		obj->PreUpdate();
	}

	for (auto& obj : m_enemyList)
	{
		obj->PreUpdate();
	}

	for (auto& obj : m_objList)
	{
		obj->PreUpdate();
	}

	CameraManager::Instance().PreUpdate();

	UIManager::GetInstance().PreUpdate();
}

void BaseScene::Update()
{
	
	// オブジェクトの更新 (ポリモーフィズム)
	if (EditorData::GetInstance().IsEditorMode() == true)
	{
		for (auto& obj : m_terrainList)
		{
			obj->Update();
		}
	}
	else
	{
		if (!m_isMovie) {
			for (auto& obj : m_playerList)
			{
				obj->Update();
			}

		for (auto& obj : m_enemyList)
		{
			obj->Update();
		}

		for (auto& obj : m_objList)
		{
			obj->Update();
		}
		}
	}

	KdEffekseerManager::GetInstance().Update();

	CameraManager::Instance().Update();

	UIManager::GetInstance().Update();


	if (EditorData::GetInstance().IsEditorMode() == false)
	{
		// シーン毎のイベント処理
		Event();
	}
}

void BaseScene::PostUpdate()
{
	for (auto& obj : m_playerList)
	{
		obj->PostUpdate();
	}

	for (auto& obj : m_enemyList)
	{
		obj->PostUpdate();
	}

	for (auto& obj : m_objList)
	{
		obj->PostUpdate();
	}

	CameraManager::Instance().PostUpdate();

}

void BaseScene::PreDraw()
{
	for (auto& obj : m_objList)
	{
		obj->PreDraw();
	}

	for (auto& obj : m_playerList)
	{
		obj->PreDraw();
	}
	for (auto& obj : m_enemyList)
	{
		obj->PreDraw();
	}

	CameraManager::Instance().PreDraw();
}

void BaseScene::PreDrawMap()
{
	CameraManager::Instance().PreDrawMap();
}

void BaseScene::Draw()
{
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 光を遮るオブジェクト(不透明な物体や2Dキャラ)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginGenerateDepthMapFromLight();
	{
		for (auto& obj : m_terrainList)
		{
			obj->GenerateDepthMapFromLight();
		}
		for (auto& obj : m_playerList)
		{
			obj->GenerateDepthMapFromLight();
		}

		for (auto& obj : m_enemyList)
		{
			obj->GenerateDepthMapFromLight();
		}
		
		for (auto& obj : m_objList)
		{
			obj->GenerateDepthMapFromLight();
		}

	}
	KdShaderManager::Instance().m_StandardShader.EndGenerateDepthMapFromLight();

	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 陰影のあるオブジェクト(不透明な物体や2Dキャラ)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginLit();
	{
		
		for (auto& obj : m_terrainList)
		{
			obj->DrawLit();
		}

		for (auto& obj : m_playerList)
		{
			obj->DrawLit();
		}

		for (auto& obj : m_enemyList)
		{
			obj->DrawLit();
		}

		for (auto& obj : m_objList)
		{
			obj->DrawLit();
		}
		
	}
	KdShaderManager::Instance().m_StandardShader.EndLit();

	KdShaderManager::Instance().m_particleShader.BeginParticle();
	{
		for (auto& obj : m_playerList)
		{
			obj->DrawParticle();
		}
		
		for (auto& obj : m_enemyList)
		{
			obj->DrawParticle();
		}
	}
	KdShaderManager::Instance().m_particleShader.EndParticle();

	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 陰影のないオブジェクト(透明な部分を含む物体やエフェクト)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
		for (auto& obj : m_terrainList)
		{
			obj->DrawUnLit();
		}

		for (auto& obj : m_playerList)
		{
			obj->DrawUnLit();
		}

		for (auto& obj : m_enemyList)
		{
			obj->DrawUnLit();
		}

		for (auto& obj : m_objList)
		{
			obj->DrawUnLit();
		}

		KdEffekseerManager::GetInstance().Draw();

		CameraManager::Instance().DrawUnLit();
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();


	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 光源オブジェクト(自ら光るオブジェクトやエフェクト)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_postProcessShader.BeginBright();
	{
		for (auto& obj : m_terrainList)
		{
			obj->DrawBright();
		}
		for (auto& obj : m_objList)
		{
			obj->DrawBright();
		}
	}
	KdShaderManager::Instance().m_postProcessShader.EndBright();

}

void BaseScene::DrawSprite()
{
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 2Dの描画はこの間で行う
	KdShaderManager::Instance().m_spriteShader.Begin();
	{
		for (auto& obj : m_objList)
		{
			obj->DrawSprite();
		}

		CameraManager::Instance().DrawSprite();

		UIManager::GetInstance().DrawSprite();
	}
	KdShaderManager::Instance().m_spriteShader.End();
}

void BaseScene::DrawDebug()
{
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// デバッグ情報の描画はこの間で行う
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
		if (EditorData::GetInstance().IsEditorMode() || EditorData::GetInstance().IsDebugMode())
		{

			for (auto& obj : m_objList)
			{
				obj->DrawDebug();
			}
			for (auto& obj : m_playerList)
			{
				obj->DrawDebug();
			}
			for (auto& obj : m_enemyList)
			{
				obj->DrawDebug();
			}
		}
		else {
			for (auto& obj : m_objList)
			{
				obj->DebugClear();
			}
			for (auto& obj : m_playerList)
			{
				obj->DebugClear();
			}
			for (auto& obj : m_enemyList)
			{
				obj->DebugClear();
			}

		}

	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();
}

void BaseScene::DrawMap()
{

	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
		CameraManager::Instance().MapDrawUnLit();
		
		KdShaderManager::Instance().m_StandardShader.SetGrayEnable(true);
		for (auto& obj : m_terrainList)
		{
			if (obj->GetTag() != KdGameObject::tPlayerAttack)
			{
				obj->DrawLit();
			}
		}
		KdShaderManager::Instance().m_StandardShader.SetGrayEnable(false);

		for (auto& obj : m_playerList)
		{
			obj->DrawMarker();
		}

		for (auto& obj : m_enemyList)
		{
			obj->DrawMarker();
		}

		
	}
	KdShaderManager::Instance().m_StandardShader.EndUnLit();
}

const std::list<std::shared_ptr<CharacterBase>>& BaseScene::GetPlayerList()
{
	// TODO: return ステートメントをここに挿入します
	return m_playerList;
}

const std::list<std::shared_ptr<CharacterBase>>& BaseScene::GetEnemyList()
{
	// TODO: return ステートメントをここに挿入します
	return m_enemyList;
}

const std::list<std::shared_ptr<KdGameObject>>& BaseScene::GetTerrainList()
{
	// TODO: return ステートメントをここに挿入します
	return m_terrainList;
}

void BaseScene::CurrentSceneCreate(const std::string& fileName)
{
	if (fileName == "") { return; }


	std::ifstream ifs(fileName);
	if (ifs.is_open())
	{
		ListClear();

		nlohmann::json j;
		ifs >> j;
		for (auto& json : j)
		{
			std::string str;

			KdJsonUtility::GetValue(json, "Name", &str);

			auto obj = KdGameObjectFactory::Instance().CreateGameObject(str);
			if (obj)
			{
				obj->Init();
				obj->Deserialize(json);
				auto tag = obj->GetTag();
				switch (tag)
				{
				case KdGameObject::tPlayer:
					//	AddPlayer(obj);
					break;
				case KdGameObject::tEnemy:
					//	AddEnemy(obj);
					break;
				case KdGameObject::tTerrain:
					AddTerrain(obj);
					break;
				case KdGameObject::tUI:
					break;
				case KdGameObject::tNone:
				case KdGameObject::tPlayerAttack:
				case KdGameObject::tEnemyAttack:
				default:
					AddObject(obj);
					break;
				}

				bool isTarget = false;
				KdJsonUtility::GetValue(json, "IsTarget", &isTarget);
				if (isTarget)
				{
					CameraManager::Instance().SetLookTarget(obj);
				}

			}
		}
	}
}

void BaseScene::TerrainCreate(const std::string& fileName)
{
	if (fileName == "") { return; }


	std::ifstream ifs(fileName);
	if (ifs.is_open())
	{
		m_terrainList.clear();

		nlohmann::json j;
		ifs >> j;
		for (auto& json : j)
		{
			std::string str;

			KdJsonUtility::GetValue(json, "Name", &str);

			auto obj = KdGameObjectFactory::Instance().CreateGameObject(str);
			if (obj)
			{
				obj->Init();
				obj->Deserialize(json);
				auto tag = obj->GetTag();
				switch (tag)
				{
				case KdGameObject::tPlayer:
					//	AddPlayer(obj);
					break;
				case KdGameObject::tEnemy:
					//	AddEnemy(obj);
					break;
				case KdGameObject::tTerrain:
					AddTerrain(obj);
					break;
				case KdGameObject::tUI:
					break;
				case KdGameObject::tNone:
				case KdGameObject::tPlayerAttack:
				case KdGameObject::tEnemyAttack:
				default:
					AddObject(obj);
					break;
				}

				bool isTarget = false;
				KdJsonUtility::GetValue(json, "IsTarget", &isTarget);
				if (isTarget)
				{
					CameraManager::Instance().SetLookTarget(obj);
				}

			}
		}
	}
}

void BaseScene::PositionReset()
{
	for (auto& obj : m_playerList)
	{
		obj->ResetPosition();
	}
}


void BaseScene::Edit_ImGui()
{
	if (ImGui::Button((const char*)u8"シーン読み込み"))
	{
		std::string filepath;
		if (EditorData::GetInstance().OpenFileDialog(filepath))
		{
			std::ifstream ifs(filepath);
			if (ifs.is_open())
			{
				ListClear();

				nlohmann::json j;
				ifs >> j;
				Deserialize(j);
			}
		}
	}

	static std::string str = "";
	if (ImGui::BeginCombo("SelectObject", str.empty() ? (const char*)u8"選択してください" : str.c_str()))
	{
		for (auto& obj : KdGameObjectFactory::Instance().GetRegisterObjectList())
		{
			if (ImGui::Selectable(obj.c_str(), obj == str))
			{
				str = obj;
			}
		}

		ImGui::EndCombo();
	}



	if (ImGui::Button((const char*)u8"オブジェクト追加"))
	{
		if (!str.empty())
		{
			auto obj = KdGameObjectFactory::Instance().CreateGameObject(str);
			if (obj)
			{
				obj->Init();
				AddObject(obj);
			}
		}
	}

	if (ImGui::Button((const char*)u8"地形追加"))
	{
		if (!str.empty())
		{
			auto obj = KdGameObjectFactory::Instance().CreateGameObject(str);
			if (obj)
			{
				obj->Init();
				AddTerrain(obj);
			}
		}
	}
	
	EnemyCreater::GetInstance().Editor_ImGui();

}



void BaseScene::Event()
{
	// 各シーンで必要な内容を実装(オーバーライド)する
}

void BaseScene::Init()
{
	// 各シーンで必要な内容を実装(オーバーライド)する
}

void BaseScene::ListClear()
{
	m_enemyList.clear();
	m_playerList.clear();
	m_terrainList.clear();
	m_objList.clear();
}
