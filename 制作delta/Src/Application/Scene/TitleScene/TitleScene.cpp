#include "TitleScene.h"
#include "../SceneManager.h"

#include"../../GameObject/Camera/CameraManager.h"

#include "../../GameObject/Effect/Polygon/PolygonEffect.h"
#include "../../GameObject/UI/NumberUI/NumberUI.h"
#include "../../GameObject/UI/DrawUI/DrawUI.h"
#include "../../GameObject/Terrain/AnimTerrain.h"
#include "../../GameObject/Terrain/MoveTerrain.h"

void TitleScene::Init()
{
	KdGameObjectFactory::Instance().RegisterGameObject<PolygonEffect>("PolygonEffect");
	KdGameObjectFactory::Instance().RegisterGameObject<NumberUI>("NumberUI");
	KdGameObjectFactory::Instance().RegisterGameObject<DrawUI>("DrawUI");
	KdGameObjectFactory::Instance().RegisterGameObject<AnimTerrain>("AnimTerrain");
	KdGameObjectFactory::Instance().RegisterGameObject<MoveTerrain>("MoveTerrain");

	CurrentSceneCreate("Asset/Data/Title.scene");

	CameraManager::Instance().SetNextType(CameraManager::CameraType:: None);
	CameraManager::Instance().EnableChangedCamera(false);
	//CameraManager::Instance().Deserialize();

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