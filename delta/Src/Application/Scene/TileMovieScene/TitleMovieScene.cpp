#include "TitleMovieScene.h"

#include "../../GameObject/Camera/CameraManager.h"
#include "../SceneManager.h"

#include "../../GameObject/UI/UIManager.h"

void TitleMovieScene::Init()
{
	CurrentSceneCreate("Asset/Data/TitleMovie.scene");

	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherEnable(false);

	CameraManager::Instance().EnableChangedCamera(true);
	CameraManager::Instance().Setting("Asset/Data/TitleMovieCamera.scene");
	
	UIManager::GetInstance().ListClear();
	UIManager::GetInstance().SetFade(Fade::FadeOut, 0.2f, false);
	UIManager::GetInstance().SceneUICreate("Asset/Data/UI/TitleMovieUI.scene");

	RenderSetting::GetInstance().RenderLoad("Asset/Data/Render/TitleMovie.render");
	
	/*KdAudioManager::Instance().Play("Asset/Sounds/Voice/Mission.wav")
		->SetVolume(KdAudioManager::Instance().GetVoiceVolume());
	*/
}

void TitleMovieScene::Event()
{
	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherDist(0.01f);

	auto flg = UIManager::GetInstance().IsFadeComplete();
	auto& key = KeyInput::GetInstance().GetKeyboardState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();


	if (flg ) {
		if (!m_once) {
			m_once = true;
		}
	}

	if (m_once)
	{
		
		if (key.Space || pad.IsAPressed())
		{
			SceneManager::Instance().SetNextScene
			(
				SceneManager::SceneType::Game
			);
		}
	}
	if (key.T)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Title
		);
	}
}
