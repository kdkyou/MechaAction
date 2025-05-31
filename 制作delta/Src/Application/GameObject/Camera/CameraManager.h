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

private:

	void Init();


	std::shared_ptr<CameraBase>	m_currentCamera = nullptr;

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