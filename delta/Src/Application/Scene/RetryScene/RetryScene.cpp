#include "RetryScene.h"

#include "../../GameObject/UI/UIManager.h"

#include "../SceneManager.h"
#include "../../GameObject/Camera/CameraManager.h"
void RetryScene::Init()
{
	KdAudioManager::Instance().StopAllSound();
	CameraManager::Instance().SetNextType(CameraManager::CameraType::None);
	UIManager::GetInstance().SceneUICreate();
}

void RetryScene::Event()
{

}
