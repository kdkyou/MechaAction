#include "TitleScene.h"
#include "../SceneManager.h"

void TitleScene::Init()
{
}

void TitleScene::Event()
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Game
		);
	}
}