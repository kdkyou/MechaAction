#include "CameraManager.h"

#include "FPSCamera/FPSCamera.h"
#include "NoneCamera/NoneCamera.h"
#include "LockCamera/LockCamera.h"
#include "TPSCamera/TPSCamera.h"
#include "TrackingCamera/TrackingCamera.h"
#include "HitCamera/HitCamera.h"
#include "LookAtCamera/LookAtCamera.h"
#include "AnimationCamera/AnimationCamera.h"
#include "MapCamera/MapCamera.h"

#include "../../Scene/SceneManager.h"

#include "../../main.h"

#include "../Character/CharacterBase.h"


void CameraManager::Init()
{
	SetNextType(m_nextType);
	KdEffekseerManager::GetInstance().Create(1280, 720);

	m_spTex = KdAssets::Instance().m_textures.GetData("Asset/Textures/UI/Target.png");

	m_mapCamera = std::make_shared<MapCamera>();
	m_mapCamera->Init();
}

void CameraManager::PreUpdate()
{
	ChangeCamera(m_nextType);
}

void CameraManager::Update()
{
	if (m_currentCamera == nullptr) { return; }

	Application::Instance().m_log.AddLog(m_currentCamera->GetName().c_str() + '\n');

	m_mapCamera->Update();
	
	m_currentCamera->Update();


	TargetUI();
}

void CameraManager::PostUpdate()
{
	if (m_currentCamera == nullptr) { return; }
	
	m_mapCamera->PostUpdate();

	m_currentCamera->PostUpdate();

}

void CameraManager::PreDraw()
{
	if (m_currentCamera == nullptr) { return; }

	m_currentCamera->PreDraw();

}

void CameraManager::MapPreDraw()
{
	if (m_mapCamera == nullptr) { return; }
	m_mapCamera->PreDraw();
}

void CameraManager::DrawUnLit()
{
	if (m_currentCamera == nullptr) { return; }

	m_currentCamera->DrawUnLit();

}

void CameraManager::MapDrawUnLit()
{
	if (m_mapCamera == nullptr) { return; }
	m_mapCamera->DrawUnLit();
}

void CameraManager::DrawSprite()
{
	if (m_currentCamera == nullptr) { return; }

	m_currentCamera->DrawSprite();
	
	if (m_wpMultiLocks.empty() == false) {

		if (m_wpMultiLocks.size() <= 0) { return; }

		for (auto& ui : m_uis)
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, (int)ui->pos.x, (int)ui->pos.y);
		}
	}

}

const Math::Vector2 CameraManager::GetLocalDirectionTo(const Math::Vector3& targetWorldPos) const
{
	if (m_currentCamera == nullptr) { return Math::Vector2(); }

	// カメラのビュー行列の逆
	auto& camMat = m_currentCamera->GetMatrix();
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
	if (ImGui::BeginMenu((const char*)u8"カメラ"))
	{
		nlohmann::json outJson;

		std::string str = "";

		auto sceneType = SceneManager::Instance().GetSceneType();
		switch (sceneType)
		{
		case SceneManager::SceneType::Title:
			str = "Asset/Data/TitleCamera.Scene";
			break;
		case SceneManager::SceneType::TitleMovie:
			str = "Asset/Data/TitleMovieCamera.Scene";
			break;
		case SceneManager::SceneType::Game:
			str = "Asset/Data/GameCamera.Scene";
			break;
		default:
			break;
		}

		if (ImGui::Button((const char*)u8"保存"))
		{
			nlohmann::json json;
			m_currentCamera->Serialize(json);
			outJson.push_back(json);

			std::ofstream ofs(str);
			if (ofs.is_open())
			{
				ofs << outJson.dump();
			}
		}

		if (ImGui::Button((const char*)u8"読み込み"))
		{
			Deserialize();
		}

		
		ImGui::EndMenu();
	}

	m_currentCamera->Editor_ImGui();
}

void CameraManager::Deserialize()
{
	
}

void CameraManager::Setting(const std::string& path)
{
	if (path == "") { return; }

	std::ifstream ifs(path);
	if (ifs.is_open())
	{
		nlohmann::json j;
		ifs >> j;
		for (auto& json : j)
		{
			std::string str;
			CameraType type = CameraType::None;
			int t = 0;
			KdJsonUtility::GetValue(json, "Name", &str);
			KdJsonUtility::GetValue(json, "Type", &t);

			switch (t)
			{
			case 0:
				type = CameraType::None;
				break;
			case 1:
				type = CameraType::FPS;
				break;
			case 2:
				type = CameraType::TPS;
				break;
			case 3:
				type = CameraType::Tracking;
				break;
			case 4:
				type = CameraType::Animation;
				break;
			case 5:
				type = CameraType::Lock;
				break;
			case 6:
				type = CameraType::Hit;
				break;
			case 7:
				type = CameraType::LookAt;
				break;
			case 8:
				break;
			default:
				break;
			}

			//if (!str.empty())
			{
				SetNextType(type);
				DeserializeChange(type,json);
			}
		}
	}
}

void CameraManager::TargetUI()
{
	m_uis.clear();

	if (m_wpMultiLocks.empty() == true) { return; }

	for(auto& obj : m_wpMultiLocks)
	{
		if (obj.expired() == true) { continue; }
		if (obj.lock()->IsDestroy() == true) { continue; }

		std::shared_ptr<TargetUIf> ui = std::make_shared<TargetUIf>();
		auto targetPos = obj.lock()->GetMatrix().Translation()+ obj.lock()->GetCorrectionMatrix().Translation();
		Math::Vector3 screenPos;
		auto& camera = m_currentCamera->GetCamera();
		camera->ConvertWorldToScreenDetail(targetPos, screenPos);
		ui->pos = { screenPos.x,screenPos.y };
		m_uis.push_back(ui);
	}
}

const float CameraManager::CalcLength(const Math::Vector3& pos, float Boundary)
{
   auto nowPos = m_currentCamera->GetMatrix().Translation();

   auto length = (pos - nowPos).Length();

   float clamp = std::clamp(length / Boundary, 0.3f, 1.0f);

	return  clamp;
}


bool CameraManager::SetNextType(const CameraType& type)
{

	if (IsEnableChanged() == false) { return false; }

	if (m_isEnableChanged == false) { return false; }

	m_nextType = type;

	return true;
}

void CameraManager::ResetAngle()
{
	Math::Vector3 deg = {};
	m_currentCamera->SetDeg(deg);
}

const std::weak_ptr<CharacterBase> CameraManager::GetLockTarget(int num)
{
	// TODO: return ステートメントをここに挿入します
	if (num >= m_multiLockNum || m_wpMultiLocks.empty() == true) {
		return {};
	}

	return m_wpMultiLocks[num];
}

void CameraManager::AnimationChange(std::shared_ptr<CameraBase> next)
{
	auto& camera = next;

}

const Math::Vector3& CameraManager::ToCameraVec(const Math::Vector3 nowPos)
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

	m_prevType = m_nowType;

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
		m_currentCamera = std::make_shared<AnimationCamera>();
		Application::Instance().m_log.AddLog("Animation\n");
		break;
	case CameraManager::Lock:
		m_currentCamera = std::make_shared<LockCamera>();
		Application::Instance().m_log.AddLog("Lock\n");
		break;
	case CameraManager::Hit:
		m_currentCamera = std::make_shared<HitCamera>();
		Application::Instance().m_log.AddLog("Hit\n");
		break;
	case CameraManager::LookAt:
		m_currentCamera = std::make_shared<LookAtCamera>();
		Application::Instance().m_log.AddLog("LookAt\n");
		break;
	default:
		break;
	}

	m_nowType = m_nextType;
	m_currentCamera->SetLook(m_wpLookTarget);
	m_currentCamera->SetTarget(m_wpCameraTarget);
	m_currentCamera->SetLockTarget(m_wpLockTarget);
	m_currentCamera->SetDeg(deg);
	m_currentCamera->Init();

	m_mapCamera->SetLook(m_wpLookTarget);
	m_mapCamera->SetTarget(m_wpCameraTarget);
	m_mapCamera->SetLockTarget(m_wpLockTarget);


	return true;
}

void CameraManager::DeserializeChange(const CameraType& type, const nlohmann::json& jsonObj)
{
	if(m_nowType == type){}

	switch (m_nextType)
	{
	case CameraManager::None:
	
	{
		auto current = std::make_shared<NoneCamera>();
		current->Deserialize(jsonObj);
		m_currentCamera = current;
	}
		Application::Instance().m_log.AddLog("None\n");
		break;
	case CameraManager::TPS:
	
	{
		auto current =std::make_shared<TPSCamera>();
			current->Deserialize(jsonObj);
		m_currentCamera = current;
	} 
		Application::Instance().m_log.AddLog("TPS\n");
		break;
	case CameraManager::FPS:
	{
	auto current = std::make_shared<FPSCamera>();
	current->Deserialize(jsonObj);
	m_currentCamera = current;
		}
		Application::Instance().m_log.AddLog("FPS\n");
		break;
	case CameraManager::Tracking:
	{
		auto current =	std::make_shared<TrackingCamera>();
		current->Deserialize(jsonObj);
		m_currentCamera = current;
	}
		Application::Instance().m_log.AddLog("Tracking\n");
		break;
	case CameraManager::Animation:
		{
		auto current = std::make_shared<AnimationCamera>();
		current->Deserialize(jsonObj);
		m_currentCamera = current;
		Application::Instance().m_log.AddLog("Animation\n");
		}
		break;
	case CameraManager::Lock:
		{
			auto current =std::make_shared<LockCamera>();
			current->Deserialize(jsonObj);
			m_currentCamera = current;
		}
		Application::Instance().m_log.AddLog("Lock\n");
		break;
	case CameraManager::Hit:
	{
		auto current =	std::make_shared<HitCamera>();
		current->Deserialize(jsonObj);
		m_currentCamera = current;
	}
		Application::Instance().m_log.AddLog("Hit\n");
		break;
	case CameraManager::LookAt:
		{
		auto current =	std::make_shared<LookAtCamera>();
		current->Deserialize(jsonObj);
		m_currentCamera = current;
		}
		Application::Instance().m_log.AddLog("LookAt\n");
		break;
	default:
		break;
	}

	m_nowType = m_nextType;
	m_currentCamera->SetLook(m_wpLookTarget);
	m_currentCamera->SetTarget(m_wpCameraTarget);
	m_currentCamera->SetLockTarget(m_wpLockTarget);
	m_currentCamera->Init();
}
