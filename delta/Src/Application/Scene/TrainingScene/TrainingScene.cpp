#include "TrainingScene.h"
#include"../SceneManager.h"


#include "../../GameObject/Character/Character.h"

#include "../../GameObject/Weapon/Shield/Shield.h"
#include "../../GameObject/Weapon/Sowrd/Sowrd.h"
#include "../../GameObject/Weapon/Gun/Missile/Missile.h"
#include "../../GameObject/Weapon/Gun/Charge/Charge.h"

#include "../../GameObject/Camera/CameraManager.h"

#include "../../GameObject/UI/UIManager.h"

#include "../../GameObject/Enemy/EnemyCreater.h"

void TrainingScene::Init()
{

	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherEnable(true);
	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherDist(6.0f);

	//===================================================================
	// ステージ初期化
	//===================================================================

	CurrentSceneCreate("Asset/Data/Training.scene");

	//===================================================================
	// キャラクター初期化
	//===================================================================
	std::shared_ptr<Character> _character = std::make_shared<Character>();
	_character->SetThis(_character);
	_character->SetThisBase(_character);
	_character->Init();
	_character->SetPos({ 0.0f,1.0f,0.0f });
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


		std::shared_ptr<Charge> charge;

		charge = std::make_shared<Charge>();
		charge->SetParent(_character);
		charge->SetAttackTrigger(WeaponBase::LeftShoulder);
		charge->SetAttachPath("LeftShoulderWeapon");
		charge->SetGunsParam("Asset/Models/Weapon/RaserCannon/RaserCannon.gltf", 2.5f, 6.0f, 0.0f, 1, 12, 24);
		charge->MakeAnimator("Close", 20.0f);
		charge->Init();
		charge->SetTag(KdGameObject::tPlayerAttack);
		charge->SetBulletsParam("Asset/Models/Weapon/Bullet/RaserBullet.gltf", 5.0f, 300, 530.0f, 200, 40.0f, 0.95f);
		charge->SetBulletsTrailParam("Asset/Textures/GameObject/Smoke2.png", Math::Color(0.47f, 0.4f, 0.88f), 20.0f, 30);
		AddObject(charge);;

		std::shared_ptr<Missile> missile;
		missile = std::make_shared<Missile>();
		missile->SetParent(_character);
		missile->SetAttackTrigger(WeaponBase::RightShoulder);
		missile->SetAttachPath("RightShoulderWeapon");
		missile->SetGunsParam("Asset/Models/Weapon/Missile/FrontMissile/3LineMissile.gltf", 1.8f, 7.0f, 0.025f, 3, 12, 24);
		missile->MakeAnimator("Close", 20.0f);
		missile->SetNodeMats("SP1");
		missile->SetNodeMats("SP2");
		missile->SetNodeMats("SP3");
		missile->Init();
		missile->SetTag(KdGameObject::tPlayerAttack);

		missile->SetBulletsParam("Asset/Models/Weapon/Bullet/MissileBullet.gltf", 20.0f, 200, 200.0f, 200, 40.0f, 0.95f);
		missile->SetBulletsTrailParam("Asset/Textures/GameObject/Smoke.png", Math::Color(0.36f, 0.3f, 0.3f), 3.5f, 30);
		missile->SetBulletChaisingData(5, 70.0f, 0.8f, 300.0f);
		AddObject(missile);

	} 



	UIManager::GetInstance().ListClear();
	UIManager::GetInstance().SceneUICreate();
	UIManager::GetInstance().SetFade(Fade::FadeOut, 0.2f, false);


	//===================================================================
	// カメラ初期化
	//===================================================================
	CameraManager::Instance().EnableChangedCamera(true);
	CameraManager::Instance().SetCameraTarget(_character);
	CameraManager::Instance().SetNextType(CameraManager::CameraType::Tracking);

	RenderSetting::GetInstance().RenderLoad("Asset/Data/Render/Training.render");

	m_duration = 1.0f;
	m_once = false;
	
}

void TrainingScene::Event()
{
	if (!m_fade) {
		m_fade = true;
		EnemyCreater::GetInstance().EnemysCreate("Asset/Data/Enemy/Training.enemy");
	}

	auto flg = UIManager::GetInstance().IsFadeComplete();

	auto& key = KeyInput::GetInstance().GetKeyboardState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	auto& um = UIManager::GetInstance();

	m_duration -= KdFPSController::GetInstance().GetDeltaTime();
	if (m_duration <= 0.0f)
	{
		m_duration = 0.0f;
		if (flg) {
			if (key.Enter || pad.IsAPressed())
			{
				if (!m_once)
				{
					m_once = true;
					um.SetFade(Fade::FadeIn, 0.14f, true);
					auto& am = KdAudioManager::Instance();
					am.Play("Asset/Sounds/SE/Enter.wav", false)->SetVolume(am.GetSEVolume());

				}
			}
		}
	}

	if (m_once)
	{
		if (um.IsFadeComplete())
		{
			SceneManager::Instance().SetNextScene(
				SceneManager::SceneType::TitleMovie
			);
		}
	}
}