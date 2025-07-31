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
	void Draw();
	void DrawSprite();
	void DrawDebug();


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

	void Edit_ImGui()
	{
		nlohmann::json outJson;
		if (ImGui::Button((const char*)u8"シーン保存"))
		{
			for (auto obj : m_objList)
			{
				nlohmann::json json;
				obj->Serialize(json);
				outJson.push_back(json);
			}

			std::ofstream ofs("Asset/Data/Game.scene");
			if (ofs.is_open())
			{
				ofs << outJson.dump();
			}
		}

		static std::string str = "";
		if (ImGui::BeginCombo("SelectObject", str.empty() ? (const char*)u8"選択してください" : str.c_str()))
		{
			for (auto obj : KdGameObjectFactory::Instance().GetRegisterObjectList())
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
					AddTerrain(obj);
				}
			}
		}

		for (auto obj : m_objList)
		{
			ImGui::PushID(obj.get());
			if (ImGui::CollapsingHeader(obj->GetName().c_str()))
			{
				obj->Editor_ImGui();
			}
			ImGui::PopID();

		}
	}


	
protected :

	// 継承先シーンで必要ならオーバーライドする
	virtual void Event();
	virtual void Init();
	
	// 全オブジェクトのアドレスをリストで管理
	std::list<std::shared_ptr<KdGameObject>> m_objList;
	std::list<std::shared_ptr<CharacterBase>> m_playerList;
	std::list<std::shared_ptr<CharacterBase>> m_enemyList;
	std::list<std::shared_ptr<KdGameObject>> m_terrainList;
};
