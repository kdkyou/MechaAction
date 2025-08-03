#include "GameScene.h"
#include"../SceneManager.h"

#include "../../GameObject/Terrain/Terrain.h"
#include "../../GameObject/Terrain/AnimTerrain.h"

#include "../../GameObject/Character/Character.h"
#include "../../GameObject/Enemy/Enemy.h"
#include "../../GameObject/Enemy/Drone/Drone.h"

#include "../../GameObject/Weapon/Blade/Blade.h"
#include "../../GameObject/Weapon/Shield/Shield.h"
#include "../../GameObject/Weapon/Sowrd/Sowrd.h"
#include "../../GameObject/Weapon/Gun/Rifle/Rifle.h"
#include "../../GameObject/Weapon/Gun/Missile/Missile.h"
#include "../../GameObject/Weapon/Gun/Charge/Charge.h"

#include "../../GameObject/Camera/CameraManager.h"

#include "../../GameObject/UI/DrawUI/DrawUI.h"
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


	//===================================================================
	// ステージ初期化
	//===================================================================
	
	Math::Vector3 pos = {0.0f,-0.5f,0.0f};

	
	std::shared_ptr<Terrain> _terrain = std::make_shared<Terrain>();
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
	pos = { -30.0f,0.0f,0.0f };
	_terrain->SetPos(pos);
	_terrain->SetModel("Asset/Models/Stage/Rubble/Rubble.gltf");
	_terrain->Init();
	AddObject(_terrain);
	AddTerrain(_terrain);

	_terrain = std::make_shared<Terrain>();
	pos = { 30.0f,0.0f,50.0f };
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
	pos = { -100.0f,0.0f,200.0f };
	_terrain->SetPos(pos);
	_terrain->SetModel("Asset/Models/Stage/BalconyApartment/BalconyApartment.gltf");
	_terrain->Init();
	AddObject(_terrain);
	AddTerrain(_terrain);

	_terrain = std::make_shared<Terrain>();
	_terrain->SetModel("Asset/Models/Stage/Sky/Sky.gltf");
	_terrain->Init();
	AddObject(_terrain);

	/*auto ta = std::make_shared<AnimTerrain>();
	ta->SetModel("Asset/Models/Stage/Door/Door.gltf");
	pos = { 0.0f,0.0f,10.0f };
	ta->SetPos(pos);
	ta->Init();
	AddObject(ta);
	AddTerrain(ta);*/

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
	AddObject(_character);
	AddPlayer(_character);

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
		_sowrd->SetTag(KdGameObject::tPlayerAttack);
		AddObject(_shield);
		AddTerrain(_shield);


		std::shared_ptr<Charge> charge;

		charge = std::make_shared<Charge>();
		charge->SetParent(_character);
		charge->SetAttackTrigger(WeaponBase::LeftShoulder);
		charge->SetAttachPath("LeftShoulderWeapon");
		charge->SetGunsParam("Asset/Models/Weapon/RaserCannon/RaserCannon.gltf", 3.5f, 6.0f,0.0f,1, 24, 80);
		charge->MakeAnimator("Close", 20.0f);
		charge->Init();
		charge->SetTag(KdGameObject::tPlayerAttack);
		charge->SetBulletsParam("", 5.0f, 300, 300, 200, 40.0f, 0.95f);
		charge->SetBulletsTrailParam("Asset/Textures/GameObject/Smoke2.png",Math::Color(0.47f, 0.4f, 0.88f), 20.0f, 30);
		AddObject(charge);;
		
		std::shared_ptr<Missile> missile;
		missile = std::make_shared<Missile>();
		missile->SetParent(_character);
		missile->SetAttackTrigger(WeaponBase::RightShoulder);
		missile->SetAttachPath("RightShoulderWeapon");
		missile->SetGunsParam("Asset/Models/Weapon/Missile/FrontMissile/3LineMissile.gltf", 1.8f, 7.0f,0.1f,3, 24, 80);
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
	AddObject(enemy);
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
		AddObject(rifle);
	}


	_character->RegistHitObject(enemy);

	auto drone = std::make_shared<Drone>();
	drone->SetThis(drone);
	drone->SetModelWork("Asset/Models/Drone/Drone.gltf");
	drone->Init();
	drone->SetParam(100);
	drone->SetPos({ 0.0f,15.0f,20.0f });
	drone->SetTag(KdGameObject::tEnemy);
	AddObject(drone);
	AddEnemy(drone);

	auto balkan = std::make_shared<Rifle>();
	rifle = std::make_shared<Rifle>();
	rifle->SetParent(drone);
	rifle->SetAttachPath("RightWeapon");
	rifle->SetAttackTrigger(WeaponBase::RightHand);
	rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
	rifle->SetGunsParam("Asset/Models/Weapon/Balkan/Balkan.gltf", 0.078f, 5.0f, 0.0f, 1, 80, 300);
	rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", 2.0f, 16, 400, 50, 10.0f, 0.9f);
	rifle->SetBulletsTrailParam("Asset/Textures/GameObject/ClockHand.png", Math::Color(0.9f, 0.2f, 0.1f), 1.7f, 10);
	AddObject(rifle);


	drone = std::make_shared<Drone>();
	drone->SetThis(drone);
	drone->SetModelWork("Asset/Models/4LegMT/5LegMT.gltf");
	drone->Init();
	drone->SetParam(100);
	drone->SetPos({ -100.0f,0.0f,20.0f });
	drone->SetTag(KdGameObject::tEnemy);
	AddObject(drone);
	AddEnemy(drone);

	rifle = std::make_shared<Rifle>();
	rifle->SetParent(drone);
	rifle->SetAttachPath("RightWeapon");
	rifle->SetAttackTrigger(WeaponBase::RightHand);
	rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
	rifle->SetGunsParam("Asset/Models/Weapon/GrenadeLauncher/GrenadeLauncher.gltf", 3.0f, 5.0f, 0.0f, 1, 5, 25);
	rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", 2.0f, 790, 300, 50, 10.0f, 0.9f);
	rifle->SetBulletsTrailParam("Asset/Textures/GameObject/ClockHand.png", Math::Color(0.9f, 0.2f, 0.1f), 1.7f, 10);
	AddObject(rifle);

	rifle = std::make_shared<Rifle>();
	rifle->SetParent(drone);
	rifle->SetAttachPath("LeftWeapon");
	rifle->SetAttackTrigger(WeaponBase::LeftHand);
	rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
	rifle->SetGunsParam("Asset/Models/Weapon/GrenadeLauncher/GrenadeLauncher.gltf", 3.0f, 5.0f, 0.0f, 1, 5, 25);
	rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", 2.0f, 790, 300, 50, 10.0f, 0.9f);
	rifle->SetBulletsTrailParam("Asset/Textures/GameObject/ClockHand.png", Math::Color(0.9f, 0.2f, 0.1f), 1.7f, 10);
	AddObject(rifle);

	auto ui = std::make_shared<DrawUI>();
	ui->Init();
	ui->SetTexture("Asset/Textures/UI/BlueUI.png");
	AddObject(ui);

	ui = std::make_shared<DrawUI>();
	ui->Init();
	ui->SetTexture("Asset/Textures/UI/MainLock.png");
	AddObject(ui);

	//===================================================================
	// カメラ初期化
	//===================================================================
	CameraManager::Instance().EnableChangedCamera(true);
	CameraManager::Instance().SetCameraTarget(_character);
	//CameraManager::Instance().SetLockTarget(enemy);
	CameraManager::Instance().SetNextType(CameraManager::CameraType::Tracking);



}

void GameScene::Event()
{
	if (KeyInput::GetInstance().GetKeyboardState().H)
	{
		auto instance = KdAudioManager::Instance().Play3D("Asset/Sounds/Thruster2.wav", { 0.0f,0.0f,10.0f });
		instance->SetVelocity({ 0.0f,0.0f,1.0f });
		instance->SetCurveDistanceScaler(1.0f);
		instance->SetVolume(1);
		instance->SetInnerRadiusAngle(45);
	}
	if (KeyInput::GetInstance().GetKeyboardState().J)
	{
		KdAudioManager::Instance().Play("Asset/Sounds/Thruster2.wav");
	}
}