#include "TitleMovieScene.h"

#include "../../GameObject/Camera/CameraManager.h"

void TitleMovieScene::Init()
{
	CurrentSceneCreate("Asset/Data/TitleMovie.scene");

	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherEnable(false);

	CameraManager::Instance().EnableChangedCamera(true);
	CameraManager::Instance().SetNextType(CameraManager::LookAt);
	CameraManager::Instance().Setting("Asset/Data/TitleMovieCamera.scene");
	
	CameraManager::Instance().EnableChangedCamera(false);

}

void TitleMovieScene::Event()
{
}
