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

	void Edit_ImGui();

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
