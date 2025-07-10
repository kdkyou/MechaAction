#include "GameScene.h"
#include"../SceneManager.h"

#include "../../GameObject/Terrain/Terrain.h"
#include "../../GameObject/Character/Character.h"
#include"../../GameObject/Enemy/Enemy.h"

#include"../../GameObject/Weapon/Blade/Blade.h"
#include"../../GameObject/Weapon/Shield/Shield.h"
#include"../../GameObject/Weapon/Sowrd/Sowrd.h"
#include"../../GameObject/Weapon/Rifle/Rifle.h"

#include"../../GameObject/Camera/CameraManager.h"
// 少数第n位で四捨五入する
void round_n(float& number, int n)
{
	number = number * static_cast<float>(pow(10, n - 1));
	number = round(number);
	number /= static_cast<float>(pow(10, n - 1));
}

void GameScene::Init()
{
	//===================================================================
	// ステージ初期化
	//===================================================================
	std::shared_ptr<Terrain> _terrain = std::make_shared<Terrain>();
	_terrain->Init();
	_terrain->SetModel("Asset/Models/Stage/Stage.gltf");
	AddObject(_terrain);


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
	_character->RegistHitObject(_terrain);
	AddObject(_character);
	AddPlayer(_character);

	// プレイヤー武器
	{
		std::shared_ptr<Blade> _blade = std::make_shared<Blade>();
		_blade->Init();
		_blade->SetModel("Asset/Models/Weapon/Blade/Blade.gltf");
		_blade->SetParent(_character);
		_blade->SetAttachPath("RightWeapon");
		AddObject(_blade);

		std::shared_ptr<Sowrd> _sowrd = std::make_shared<Sowrd>();
		_sowrd->Init();
		_sowrd->SetModel("Asset/Models/Weapon/Sowrd/Sowrd.gltf");
		_sowrd->SetParent(_character);
		_sowrd->SetAttachPath("RightHand");
		AddObject(_sowrd);


		std::shared_ptr<Shield> _shield = std::make_shared<Shield>();
		_shield->Init();
		_shield->SetModel("Asset/Models/Weapon/Shield/Shield.gltf");
		_shield->SetParent(_character);
		_shield->SetAttachPath("LeftWeapon");
		AddObject(_shield);


		std::shared_ptr<Rifle> rifle;

		rifle = std::make_shared<Rifle>();
		rifle->SetParent(_character);
		rifle->SetAttackTrigger(WeaponBase::LeftShoulder);
		rifle->SetAttachPath("LeftShoulderWeapon");
		rifle->SetGunsParam("Asset/Models/Weapon/RaserCannon/RaserCannon.gltf", 2.5f, 5.0f, 24, 80);
		rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", "Asset/Textures/GameObject/Thunder1.png", 200, 500, 200, 40.0f, 0.95f);
		AddObject(rifle);
		
		rifle = std::make_shared<Rifle>();
		rifle->SetParent(_character);
		rifle->SetAttackTrigger(WeaponBase::RightShoulder);
		rifle->SetAttachPath("RightShoulderWeapon");
		rifle->SetGunsParam("Asset/Models/Weapon/Missile/FrontMissile/FrontMisail.gltf", 1.8f, 5.0f, 24, 80);
		rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", "Asset/Textures/GameObject/Thunder1.png", 200, 500, 200, 40.0f, 0.95f);
		AddObject(rifle);


	}




	//エネミー
	std::shared_ptr<Enemy> enemy = std::make_shared<Enemy>();
	enemy->SetThis(enemy);
	enemy->SetTarget(_character);
	enemy->Init();
	AddObject(enemy);
	AddEnemy(enemy);

	//エネミー武器
	std::shared_ptr<Rifle> rifle;
	{
		rifle = std::make_shared<Rifle>();
		rifle->SetParent(enemy);
		rifle->SetAttachPath("RightWeapon");
		rifle->SetGunsParam("Asset/Models/Weapon/RailGun/RailGun.gltf", 1.8f, 5.0f, 24, 80);
		rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", "Asset/Textures/GameObject/Prazma1.png", 200, 500, 200, 40.0f, 0.95f);
		AddObject(rifle);
	}
	{
		rifle = std::make_shared<Rifle>();
		rifle->SetParent(enemy);
		rifle->SetAttachPath("LeftWeapon");
		rifle->SetGunsParam("Asset/Models/Weapon/LinearRifle/LinearRifle.gltf", 0.5f, 2.0f, 36, 250);
		rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", "Asset/Textures/GameObject/ClockHand.png", 50, 300, 200, 20.0f, 0.9f);
		AddObject(rifle);
	}


	_character->RegistHitObject(enemy);

	//===================================================================
	// カメラ初期化
	//===================================================================
	CameraManager::Instance().SetCameraTarget(_character);
	CameraManager::Instance().SetLockTarget(enemy);
	CameraManager::Instance().SetNextType(CameraManager::CameraType::None);



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