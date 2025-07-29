#include "TitleScene.h"
#include "../SceneManager.h"

#include"../../GameObject/Camera/CameraManager.h"

#include "../../GameObject/Effect/Polygon/PolygonEffect.h"

void TitleScene::Init()
{
	KdGameObjectFactory::Instance().RegisterGameObject<PolygonEffect>("PolygonEffect");

	CameraManager::Instance().SetNextType(CameraManager::CameraType:: None);
	CameraManager::Instance().EnableChangedCamera(false);

}

void TitleScene::Event()
{
	auto& key = KeyInput::GetInstance().GetKeyboardState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();
	auto& mouse = KeyInput::GetInstance().GetMouseState();

	if (key.Enter || pad.IsAPressed())
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Game
		);
	}
}