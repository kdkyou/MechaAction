#include "TitleMovieScene.h"

#include "../../GameObject/Camera/CameraManager.h"

void TitleMovieScene::Init()
{
	CurrentSceneCreate("Asset/Data/TitleMovie.scene");

	CameraManager::Instance().EnableChangedCamera(true);
	CameraManager::Instance().SetNextType(CameraManager::LookAt);
//	CameraManager::Instance().Setting();
	CameraManager::Instance().EnableChangedCamera(false);

}

void TitleMovieScene::Event()
{
}
