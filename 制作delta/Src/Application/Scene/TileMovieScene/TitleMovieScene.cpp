#include "TitleMovieScene.h"

#include "../../GameObject/Camera/CameraManager.h"
#include "../SceneManager.h"

#include "../../GameObject/UI/UIManager.h"

void TitleMovieScene::Init()
{
	CurrentSceneCreate("Asset/Data/TitleMovie.scene");

	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherEnable(false);
	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherDist(0.5f);

	CameraManager::Instance().EnableChangedCamera(true);
	CameraManager::Instance().SetNextType(CameraManager::LookAt);
	CameraManager::Instance().Setting("Asset/Data/TitleMovieCamera.scene");
	
	CameraManager::Instance().EnableChangedCamera(false);

	UIManager::GetInstance().ListClear();
	UIManager::GetInstance().SetFade(Fade::FadeOut, 0.5f, false);

	RenderSetting::GetInstance().RenderLoad("Asset/Data/TitleMovie.render");
	

}

void TitleMovieScene::Event()
{
	auto flg = UIManager::GetInstance().IsFadeComplete();
	auto& key = KeyInput::GetInstance().GetKeyboardState();

	if (flg ) {
		if (!m_once) {
			m_once = true;
		}
	}

	if (m_once)
	{
		duration -= KdFPSController::GetInstance().GetDeltaTime();

		if (duration < 0.0f || key.Enter)
		{
			SceneManager::Instance().SetNextScene
			(
				SceneManager::SceneType::Game
			);
		}
	}
}
