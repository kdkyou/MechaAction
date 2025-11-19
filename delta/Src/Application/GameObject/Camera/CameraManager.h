#pragma once

class CameraBase;
class CharacterBase;

class CameraManager
{
public:
	enum CameraType
	{
		None,
		FPS,
		TPS,
		Tracking,
		Animation,
		Lock,
		Hit,
		LookAt,
	};

	struct LockTargetInfo
	{
		std::weak_ptr<CharacterBase> wpLockTarget;
		float						distance = 0.0f;

		// ソート：距離が小さい順
		bool operator<(const std::shared_ptr<LockTargetInfo>& other)const{
			return distance < other->distance;
		}
	};


	void PreDraw();
	void MapPreDraw();

	void DrawUnLit();
	void MapDrawUnLit();
	void DrawSprite();

	void PreUpdate();
	void Update();
	void PostUpdate();


	//外部から変更する際に通る関数
	bool SetNextType(const CameraType& type);
	void ResetAngle();

	void SetCameraTarget(const std::shared_ptr<CharacterBase>& target) { m_wpCameraTarget = target; }
	void SetLockTarget(const std::shared_ptr<CharacterBase>& target) { m_wpLockTarget = target; }
	void SetLookTarget(const std::shared_ptr<KdGameObject>& target) { m_wpLookTarget = target; }

	void SetMultiLocks(const std::shared_ptr<CharacterBase>& locks) { m_wpMultiLocks .push_back(locks); }
	void ResetMultiLocks() { m_wpMultiLocks.clear(); }
	void SetMultiLockNum(int num) { m_multiLockNum = num; }
	const int	 GetMultiLockNum()const { return m_multiLockNum; }
	const std::weak_ptr<CharacterBase> GetLockTarget(int num);
	const std::vector<std::weak_ptr<CharacterBase>>& GetMultiLockList(){ return m_wpMultiLocks; }

	void EnableChangedCamera(bool isEnablechanged) { m_isEnableChanged = isEnablechanged; }

	const bool IsEnableChanged() const { return m_isEnableChanged; }

	void AnimationChange(std::shared_ptr<CameraBase> next);

	const std::weak_ptr<CameraBase> GetCurrentCamera() { return m_currentCamera; }
	const std::weak_ptr<CameraBase> GetMapCamera() { return m_mapCamera; }

	const CameraType GetPrevType()const { return m_prevType; }
	const CameraType GetNowType()const { return m_nowType; }

	const Math::Vector3& ToCameraVec(const Math::Vector3 nowPos);

	// 自身のm_mWorldを使ってターゲットの相対方向(2D予定)を取得 // y+正面　x+右
	const Math::Vector2 GetLocalDirectionTo(const Math::Vector3& targetWorldPos)const;

	void Editor_ImGui();

	void Deserialize();

	void Setting(const std::string& path);

	void TargetUI();

	// カメラ対オブジェクトの距離を図って近いほど数字が小さい
	const float CalcLength(const Math::Vector3& pos,float Boundary=100);

private:

	//ChangeTypeで決めた型のカメラに変更 
	bool ChangeCamera(const CameraType& type);
	void DeserializeChange(const CameraType& type, const nlohmann::json& jsonObj);

	void Init();

	std::shared_ptr<CameraBase>	m_currentCamera = nullptr;
	std::shared_ptr<CameraBase> m_mapCamera = nullptr;
	std::weak_ptr<KdGameObject>  m_wpLookTarget;
	std::weak_ptr<CharacterBase> m_wpCameraTarget;
	std::weak_ptr<CharacterBase> m_wpLockTarget;

	std::vector<std::weak_ptr<CharacterBase>>	m_wpMultiLocks;
	int											m_multiLockNum = 5;

	

	CameraType					m_prevType  = None;
	CameraType					m_nowType  = TPS;
	CameraType					m_nextType = None;

	bool						m_isEnableChanged = true;

	std::shared_ptr<KdTexture> m_spTex;
	struct TargetUIf
	{
		Math::Vector2 pos;
	};

	std::vector<std::shared_ptr<TargetUIf>> m_uis;
	//std::list<std::shared_ptr<TargetUIf>> m_targetUIList;

private:


	CameraManager() { Init(); }
	~CameraManager() {}

public:

	// シングルトンパターン
	// 常に存在する && 必ず1つしか存在しない(1つしか存在出来ない)
	// どこからでもアクセスが可能で便利だが
	// 何でもかんでもシングルトンという思考はNG
	static CameraManager& Instance()
	{
		static CameraManager instance;
		return instance;
	}

};