#include"CameraManager.h"

#include"FPSCamera/FPSCamera.h"
#include"NoneCamera/NoneCamera.h"
#include"RockCamera/RockCamera.h"
#include"TPSCamera/TPSCamera.h"
#include"TrackingCamera/TrackingCamera.h"

#include"../../Scene/SceneManager.h"

void CameraManager::Init() 
{
	SetNextType(m_nextType);
	KdEffekseerManager::GetInstance().Create(1280, 720);
}

void CameraManager::Update()
{
	if (m_currentCamera == nullptr) { return; }
	
	m_currentCamera->Update();
}

void CameraManager::PostUpdate()
{
	if (m_currentCamera == nullptr) { return; }

	m_currentCamera->PostUpdate();
}

void CameraManager::PreDraw()
{
	if (m_currentCamera == nullptr) { return; }

	m_currentCamera->PreDraw();
}


bool CameraManager::SetNextType(const CameraType& type)
{
	
	m_nextType = type;

	ChangeCamera(m_nextType);


	return true;
}

bool CameraManager::ChangeCamera(const CameraType& type)
{
	if (m_nextType == None) { return false; }

	if (m_nextType == m_nowType) { return false; }


	switch (m_nextType)
	{
	case CameraManager::None:
		m_currentCamera = std::make_shared<NoneCamera>();
		break;
	case CameraManager::TPS:
		m_currentCamera = std::make_shared<TPSCamera>();
		break;
	case CameraManager::FPS:
		m_currentCamera = std::make_shared<FPSCamera>();
		break;
	case CameraManager::Tracking:
		m_currentCamera = std::make_shared<TrackingCamera>();
		break;
	case CameraManager::Animation:
		break;
	case CameraManager::Rock:
		m_currentCamera = std::make_shared<RockCamera>();
		break;
	default:
		break;
	}

	m_nowType = m_nextType;
	m_currentCamera->SetTarget(m_wpCameraTarget);
	m_currentCamera->SetRockTarget(m_wpRockTarget);
	m_currentCamera->Init();
	
	return true;
}
