#include"CameraManager.h"
#include"CameraBase.h"
#include"TrackingCamera/TrackingCamera.h"
#include"RockCamera/RockCamera.h"
#include"TPSCamera/TPSCamera.h"

bool CameraManager::ChangeType(const CameraType& type)
{
	std::shared_ptr<CameraBase> camera = nullptr;

	switch (type)
	{
	case Tracking: camera = std::make_shared<TrackingCamera>(); 
		break;
	case Rock: camera = std::make_shared<RockCamera>();
		break;
	case TPS: camera = std::make_shared<TPSCamera>();
		break;
	default:
		break;
	}

	if (camera == nullptr) { return false; }

	ChangeCamera(camera);


	return true;
}

bool CameraManager::ChangeCamera(std::shared_ptr<CameraBase> nextCamera)
{
	if (nextCamera == nullptr) { return false; }

	m_currentCamera = nextCamera;
	m_currentCamera->Init();

	return true;
}
