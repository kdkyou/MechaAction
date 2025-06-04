#pragma once

class CameraBase;

class CameraManager
{
public :

	enum CameraType
	{
		None,
		TPS,
		Tracking,
		Animation,
		Rock,
	};

	bool ChangeType(const CameraType& type);
	bool ChangeCamera(std::shared_ptr<CameraBase> nextCamera);

	void SetCameraTarget(const std::shared_ptr<KdGameObject>& target) { m_wpCameraTarget = target; }
	void SetRockTarget(const std::shared_ptr<KdGameObject>& target) { m_wpRockTarget = target; }

	void AnimationChange(std::shared_ptr<CameraBase> next){}

private:

	void Init();


	std::shared_ptr<CameraBase>	m_currentCamera = nullptr;
	std::weak_ptr<KdGameObject> m_wpCameraTarget;
	std::weak_ptr<KdGameObject> m_wpRockTarget;

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