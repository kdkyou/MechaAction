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
		Rock,
		Hit,
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

	void DrawUnLit();
	void DrawSprite();

	void PreUpdate();
	void Update();
	void PostUpdate();


	//外部から変更する際に通る関数
	bool SetNextType(const CameraType& type);

	void SetCameraTarget(const std::shared_ptr<CharacterBase>& target) { m_wpCameraTarget = target; }
	void SetLockTarget(const std::shared_ptr<CharacterBase>& target) { m_wpLockTarget = target; }

	void SetMultiLocks(const std::shared_ptr<CharacterBase>& locks) { m_wpMultiLocks .push_back(locks); }
	void ResetMultiLocks() { m_wpMultiLocks.clear(); }
	void SetMultiLockNum(int num) { m_multiLockNum = num; }
	const int	 GetMultiLockNum()const { return m_multiLockNum; }
	const std::weak_ptr<CharacterBase>& GetLockTarget(UINT num);
	const std::vector<std::weak_ptr<CharacterBase>>& GetMultiLockList(){ return m_wpMultiLocks; }

	void EnableChangedCamera(bool isEnablechanged) { m_isEnableChanged = isEnablechanged; }

	bool IsEnableChanged() { return m_isEnableChanged; }

	void AnimationChange(std::shared_ptr<CameraBase> next);

	const std::weak_ptr<CameraBase> GetCurrentCamera() { return m_currentCamera; }

	const CameraType GetNowType()const { return m_nowType; }

	const Math::Vector3& ToCameraVec(const Math::Vector3 nowPos);

	// 自身のm_mWorldを使ってターゲットの相対方向(2D予定)を取得
	const Math::Vector2 GetLocalDirectionTo(const Math::Vector3& targetWorldPos)const;

	void Editor_ImGui();

private:

	//ChangeTypeで決めた型のカメラに変更 
	bool ChangeCamera(const CameraType& type);

	void Init();


	std::shared_ptr<CameraBase>	m_currentCamera = nullptr;
	std::weak_ptr<CharacterBase> m_wpCameraTarget;
	std::weak_ptr<CharacterBase> m_wpLockTarget;

	std::vector<std::weak_ptr<CharacterBase>>	m_wpMultiLocks;
	int											m_multiLockNum = 3;

	

	CameraType					m_nowType  = None;
	CameraType					m_nextType = None;

	bool						m_isEnableChanged = true;



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