#include "KdFPSController.h"

// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
// FPSの制御コントローラー
// ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### ##### #####
void KdFPSController::Initialize()
{
	m_startTime = std::chrono::system_clock::now();
}


void KdFPSController::Update()
{
	auto endTime = std::chrono::system_clock::now();
	auto durationTime = endTime - m_startTime;
	auto msec = std::chrono::duration_cast<std::chrono::microseconds>(durationTime).count();

	float prevTime = m_time;

	m_time = msec / 1000000.0f;

	m_deltaTime = m_time - prevTime;

	if (m_fixedFrameRate)
	{
		float targetFrameTime = 1.0f / m_targetFPS;
		float sleepTime = targetFrameTime - m_deltaTime;
		if (sleepTime > 0)
		{
			//		std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
			std::this_thread::sleep_for(std::chrono::milliseconds(16));
		}
	}

}
