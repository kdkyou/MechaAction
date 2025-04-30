#include "GameScene.h"
#include"../SceneManager.h"

#include "../../GameObject/Terrain/Terrain.h"
#include "../../GameObject/Character/Character.h"
#include"../../GameObject/Weapon/Blade/Blade.h"

#include"../../GameObject/Enemy/Enemy.h"

#include "../../GameObject/Camera/FPSCamera/FPSCamera.h"
#include "../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include"../../GameObject/Camera/TrackingCamera/TrackingCamera.h"
#include"../../GameObject/Camera/RockCamera/RockCamera.h"

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
	AddObject(_terrain);

	//===================================================================
	// キャラクター初期化
	//===================================================================
	std::shared_ptr<Character> _character = std::make_shared<Character>();
	_character->Init();
	_character->RegistHitObject(_terrain);
	AddObject(_character);

	std::shared_ptr<Blade> _blade = std::make_shared<Blade>();
	_blade->Init();
	_blade->SetParent(_character);
	AddObject(_blade);

	//エネミー
	std::shared_ptr<Enemy> enemy = std::make_shared<Enemy>();
	enemy->Init();
	AddObject(enemy);

	//===================================================================
	// カメラ初期化
	//===================================================================
//	std::shared_ptr<FPSCamera>		_camera = std::make_shared<FPSCamera>();
//	std::shared_ptr<TPSCamera>		_camera = std::make_shared<TPSCamera>();
	std::shared_ptr<TrackingCamera>		_camera = std::make_shared<TrackingCamera>();
//	std::shared_ptr<RockCamera>		_camera = std::make_shared<RockCamera>();
	_camera->Init();
	_camera->SetTarget(_character);
//	_camera->SetRock(enemy);
	_camera->RegistHitObject(_terrain);
	_character->SetCamera(_camera);
	AddObject(_camera);
}

void GameScene::Event()
{
}