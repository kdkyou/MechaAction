#include "TitleMovieScene.h"

#include "../../GameObject/Camera/CameraManager.h"
#include "../SceneManager.h"

#include "../../GameObject/UI/UIManager.h"

void TitleMovieScene::Init()
{
	CurrentSceneCreate("Asset/Data/TitleMovie.scene");

	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherEnable(false);

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
	if (flg)
	{
		duration -= KdFPSController::GetInstance().GetDeltaTime();

		if (duration < 0.0f)
		{
			SceneManager::Instance().SetNextScene
			(
				SceneManager::SceneType::Game
			);
		}
	}
}
