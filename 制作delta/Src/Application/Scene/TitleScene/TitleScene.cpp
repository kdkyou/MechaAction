#include "TitleScene.h"
#include "../SceneManager.h"

#include"../../GameObject/Camera/CameraManager.h"

void TitleScene::Init()
{
	CameraManager::Instance().SetNextType(CameraManager::CameraType::TPS);

}

void TitleScene::Event()
{
	auto& key = KeyInput::GetInstance().GetKeyboardState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();
	auto& mouse = KeyInput::GetInstance().GetMouseState();

	if (key.Enter || pad.IsAPressed() ||mouse.leftButton)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Game
		);
	}
}