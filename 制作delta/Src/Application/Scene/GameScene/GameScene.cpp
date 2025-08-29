#include "GameScene.h"
#include"../SceneManager.h"

#include "../../GameObject/Terrain/Terrain.h"
#include "../../GameObject/Terrain/AnimTerrain.h"
#include "../../GameObject/Terrain/ParentTerrain.h"

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

#include "../../GameObject/UI/DrawUI/DrawUI.h"
#include "../../GameObject/UI/NumberUI/NumberUI.h"
#include "../../GameObject/UI/UIManager.h"

// 少数第n位で四捨五入する
void round_n(float& number, int n)
{
	number = number * static_cast<float>(pow(10, n - 1));
	number = round(number);
	number /= static_cast<float>(pow(10, n - 1));
}

void GameScene::Init()
{
	KdGameObjectFactory::Instance().RegisterGameObject<Character>("Player");
	KdGameObjectFactory::Instance().RegisterGameObject<Enemy>("Another");
	KdGameObjectFactory::Instance().RegisterGameObject<Drone>("Drone");
	KdGameObjectFactory::Instance().RegisterGameObject<Terrain>("Terrain");
	KdGameObjectFactory::Instance().RegisterGameObject<AnimTerrain>("AnimTerrain");
	KdGameObjectFactory::Instance().RegisterGameObject<NumberUI>("NumberUI");

	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherEnable(true);
	KdShaderManager::Instance().m_StandardShader.SetAlphaDitherDist(6.0f);


	//===================================================================
	// ステージ初期化
	//===================================================================
	
	Math::Vector3 pos = {0.0f,-0.5f,0.0f};
	
	/*std::shared_ptr<ParentTerrain> _terrain = std::make_shared<ParentTerrain>();
	_terrain->Init();
	_terrain->SetModel("Asset/Models/Stage/Sky/Sky.gltf");
	AddObject(_terrain);*/

	CurrentSceneCreate("Asset/Data/Game.scene");
	
	/*std::shared_ptr<Terrain> _terrain = std::make_shared<Terrain>();
	_terrain->SetModel("Asset/Models/Stage/Tail/Tail.gltf");
	_terrain->Init();
	AddObject(_terrain);
	AddTerrain(_terrain);

	_terrain = std::make_shared<Terrain>();
	pos = {30.0f,0.0f,0.0f};
	_terrain->SetPos(pos);
	_terrain->SetModel("Asset/Models/Stage/House/House.gltf");
	_terrain->Init();
	AddObject(_terrain);
	AddTerrain(_terrain);

	_terrain = std::make_shared<Terrain>();
	pos = { -50.0f,0.0f,0.0f };
	_terrain->SetPos(pos);
	_terrain->SetModel("Asset/Models/Stage/Rubble/Rubble.gltf");
	_terrain->Init();
	AddObject(_terrain);
	AddTerrain(_terrain);

	_terrain = std::make_shared<Terrain>();
	pos = { 50.0f,0.0f,50.0f };
	_terrain->SetPos(pos);
	_terrain->SetModel("Asset/Models/Stage/Rubble2/Rubble2.gltf");
	_terrain->Init();
	AddObject(_terrain);
	AddTerrain(_terrain);

	_terrain = std::make_shared<Terrain>();
	pos = { 50.0f,0.0f,300.0f };
	_terrain->SetPos(pos);
	_terrain->SetModel("Asset/Models/Stage/LowApartment/LowApartment.gltf");
	_terrain->Init();
	AddObject(_terrain);
	AddTerrain(_terrain);
	
	_terrain = std::make_shared<Terrain>();
	pos = { -100.0f,0.0f,100.0f };
	_terrain->SetPos(pos);
	_terrain->SetModel("Asset/Models/Stage/dai.gltf");
	_terrain->Init();
	AddObject(_terrain);
	AddTerrain(_terrain);

	_terrain = std::make_shared<Terrain>();
	pos = { -100.0f,0.0f,200.0f };
	_terrain->SetPos(pos);
	_terrain->SetModel("Asset/Models/Stage/BalconyApartment/BalconyApartment.gltf");
	_terrain->Init();
	AddObject(_terrain);
	AddTerrain(_terrain);

	_terrain = std::make_shared<Terrain>();
	_terrain->SetModel("Asset/Models/Stage/Sky/Sky.gltf");
	_terrain->Init();
	AddObject(_terrain);*/

	

	/*std::shared_ptr<Terrain> _serrain = std::make_shared<Terrain>();
	_serrain->Init();
	_serrain->SetPos({ 0.0f,0.0f,0.0f });
	_serrain->SetModel("Asset/Models/Building/Building.gltf");

	AddObject(_serrain);*/



	//===================================================================
	// キャラクター初期化
	//===================================================================
	std::shared_ptr<Character> _character = std::make_shared<Character>();
	_character->SetThis(_character);
	_character->Init();
	AddPlayer(_character);

	//_terrain->SetParent(_character);

	// プレイヤー武器
	{
		/*std::shared_ptr<Blade> _blade = std::make_shared<Blade>();
		_blade->Init();
		_blade->SetModel("Asset/Models/Weapon/Blade/Blade.gltf");
		_blade->SetParent(_character);
		_blade->SetAttachPath("RightWeapon");
		AddObject(_blade);*/

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
		charge->SetGunsParam("Asset/Models/Weapon/RaserCannon/RaserCannon.gltf", 2.5f, 6.0f,0.0f,1, 12, 24);
		charge->MakeAnimator("Close", 20.0f);
		charge->Init();
		charge->SetTag(KdGameObject::tPlayerAttack);
		charge->SetBulletsParam("Asset/Models/Weapon/Bullet/RaserBullet.gltf", 5.0f, 300, 130.0f, 200, 40.0f, 0.95f);
		charge->SetBulletsTrailParam("Asset/Textures/GameObject/Smoke2.png",Math::Color(0.47f, 0.4f, 0.88f), 20.0f, 30);
		AddObject(charge);;
		
		std::shared_ptr<Missile> missile;
		missile = std::make_shared<Missile>();
		missile->SetParent(_character);
		missile->SetAttackTrigger(WeaponBase::RightShoulder);
		missile->SetAttachPath("RightShoulderWeapon");
		missile->SetGunsParam("Asset/Models/Weapon/Missile/FrontMissile/3LineMissile.gltf", 1.8f, 7.0f,0.1f,3, 12, 24);
		missile->MakeAnimator("Close", 20.0f);
		missile->SetNodeMats("SP1");
		missile->SetNodeMats("SP2");
		missile->SetNodeMats("SP3");
		missile->Init();
		missile->SetTag(KdGameObject::tPlayerAttack);
		
		missile->SetBulletsParam("Asset/Models/Weapon/Bullet/MissileBullet.gltf",20.0f,200, 100.0f, 200, 40.0f, 0.95f);
		missile->SetBulletsTrailParam("Asset/Textures/GameObject/Smoke.png", Math::Color(0.36f, 0.3f, 0.3f), 3.5f, 30);
		missile->SetBulletChaisingData(10, 150.0f, 0.8f, 2000.0f);
		AddObject(missile);

	}

	//エネミー
	std::shared_ptr<Enemy> enemy = std::make_shared<Enemy>();
	enemy->SetThis(enemy);
	enemy->Init();
	enemy->SetTag(KdGameObject::tEnemy);
	AddEnemy(enemy);

	//エネミー武器
	std::shared_ptr<Rifle> rifle;
	{
		rifle = std::make_shared<Rifle>();
		rifle->SetParent(enemy);
		rifle->SetAttachPath("RightWeapon");
		rifle->SetAttackTrigger(WeaponBase::RightHand);
		rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
		rifle->SetGunsParam("Asset/Models/Weapon/RailGun/RailGun.gltf", 0.7f, 3.0f, 0.0f, 1, 24, 80);
		rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", 5.0f, 400, 400, 200, 40.0f, 0.95f);
		rifle->SetBulletsTrailParam("Asset/Textures/GameObject/Prazma2.png", Math::Color(0.7f, 0.4f, 0.8f), 1.7f, 10);
		rifle->SetShotSoundPath("Asset/Sounds/SE/Weapon/Shot_Raifl.wav");
		AddObject(rifle);
	}
	{
		rifle = std::make_shared<Rifle>();
		rifle->SetParent(enemy);
		rifle->SetAttachPath("LeftWeapon");
		rifle->SetAttackTrigger(WeaponBase::LeftHand);
		rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
		rifle->SetGunsParam("Asset/Models/Weapon/LinearRifle/LinearRifle.gltf", 0.2f, 2.0f, 0.0f, 1, 36, 250);
		rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", 5.0f, 56, 300, 200, 20.0f, 0.9f);
		rifle->SetBulletsTrailParam("Asset/Textures/GameObject/ClockHand.png", Math::Color(0.7f, 0.4f, 0.1f), 1.7f, 20);
		rifle->SetShotSoundPath("Asset/Sounds/SE/Weapon/Shot_Raifl.wav");
		AddObject(rifle);
	}




	_character->RegistHitObject(enemy);

	auto drone = std::make_shared<Drone>();
	drone->SetThis(drone);
	drone->SetModelWork("Asset/Models/Drone/Drone.gltf");
	drone->Init();
	drone->SetParam(100);
	drone->SetPos({ 10.0f,15.0f,150.0f });
	drone->SetTag(KdGameObject::tEnemy);
	AddEnemy(drone);
	

	// バルカン想定
	rifle = std::make_shared<Rifle>();
	rifle->SetParent(drone);
	rifle->SetAttachPath("RightWeapon");
	rifle->SetAttackTrigger(WeaponBase::RightHand);
	rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
	rifle->SetGunsParam("Asset/Models/Weapon/Balkan/Balkan.gltf", 0.078f, 5.0f, 0.0f, 1, 80, 300);
	rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", 2.0f, 16, 400, 50, 10.0f, 0.9f);
	rifle->SetBulletsTrailParam("Asset/Textures/GameObject/ClockHand.png", Math::Color(1.0f, 0.17f, 0.19f), 1.7f, 10);
	rifle->SetShotSoundPath("Asset/Sounds/SE/Weapon/Shot_Balkan.wav");
	AddObject(rifle);

	drone = std::make_shared<Drone>();
	drone->SetThis(drone);
	drone->SetModelWork("Asset/Models/Drone/Drone.gltf");
	drone->Init();
	drone->SetParam(100);
	drone->SetPos({ 10.0f,15.0f,170.0f });
	drone->SetTag(KdGameObject::tEnemy);
	AddEnemy(drone);


	// バルカン想定
	rifle = std::make_shared<Rifle>();
	rifle->SetParent(drone);
	rifle->SetAttachPath("RightWeapon");
	rifle->SetAttackTrigger(WeaponBase::RightHand);
	rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
	rifle->SetGunsParam("Asset/Models/Weapon/Balkan/Balkan.gltf", 0.078f, 5.0f, 0.0f, 1, 80, 300);
	rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", 2.0f, 16, 400, 50, 10.0f, 0.9f);
	rifle->SetBulletsTrailParam("Asset/Textures/GameObject/ClockHand.png", Math::Color(1.0f, 0.17f, 0.19f), 1.7f, 10);
	rifle->SetShotSoundPath("Asset/Sounds/SE/Weapon/Shot_Balkan.wav");
	AddObject(rifle);

	// 4脚
	drone = std::make_shared<Drone>();
	drone->SetThis(drone);
	drone->SetModelWork("Asset/Models/4LegMT/5LegMT.gltf");
	drone->Init();
	drone->SetParam(500);
	drone->SetPos({ -100.0f,0.0f,50.0f });
	drone->SetTag(KdGameObject::tEnemy);
	AddEnemy(drone);

	rifle = std::make_shared<Rifle>();
	rifle->SetParent(drone);
	rifle->SetAttachPath("RightWeapon");
	rifle->SetAttackTrigger(WeaponBase::RightHand);
	rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
	rifle->SetGunsParam("Asset/Models/Weapon/GrenadeLauncher/GrenadeLauncher.gltf", 3.0f, 5.0f, 0.0f, 1, 5, 25);
	rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/GrenadeBullet.gltf", 2.0f, 790, 300, 50, 10.0f, 0.9f);
	rifle->SetBulletsTrailParam("Asset/Textures/GameObject/ClockHand.png", Math::Color(0.9f, 0.2f, 0.1f), 1.7f, 10);
	rifle->SetShotSoundPath("Asset/Sounds/SE/Weapon/Shot_Launchur.wav");
	AddObject(rifle);

	rifle = std::make_shared<Rifle>();
	rifle->SetParent(drone);
	rifle->SetAttachPath("LeftWeapon");
	rifle->SetAttackTrigger(WeaponBase::LeftHand);
	rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
	rifle->SetGunsParam("Asset/Models/Weapon/GrenadeLauncher/GrenadeLauncher.gltf", 3.0f, 5.0f, 0.0f, 1, 5, 25);
	rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/GrenadeBullet.gltf", 2.0f, 790, 300, 50, 10.0f, 0.9f);
	rifle->SetBulletsTrailParam("Asset/Textures/GameObject/ClockHand.png", Math::Color(0.9f, 0.1f, 0.1f), 1.7f, 10);
	rifle->SetShotSoundPath("Asset/Sounds/SE/Weapon/Shot_Launchur.wav");
	AddObject(rifle);

	auto mt = std::make_shared<MT>();
	mt->SetThis(mt);
	mt->SetModelWork("Asset/Models/Leg/Leg.gltf");
	mt->Init();
	mt->SetParam(3000);
	mt->SetPos({ -100.0f,0.0f,-200.0f });
	mt->SetTag(KdGameObject::tEnemy);
	AddEnemy(mt);

	rifle = std::make_shared<Rifle>();
	rifle->SetParent(mt);
	rifle->SetAttachPath("RightWeapon");
	rifle->SetAttackTrigger(WeaponBase::RightHand);
	rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
	rifle->SetGunsParam("Asset/Models/Weapon/SMG/MP5.gltf", 0.08f, 5.0f, 0.0f, 1, 40, 250);
	rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", 2.0f, 45, 300, 50, 10.0f, 0.9f);
	rifle->SetBulletsTrailParam("Asset/Textures/GameObject/ClockHand.png", Math::Color(0.7f, 0.3f, 0.2f), 1.7f, 10);
	rifle->SetShotSoundPath("Asset/Sounds/SE/Weapon/Shot_Masingun.wav");
	AddObject(rifle);	

	UIManager::GetInstance().SceneUICreate();


	//===================================================================
	// カメラ初期化
	//===================================================================
	CameraManager::Instance().EnableChangedCamera(true);
	CameraManager::Instance().SetCameraTarget(_character);
	//CameraManager::Instance().SetLockTarget(enemy);
	CameraManager::Instance().SetNextType(CameraManager::CameraType::Tracking);


	m_duration = 1.0f;
	m_fade = false;
	m_once = false;

	KdAudioManager::Instance().StopAllSound();
	KdAudioManager::Instance().Play("Asset/Sounds/BGM/Rusty.wav", true)->SetVolume(0.1f);

}

void GameScene::Event()
{
	auto& key = KeyInput::GetInstance().GetKeyboardState();
	if (m_enemyList.empty() == true || key.D9) {
		if (!m_once)
		{
			m_once = true;
			UIManager::GetInstance().SceneUICreate("Asset/Data/CompleteUI.scene");
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