#include "RetryScene.h"

#include "../../GameObject/UI/UIManager.h"

#include "../SceneManager.h"

void RetryScene::Init()
{
//	RenderSetting::GetInstance().RenderLoad("Asset/Data/Render/Retry.render");
	UIManager::GetInstance().SceneUICreate();
}

void RetryScene::Event()
{
	auto& key = KeyInput::GetInstance().GetKeyboardState();

	if (key.T) {
		SceneManager::Instance().SetNextScene(SceneManager::SceneType::Title);
	}
}
