#include "TitleMovieScene.h"

#include "../../GameObject/Camera/CameraManager.h"
#include "../SceneManager.h"

#include "../../GameObject/UI/UIManager.h"

void TitleMovieScene::Init()
{
	CurrentSceneCreate("Asset/Data/TitleMovie.scene");

	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherDist(0.1f);

	CameraManager::Instance().EnableChangedCamera(true);
	CameraManager::Instance().Setting("Asset/Data/TitleMovieCamera.scene");
	
	UIManager::GetInstance().ListClear();
	UIManager::GetInstance().SetFade(Fade::FadeOut, 0.5f, false);

	RenderSetting::GetInstance().RenderLoad("Asset/Data/TitleMovie.render");
	
}

void TitleMovieScene::Event()
{

	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherEnable(false);

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

		if (duration <= 5.5f && duration>=5.4f) {
			if (!m_isFirstChange) {
				m_isFirstChange = true;
				CameraManager::Instance().Setting("Asset/Data/TitleMovieCamera2.scene");
			}
		}

		if (duration <= 4.0f && duration>=3.9f) {
			if (!m_isSecondChange) {
				m_isSecondChange = true;
				CameraManager::Instance().Setting("Asset/Data/TitleMovieCamera3.scene");
			}
		}

		/*if (duration < 0.0f || key.Enter)
		{
			SceneManager::Instance().SetNextScene
			(
				SceneManager::SceneType::Game
			);
		}*/
	}
}
