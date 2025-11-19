#pragma once

class BaseScene;

class CharacterBase;

class SceneManager
{
public :

	// シーン情報
	enum class SceneType
	{
		Title,
		TitleMovie,
		Game,
		Retry,
		Training,
		CheckStart,
	};

	void PreUpdate();
	void Update();
	void PostUpdate();

	void PreDraw();
	void Draw();
	void DrawSprite();
	void DrawDebug();

	void DrawMap();

	// 次のシーンをセット (次のフレームから切り替わる)
	void SetNextScene(SceneType nextScene)
	{
		m_nextSceneType = nextScene;
	}

	const SceneType GetSceneType()const { return m_currentSceneType; }

	// 現在のシーンのオブジェクトリストを取得
	const std::list<std::shared_ptr<KdGameObject>>& GetObjList();

	const std::list<std::shared_ptr<CharacterBase>>& GetPlayerList();

	const std::list<std::shared_ptr<CharacterBase>>& GetEnemyList();

	const std::list<std::shared_ptr<KdGameObject>>& GetTerrainList();

	// 現在のシーンにオブジェクトを追加
	void AddObject(const std::shared_ptr<KdGameObject>& obj);

	void AddEnemy(const std::shared_ptr<CharacterBase>& obj);
	
	void AddTerrain(const std::shared_ptr<KdGameObject>& obj);

	void Edit_ImGui();

	// レンダーターゲット切り替え用
	void ChangeRenderTarget();
	// 元に戻す関数
	void UndoRenderTarget();
	// 出来上がったテクスチャを取得
	const std::shared_ptr<KdTexture>& GetRenderTargetTexture()const
	{
		return m_rtPack.m_RTTexture;
	}
	
	void  SetIsMovie(const bool ismovie);
	const bool GetIsMovie() const;

private :

	// マネージャーの初期化
	// インスタンス生成(アプリ起動)時にコンストラクタで自動実行
	void Init()
	{
		// 開始シーンに切り替え
		ChangeScene(m_currentSceneType);

		// レンダーターゲット初期化
		m_rtPack.CreateRenderTarget(1280, 720, true);
	}

	// シーン切り替え関数
	void ChangeScene(SceneType sceneType);

	// 現在のシーンのインスタンスを保持しているポインタ
	std::shared_ptr<BaseScene> m_currentScene = nullptr;
	// 前のシーン
	std::shared_ptr<BaseScene> m_prevScene = nullptr;

	// 現在のシーンの種類を保持している変数
	SceneType m_currentSceneType = SceneType::Title;
	
	// 次のシーンの種類を保持している変数
	SceneType m_nextSceneType = m_currentSceneType;

	// レンダーターゲット切り替え用
	KdRenderTargetPack m_rtPack;

	KdRenderTargetChanger m_rtChanger;

private:

	SceneManager() { Init(); }
	~SceneManager() {}

public:

	// シングルトンパターン
	// 常に存在する && 必ず1つしか存在しない(1つしか存在出来ない)
	// どこからでもアクセスが可能で便利だが
	// 何でもかんでもシングルトンという思考はNG
	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}
};
