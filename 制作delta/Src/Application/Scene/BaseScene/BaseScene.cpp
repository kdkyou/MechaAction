#include "BaseScene.h"

#include "../../GameObject/Camera/CameraManager.h"
#include "../../GameObject/Character/CharacterBase.h"
#include "../../GameObject/UI/UIManager.h"

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

	// ↑の後には有効なオブジェクトだけのリストになっている

	for (auto& obj : m_objList)
	{
		obj->PreUpdate();
	}

	CameraManager::Instance().PreUpdate();

	UIManager::GetInstance().PreUpdate();
}

void BaseScene::Update()
{



	// KdGameObjectを継承した全てのオブジェクトの更新 (ポリモーフィズム)
	for (auto& obj : m_objList)
	{
		obj->Update();
	}

	KdEffekseerManager::GetInstance().Update();

	CameraManager::Instance().Update();

	UIManager::GetInstance().Update();


	// シーン毎のイベント処理
	Event();
}

void BaseScene::PostUpdate()
{
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

	CameraManager::Instance().PreDraw();
}

void BaseScene::Draw()
{
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 光を遮るオブジェクト(不透明な物体や2Dキャラ)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginGenerateDepthMapFromLight();
	{
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
		for (auto& obj : m_objList)
		{
			obj->DrawLit();
		}
	}
	KdShaderManager::Instance().m_StandardShader.EndLit();

	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 陰影のないオブジェクト(透明な部分を含む物体やエフェクト)はBeginとEndの間にまとめてDrawする
	KdShaderManager::Instance().m_StandardShader.BeginUnLit();
	{
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
		for (auto& obj : m_objList)
		{
			obj->DrawDebug();
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
					break;
				}
				AddObject(obj);

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
