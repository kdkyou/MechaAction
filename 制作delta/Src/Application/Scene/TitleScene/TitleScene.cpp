#include "TitleScene.h"
#include "../SceneManager.h"

#include"../../GameObject/Camera/CameraManager.h"

#include "../../GameObject/Effect/Polygon/PolygonEffect.h"
#include "../../GameObject/UI/NumberUI/NumberUI.h"
#include "../../GameObject/UI/DrawUI/DrawUI.h"
#include "../../GameObject/Terrain/AnimTerrain.h"
#include "../../GameObject/Terrain/MoveTerrain.h"

#include "../../GameObject/UI/UIManager.h"

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
	CameraManager::Instance().Setting("Asset/Data/TitleCamera.scene");

	RenderSetting::GetInstance().RenderLoad("Asset/Data/Title.render");

	UIManager::GetInstance().SceneUICreate("Asset/Data/TitleUI.scene");

	m_once = false;

	KdAudioManager::Instance().Play("Asset/Sounds/BGM/Attack of the cyborg Legion.wav", true)->SetVolume(0.2f);
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
			um.SetFade(Fade::FadeIn, 0.5f, true);
			KdAudioManager::Instance().Play("Asset/Sounds/SE/Enter.wav", false)->SetVolume(0.25f);
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