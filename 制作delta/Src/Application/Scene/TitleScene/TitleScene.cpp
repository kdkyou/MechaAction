#include "TitleScene.h"
#include "../SceneManager.h"

#include"../../GameObject/Camera/CameraManager.h"

#include "../../GameObject/Effect/Polygon/PolygonEffect.h"
#include "../../GameObject/UI/NumberUI/NumberUI.h"
#include "../../GameObject/UI/DrawUI/DrawUI.h"
#include "../../GameObject/Terrain/AnimTerrain.h"
#include "../../GameObject/Terrain/MoveTerrain.h"
#include "../../GameObject/Terrain/EmmisiveTerrain.h"
#include "../../GameObject/Character/TitleMovie.h"
#include "../../GameObject/Terrain/Terrain.h"

#include "../../GameObject/UI/UIManager.h"

void TitleScene::Init()
{
	KdEffekseerManager::GetInstance().StopAllEffect();

	CameraManager::Instance().ResetMultiLocks();
	UIManager::GetInstance().SetFade(Fade::FadeOut, 0.2f, false);
	KdAudioManager::Instance().StopAllSound();

	KdGameObjectFactory::Instance().RegisterGameObject<PolygonEffect>("PolygonEffect");
	KdGameObjectFactory::Instance().RegisterGameObject<NumberUI>("NumberUI");
	KdGameObjectFactory::Instance().RegisterGameObject<DrawUI>("DrawUI");
	KdGameObjectFactory::Instance().RegisterGameObject<AnimTerrain>("AnimTerrain");
	KdGameObjectFactory::Instance().RegisterGameObject<MoveTerrain>("MoveTerrain");
	KdGameObjectFactory::Instance().RegisterGameObject<EmmisiveTerrain>("EmmisiveTerrain");
	KdGameObjectFactory::Instance().RegisterGameObject<TitleMovie>("TitleMovie");
	KdGameObjectFactory::Instance().RegisterGameObject<Terrain>("Terrain");


	CurrentSceneCreate("Asset/Data/Title.scene");

	CameraManager::Instance().SetNextType(CameraManager::CameraType:: None);
	CameraManager::Instance().EnableChangedCamera(false);
	CameraManager::Instance().Setting("Asset/Data/TitleCamera.scene");
	CameraManager::Instance().EnableChangedCamera(true);

	RenderSetting::GetInstance().RenderLoad("Asset/Data/Title.render");

	UIManager::GetInstance().SceneUICreate("Asset/Data/TitleUI.scene");

	m_once = false;

	KdAudioManager::Instance().Play("Asset/Sounds/BGM/ApocalypseFull.wav", true)->SetVolume(0.15f);
}

void TitleScene::Event()
{
	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherEnable(RenderSetting::GetInstance().IsAlphaDither());


	auto& key = KeyInput::GetInstance().GetKeyboardState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();
	auto& mouse = KeyInput::GetInstance().GetMouseState();

	auto& um = UIManager::GetInstance();

	if (key.Enter || pad.IsAPressed())
	{
		if (!m_once)
		{
			m_once = true;
			um.SetFade(Fade::FadeIn, 0.2f, true);
			KdAudioManager::Instance().Play("Asset/Sounds/SE/Enter.wav", false)->SetVolume(0.25f);
			//KdAudioManager::Instance().Play("Asset/Sounds/SE/LongWalk.wav", false)->SetVolume(0.25f);
		}
	}
	
	if (m_once)
	{
		if (um.IsFadeComplete())
		{
			SceneManager::Instance().SetNextScene(
				SceneManager::SceneType::TitleMovie
			);
		}
	}
}