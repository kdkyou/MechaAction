#include"CameraManager.h"

#include"FPSCamera/FPSCamera.h"
#include"NoneCamera/NoneCamera.h"
#include"LockCamera/LockCamera.h"
#include"TPSCamera/TPSCamera.h"
#include"TrackingCamera/TrackingCamera.h"
#include"HitCamera/HitCamera.h"

#include"../../Scene/SceneManager.h"

#include"../../main.h"

#include"../Character/CharacterBase.h"


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

void CameraManager::DrawUnLit()
{
	if (m_currentCamera == nullptr) { return; }

	m_currentCamera->DrawUnLit();

}

void CameraManager::DrawSprite()
{
	if (m_currentCamera == nullptr) { return; }

	m_currentCamera->DrawSprite();

}

const Math::Vector2 CameraManager::GetLocalDirectionTo(const Math::Vector3& targetWorldPos) const
{
	if (m_currentCamera == nullptr) { return Math::Vector2(); }
	
	// カメラのビュー行列の逆
	auto camMat = m_currentCamera->GetMatrix();
	auto invCam = camMat.Invert();

	Math::Vector3 pos = camMat.Translation();
	Math::Vector3 toTarget = targetWorldPos - pos;

	Math::Vector3 localDir = Math::Vector3::TransformNormal(toTarget, invCam);

	// z+正面　x+右
	Math::Vector2 result = { localDir.x,localDir.z };
	if (result.Length() > 0.0001f)
	{
		result.Normalize();
	}

	return result;

}

void CameraManager::Editor_ImGui() {

	

	if (m_currentCamera == nullptr) { return; }

	auto  deg =  m_currentCamera->GetDeg();
	ImGui::Text("deg.x%.2f,deg.y%.2f,deg.z%.2f", deg.x, deg.y,deg.z);
	
	m_currentCamera->Editor_ImGui();
}


bool CameraManager::SetNextType(const CameraType& type)
{
	
	if (IsEnableChanged() == false) { return false; }

	if (m_isEnableChanged == false) { return false; }

	m_nextType = type;
	
	return true;
}

const std::weak_ptr<CharacterBase>& CameraManager::GetLockTarget(UINT num)
{
	// TODO: return ステートメントをここに挿入します
	if (num >= m_multiLockNum || m_wpMultiLocks.empty() == true) { return std::weak_ptr<CharacterBase>(); }

	return m_wpMultiLocks[num];
}

void CameraManager::AnimationChange(std::shared_ptr<CameraBase> next)
{

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
		m_currentCamera = std::make_shared<LockCamera>();
		Application::Instance().m_log.AddLog("Lock\n");
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
	m_currentCamera->SetLockTarget(m_wpLockTarget);
	m_currentCamera->SetDeg(deg);
	m_currentCamera->Init();
	
	return true;
}
