#pragma once

//===========================================
// 
// ゲーム時間
// 
//===========================================
class KdFPSController
{
public:

	float GetTime() { return m_time; }
	float GetDeltaTime() { return m_deltaTime; }

	void Initialize();

	void Update();


private:

	std::chrono::system_clock::time_point m_startTime;

	float m_time = 0.0f;
	float m_deltaTime = 0.0f;

	bool m_fixedFrameRate = true;
	float m_targetFPS = 60.0f;


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