#pragma once

class CharacterBase;

class BaseScene
{
public :

	BaseScene() { Init(); }
	virtual ~BaseScene() {}

	void PreUpdate();
	void Update();
	void PostUpdate();

	void PreDraw();
	void PreDrawMap();
	void Draw();
	void DrawSprite();
	void DrawDebug();

	void DrawMap();


	// オブジェクトリストを取得
	const std::list<std::shared_ptr<KdGameObject>>& GetObjList()
	{
		return m_objList;
	}

	const std::list<std::shared_ptr<CharacterBase>>& GetPlayerList();
	
	const std::list<std::shared_ptr<CharacterBase>>& GetEnemyList();
	

	const std::list<std::shared_ptr<KdGameObject>>& GetTerrainList();


	// オブジェクトリストに追加
	void AddObject(const std::shared_ptr<KdGameObject>& obj)
	{
		m_objList.push_back(obj);
	}
	
	// プレイヤーリストに追加
	void AddPlayer(const std::shared_ptr<CharacterBase>& obj)
	{
		m_playerList.push_back(obj);
	}

	// オブジェクトリストに追加
	void AddEnemy(const std::shared_ptr<CharacterBase>& obj)
	{
		m_enemyList.push_back(obj);
	}

	// オブジェクトリストに追加
	void AddTerrain(const std::shared_ptr<KdGameObject>& obj)
	{
		m_terrainList.push_back(obj);
	}

	virtual void Deserialize(const nlohmann::json& jsonObj)
	{
		for (auto json : jsonObj)
		{
			std::string str;
			KdJsonUtility::GetValue(json, "Name", &str);
			if (!str.empty())
			{
				auto obj = KdGameObjectFactory::Instance().CreateGameObject(str);
				if (obj)
				{
					obj->Init();
					obj->Deserialize(json);
					AddObject(obj);
				}
			}
		}
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const
	{
		for (auto obj : m_objList)
		{
			obj->Serialize(outJson);
		}
	}

	void Edit_ImGui();

	const int GetNum()const { return m_num; }
	
protected :

	void CurrentSceneCreate(const std::string& fileName);
	void EnemyCreate(const std::string& fileName);
	
	// 継承先シーンで必要ならオーバーライドする
	virtual void Event();
	virtual void Init();
	
	void ListClear();

	// 全オブジェクトのアドレスをリストで管理
	std::list<std::shared_ptr<KdGameObject>> m_objList;
	std::list<std::shared_ptr<CharacterBase>> m_playerList;
	std::list<std::shared_ptr<CharacterBase>> m_enemyList;
	std::list<std::shared_ptr<KdGameObject>> m_terrainList;

	bool m_once;

	int m_num = 0;
};
