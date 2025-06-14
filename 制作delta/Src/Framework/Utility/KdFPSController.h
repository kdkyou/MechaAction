#pragma once

//===========================================
// ゲーム時間
//===========================================
class KdFPSController
{
public:

	// FPS制御
	int		m_nowfps = 0;		// 現在のFPS値
	int		m_maxFps = 60;		// 最大FPS

	float GetFPS() { return m_nowfps; }
	float GetTime() { return m_time; }
	float GetDeltaTime() { return m_deltaTime; }

	void Initialize();

	void UpdateStartTime();

	void Update();

private:

	std::chrono::system_clock::time_point m_startTime;

	float m_time =0.0f;
	float m_deltaTime = 0.0f;

	void Control();

	void Monitoring();

	DWORD		m_frameStartTime = 0;		// フレームの開始時間

	int			m_fpsCnt = 0;				// FPS計測用カウント
	DWORD		m_fpsMonitorBeginTime = 0;	// FPS計測開始時間

	const int	kSecond = 1000;				// １秒のミリ秒


	//-------------------------------
	// シングルトン
	//-------------------------------
private:
	KdFPSController()
	{
	}
public:
	static KdFPSController& GetInstance() {
		static KdFPSController instance;
		return instance;
	}
};

//============================================================
// アプリケーションのFPS制御 + 測定
//============================================================
//struct KdFPSController
//{
//	// FPS制御
//	int		m_nowfps = 0;		// 現在のFPS値
//	int		m_maxFps = 60;		// 最大FPS
//
//	void Init();
//
//	void UpdateStartTime();
//
//	void Update();
//
//private:
//
//	void Control();
//
//	void Monitoring();
//
//	DWORD		m_frameStartTime = 0;		// フレームの開始時間
//
//	int			m_fpsCnt = 0;				// FPS計測用カウント
//	DWORD		m_fpsMonitorBeginTime = 0;	// FPS計測開始時間
//
//	const int	kSecond = 1000;				// １秒のミリ秒
//};
