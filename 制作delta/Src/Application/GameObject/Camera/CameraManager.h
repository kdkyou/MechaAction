#pragma once

class CameraBase;

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


	void PreDraw();
	void PreUpdate();
	void Update();
	void PostUpdate();


	//外部から変更する際に通る関数
	bool SetNextType(const CameraType& type);

	void SetCameraTarget(const std::shared_ptr<KdGameObject>& target) { m_wpCameraTarget = target; }
	void SetRockTarget(const std::shared_ptr<KdGameObject>& target) { m_wpRockTarget = target; }

	void EnableChangedCamera(bool isEnablechanged) { m_isEnableChanged = isEnablechanged; }

	bool IsEnableChanged() { return m_isEnableChanged; }

	void AnimationChange(std::shared_ptr<CameraBase> next) {}

	const std::weak_ptr<CameraBase> GetCurrentCamera() { return m_currentCamera; }

	const Math::Vector3& ToCameraVec(const Math::Vector3 nowPos);

private:

	//ChangeTypeで決めた型のカメラに変更 
	bool ChangeCamera(const CameraType& type);

	void Init();


	std::shared_ptr<CameraBase>	m_currentCamera = nullptr;
	std::weak_ptr<KdGameObject> m_wpCameraTarget;
	std::weak_ptr<KdGameObject> m_wpRockTarget;

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