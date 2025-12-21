#include "GameScene.h"
#include"../SceneManager.h"


#include "../../GameObject/Character/Character.h"
#include "../../GameObject/Enemy/Enemy.h"
#include "../../GameObject/Enemy/Drone/Drone.h"
#include "../../GameObject/Enemy/MT/MT.h"

#include "../../GameObject/Weapon/Blade/Blade.h"
#include "../../GameObject/Weapon/Shield/Shield.h"
#include "../../GameObject/Weapon/Sowrd/Sowrd.h"
#include "../../GameObject/Weapon/Gun/Rifle/Rifle.h"
#include "../../GameObject/Weapon/Gun/Missile/Missile.h"
#include "../../GameObject/Weapon/Gun/Charge/Charge.h"

#include "../../GameObject/Camera/CameraManager.h"

#include "../../GameObject/UI/UIManager.h"

#include "../../GameObject/Enemy/EnemyCreater.h"

// 少数第n位で四捨五入する
void round_n(float& number, int n)
{
	number = number * static_cast<float>(pow(10, n - 1));
	number = round(number);
	number /= static_cast<float>(pow(10, n - 1));
}

void GameScene::Init()
{
	
	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherEnable(true);
	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherDist(6.0f);

	KdEffekseerManager::GetInstance().StopAllEffect();

	//===================================================================
	// ステージ初期化
	//===================================================================
	
	CurrentSceneCreate("Asset/Data/Scene/Game.scene");


	//===================================================================
	// キャラクター初期化
	//===================================================================
	std::shared_ptr<Character> _character = std::make_shared<Character>();
	_character->SetThis(_character);
	_character->SetThisBase(_character);
	_character->Init();
	_character->SetPos({ 0.0f,5.0f,-650.0f });
	AddPlayer(_character);


	// プレイヤー武器
	{
	

		std::shared_ptr<Sowrd> _sowrd = std::make_shared<Sowrd>();
		_sowrd->Init();
		_sowrd->SetModel("Asset/Models/Weapon/Sowrd/Sowrd.gltf");
		_sowrd->SetParent(_character);
		_sowrd->SetAttachPath("RightHand");
		_sowrd->SetTag(KdGameObject::tPlayerAttack);
		_sowrd->SetDamage(1000);
		AddObject(_sowrd);


		std::shared_ptr<Shield> _shield = std::make_shared<Shield>();
		_shield->Init();
		_shield->SetModel("Asset/Models/Weapon/Shield/Shield.gltf");
		_shield->SetParent(_character);
		_shield->SetAttachPath("LeftWeapon");
		_shield->SetTag(KdGameObject::tPlayerAttack);
		AddObject(_shield);
		AddTerrain(_shield);
		m_wpShield = _shield;

		std::shared_ptr<Charge> charge;

		charge = std::make_shared<Charge>();
		charge->SetParent(_character);
		charge->SetAttackTrigger(WeaponBase::LeftShoulder);
		charge->SetAttachPath("LeftShoulderWeapon");
		charge->SetGunsParam("Asset/Models/Weapon/RaserCannon/RaserCannon.gltf", 2.5f, 6.0f,0.0f,1, 12, 24);
		charge->MakeAnimator("Close", 20.0f);
		charge->Init();
		charge->SetTag(KdGameObject::tPlayerAttack);
		charge->SetBulletsParam("Asset/Models/Weapon/Bullet/RaserBullet.gltf", 5.0f, 300, 530.0f, 200, 40.0f, 0.95f);
		charge->SetBulletsTrailParam("Asset/Textures/GameObject/Prazma2.png",Math::Color(0.47f, 0.4f, 0.88f), 20.0f, 30);
		charge->SetNodeMats("SP");
		AddObject(charge);;
		
		std::shared_ptr<Missile> missile;
		missile = std::make_shared<Missile>();
		missile->SetParent(_character);
		missile->SetAttackTrigger(WeaponBase::RightShoulder);
		missile->SetAttachPath("RightShoulderWeapon");
		missile->SetGunsParam("Asset/Models/Weapon/Missile/FrontMissile/3LineMissile.gltf", 1.8f, 7.0f,0.025f,3, 12, 24);
		missile->MakeAnimator("Close", 20.0f);
		missile->SetNodeMats("SP1");
		missile->SetNodeMats("SP2");
		missile->SetNodeMats("SP3");
	
		missile->Init();
		missile->SetTag(KdGameObject::tPlayerAttack);
		
		missile->SetBulletsParam("Asset/Models/Weapon/Bullet/MissileBullet.gltf",20.0f,200, 200.0f, 200, 40.0f, 0.95f);
		missile->SetBulletsTrailParam("Asset/Textures/GameObject/Smoke.png", Math::Color(0.36f, 0.3f, 0.3f), 3.5f, 30);
		missile->SetBulletChaisingData(9, 70.0f, 0.8f, 2000.0f);
		AddObject(missile);

	}

	UIManager::GetInstance().SetFade(Fade::FadeOut, 0.01f, false);
	UIManager::GetInstance().SceneUICreate();

	//===================================================================
	// カメラ初期化
	//===================================================================
	CameraManager::Instance().EnableChangedCamera(true);
	CameraManager::Instance().SetCameraTarget(_character);
	CameraManager::Instance().SetNextType(CameraManager::CameraType::Tracking);
	CameraManager::Instance().ResetAngle();

	m_duration = 1.0f;
	m_durationFade = 0.0f;
	m_fade = false;
	m_once = false;

	auto& am = KdAudioManager::Instance();
	am.StopAllSound();
	am.Play("Asset/Sounds/BGM/Rusty.wav", true)->SetVolume(am.GetBGMVolume());

	RenderSetting::GetInstance().RenderLoad("Asset/Data/Render/Game.render");
}

void GameScene::SetNum(int num)
{
	m_waveProgress = (Wave)num;
	m_num = num;
}

void GameScene::Event()
{
	if (m_enemyList.empty() == true)
	{
		switch (m_waveProgress)
		{
		case Wave::Start:
			m_waveProgress = First;
			EnemyCreater::GetInstance().EnemysCreate("Asset/Data/Enemy/First.enemy");
			m_num = Start;
			break;
		case Wave::First:
			m_waveProgress = Second;
			TerrainCreate("Asset/Data/Scene/Game2.scene");
			PositionReset();
			AddTerrain(m_wpShield.lock());
			EnemyCreater::GetInstance().EnemysCreate("Asset/Data/Enemy/Second.enemy");
			KdAudioManager::Instance().Play("Asset/Sounds/Voice/Second.wav", false)->SetVolume(KdAudioManager::Instance().GetVoiceVolume());
			m_isMovie = true;
			m_num = First;
			break;
		case Wave::Second:
			m_waveProgress = Last;
			TerrainCreate("Asset/Data/Scene/Game3.scene");
			PositionReset();
			AddTerrain(m_wpShield.lock());
			EnemyCreater::GetInstance().EnemysCreate("Asset/Data/Enemy/Last.enemy");
			KdAudioManager::Instance().Play("Asset/Sounds/Voice/Last.wav", false)->SetVolume(KdAudioManager::Instance().GetVoiceVolume());
			m_isMovie = true;
			m_num = Second;
			break;
		case Wave::Last:
			m_waveProgress = Complete;
			m_num = Last;
			break;
		case Wave::Complete:
			m_num = Complete;
			break;
		default:
			break;
		}
	}

	if (m_waveProgress == Complete) {
		if (!m_once)
		{
			m_once = true;
			UIManager::GetInstance().SceneUICreate("Asset/Data/UI/CompleteUI.scene");
		}
	}

	if (m_isMovie) {
		auto sin = std::sinf(m_durationFade * DirectX::XM_2PI);

		KdShaderManager::Instance().m_postProcessShader.SetRadialBlurInfo(8, sin, { 0.5f,0.5f }, 0.05f, 0, 0.0f);
		UINT kind = KdShaderManager::Instance().m_postProcessShader.RadialBlur;
		KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
		
		m_durationFade += KdFPSController::GetInstance().GetDeltaTime();
		if (m_durationFade > 0.5f)
		{
			m_durationFade = 0.0f;
			m_isMovie = false;
			kind = KdShaderManager::Instance().m_postProcessShader.Normal;
			KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
		}
	}


	if (m_once)
	{

		m_duration -= KdFPSController::GetInstance().GetDeltaTime();

		if (m_duration < 0.0f)
		{
			if (!m_fade)
			{
				m_fade = true;
				UIManager::GetInstance().SetFade(Fade::FadeIn, 1.0f, true);
			}
		}
	}

	if (m_fade)
	{
		if (UIManager::GetInstance().IsFadeComplete())
		{
			SceneManager::Instance().SetNextScene(SceneManager::SceneType::Title);
			CameraManager::Instance().ResetMultiLocks();
		}
	}
}