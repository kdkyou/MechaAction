#include"CameraManager.h"

#include"FPSCamera/FPSCamera.h"
#include"NoneCamera/NoneCamera.h"
#include"RockCamera/RockCamera.h"
#include"TPSCamera/TPSCamera.h"
#include"TrackingCamera/TrackingCamera.h"
#include"HitCamera/HitCamera.h"

#include"../../Scene/SceneManager.h"

#include"../../main.h"

void CameraManager::Init() 
{
	SetNextType(m_nextType);
	KdEffekseerManager::GetInstance().Create(1280, 720);
}

void CameraManager::PreUpdate()
{
	ChangeCamera(m_nextType);
}

void CameraManager::Update()
{
	if (m_currentCamera == nullptr) { return; }

	Application::Instance().m_log.AddLog(m_currentCamera->GetName().c_str()+'\n');
	
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
	
	if (IsEnableChanged() == false) { return false; }

	m_nextType = type;
	
	return true;
}

const Math::Vector3& CameraManager::ToCameraVec( const Math::Vector3 nowPos)
{
	Math::Vector3 toVec = {};
	toVec = m_currentCamera->GetPos() - nowPos;

	toVec.Normalize();

	return toVec;
}

bool CameraManager::ChangeCamera(const CameraType& type)
{
	if (m_nextType == m_nowType) { return false; }

	Math::Vector3 deg = Math::Vector3::Zero;
	if (m_currentCamera != nullptr)
	{
		deg = m_currentCamera->GetDeg();
	}

	switch (m_nextType)
	{
	case CameraManager::None:
		m_currentCamera = std::make_shared<NoneCamera>();
		Application::Instance().m_log.AddLog("None\n");
		break;
	case CameraManager::TPS:
		m_currentCamera = std::make_shared<TPSCamera>();
		Application::Instance().m_log.AddLog("TPS\n");
		break;
	case CameraManager::FPS:
		m_currentCamera = std::make_shared<FPSCamera>();
		Application::Instance().m_log.AddLog("FPS\n");
		break;
	case CameraManager::Tracking:
		m_currentCamera = std::make_shared<TrackingCamera>();
		Application::Instance().m_log.AddLog("Tracking\n");
		break;
	case CameraManager::Animation:
		Application::Instance().m_log.AddLog("Animation\n");
		break;
	case CameraManager::Rock:
		m_currentCamera = std::make_shared<RockCamera>();
		Application::Instance().m_log.AddLog("Rock\n");
		break;
	case CameraManager::Hit:
		m_currentCamera = std::make_shared<HitCamera>();
		Application::Instance().m_log.AddLog("Hit\n");
		break;
	default:
		break;
	}

	m_nowType = m_nextType;
	m_currentCamera->SetTarget(m_wpCameraTarget);
	m_currentCamera->SetRockTarget(m_wpRockTarget);
	m_currentCamera->Init();
	m_currentCamera->SetDeg(deg);
	
	return true;
}
