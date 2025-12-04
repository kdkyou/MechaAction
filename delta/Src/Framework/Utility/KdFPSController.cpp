#include "KdFPSController.h"

// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// FPSの制御コントローラー
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
void KdFPSController::Initialize()
{
	m_fpsMonitorBeginTime = timeGetTime();
	m_startTime = std::chrono::system_clock::now();
}


void KdFPSController::Update()
{
	Control();

	Monitoring();

}

void KdFPSController::UpdateStartTime()
{
	m_frameStartTime = timeGetTime();
}

// FPS制御
void KdFPSController::Control()
{
	// 処理終了時間Get
	DWORD frameProcessEndTime = timeGetTime();

	auto endTime = std::chrono::system_clock::now();
	auto durationTime = endTime - m_startTime;
	auto msec = std::chrono::duration_cast<std::chrono::microseconds>(durationTime).count();

	float prevTime = m_time;
	m_time = msec / 1000000.0f;

	// 1フレームで経過すべき時間
	DWORD timePerFrame = kSecond / m_maxFps;

	//実際に1フレームで経過した時間
	m_deltaTime = m_time - prevTime;

	if (frameProcessEndTime - m_frameStartTime < timePerFrame)
	{
		// 1秒間にMaxFPS回数以上処理が回らないように待機する
		Sleep(timePerFrame - (frameProcessEndTime - m_frameStartTime));
	}
}

// 現在のFPS計測
void KdFPSController::Monitoring()
{
	// FPS計測のタイミング　0.5秒おき
	constexpr float kFpsRefreshFrame = 500;

	m_fpsCnt++;

	// 0.5秒おきに FPS計測
	if (m_frameStartTime - m_fpsMonitorBeginTime >= kFpsRefreshFrame)
	{
		// 現在のFPS算出
		m_nowfps = (m_fpsCnt * kSecond) / (m_frameStartTime - m_fpsMonitorBeginTime);

		m_fpsMonitorBeginTime = m_frameStartTime;

		m_fpsCnt = 0;
	}
}

void KdFPSController::Edit_ImGui()
{
	ImGui::DragInt("MaxFps", &m_maxFps, 1,30, 1200);
	ImGui::Text("NowFps:%d", m_nowfps);
	ImGui::Text("DeltaTime:%.5f", m_deltaTime);
	ImGui::Text("Time:%.5f", m_time);
}