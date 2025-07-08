#include "Character.h"

#include "../../main.h"
#include "../Camera/CameraBase.h"

#include"../../Scene/SceneManager.h"
#include"../Camera/CameraManager.h"

//#include"../Effect/ConcentrationLine/ConcentrationLine.h"

#include"TransAC.h"

void Character::Init()
{
	if (!m_spModelWork)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData("Asset/Models/Grint/Grint.gltf");

		// 初期のアニメーションをセットする
		m_spAnimator = std::make_shared<KdAnimator>();
		m_spAnimator->SetAnimation(m_spModelWork->GetData()->GetAnimation("Stand"), 1.0f, true, true);
	}

	m_gravity = 0;
	m_mWorld = Math::Matrix::Identity;
	SetPos({ 0, 1.0f, 0 });

	

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
	m_pCollider = std::make_unique<KdCollider>();

	InitTrail();
	UnEnableTrail();

	//初期状態を「待機状態」へ設定
	ChangeActionState(std::make_shared<ActionIdle>());



}

void Character::Update()
{

	Application::Instance().m_log.Clear();

	auto spThis = m_wpThis.lock();

	color = { 0,1,0,1 };


	m_wpCamera = CameraManager::Instance().GetCurrentCamera();

	auto& key = KeyInput::GetInstance().GetKeyboardState();
	auto& mouse = KeyInput::GetInstance().GetMouseState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	if (key.I || mouse.middleButton || pad.IsRightStickPressed())
	{
		CameraManager::Instance().SetNextType(CameraManager::Rock);
	}
	if (key.U)
	{
		CameraManager::Instance().SetNextType(CameraManager::Tracking);
	}
	if (key.Y)
	{
		CameraManager::Instance().SetNextType(CameraManager::None);
	}

	if (key.H)
	{
		CameraManager::Instance().SetNextType(CameraManager::Hit);
	}

	if (key.T)
	{
		ChangeActionState(std::make_shared<ActionHited>());
		return;

	}

	if (key.O)
	{
		//ブルーム
		KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(0.75f);
		// リムライト
		m_limEnable = false;

		m_transAC = false;

		m_speedMag = 1.0f;

	}

	if (key.P)
	{
		//ブルーム
		KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(0.15f);
		// リムライト
		m_limEnable = true;

		m_transAC = true;

		m_speedMag = 1.5f;
	}

	if (key.L)
	{
		ChangeActionState(std::make_shared<ActionDestroyed>());
		return;
	}


	if (spThis)
	{
		//各種「状態に応じた」更新処理を実行する
		if (m_nowAction)
		{
			m_nowAction->Update(m_wpThis);
		}
	}
	// 重力の更新()
	Move(m_gravity, Math::Vector3::Down, KdCollider::TypeGround, false);
	m_gravity += m_gravityPow * KdFPSController::GetInstance().GetDeltaTime();

	//1m_pCollider->RegisterCollisionShape(KdCollider::TypeBump)


	// キャラクターの座標が確定してからコリジョンによる位置補正を行う
	UpdateCollision();
}

void Character::PostUpdate()
{
	if (m_nowAction)
	{
		m_nowAction->PostUpdate(m_wpThis);
	}

	m_spModelWork->CalcNodeMatrices();

	AddTrail();

}

void Character::DrawLit()
{
	if (!m_spModelWork) return;

	if (m_limEnable)
	{
		KdShaderManager::Instance().m_StandardShader.SetLimLightEnable(true);
		KdShaderManager::Instance().m_StandardShader.SetLimlightParam(m_limColor, m_limPow);

	}

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld, kWhiteColor, Math::Vector3{ 1,1,1 });

	KdShaderManager::Instance().m_StandardShader.SetLimLightEnable(false);
}

void Character::DrawUnLit()
{
	if (m_spTrails.size() <= 0) { return; }

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	for (auto& trail : m_spTrails)
	{
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(*trail->trail);
	}

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
}

void Character::Editor_ImGui()
{
	ImGui::Text("CharacterSpeed");
	ImGui::SliderFloat("Walk", &m_walkSpeed, 0.0f, 100.0f);
	ImGui::SliderFloat("Jump", &m_jumpSpeed, 0.0f, 100.0f);
	ImGui::SliderFloat("Boost", &m_boostSpeed, 0.0f, 300.0f);
	ImGui::SliderFloat("BoostEnd", &m_boostEndSpeed, 0.0f, 300.0f);
	ImGui::SliderFloat("BoostDush", &m_boostDushSpeed, 0.0f, 300.0f);
	ImGui::SliderFloat("BladeAttack", &m_bladeAttackSpeed, 0.0f, 500.0f);
	ImGui::SliderFloat("Hit", &m_hitedSpeed, 0.0f, 50.0f);

	ImGui::DragFloat3("LimColor", &m_limColor.x, 0.01f);
	ImGui::DragFloat("LimPow", &m_limPow, 0.01f);

}

const bool Character::IsMove()
{
	bool move = false;
	m_vMove = Math::Vector3::Zero;

	auto& key = KeyInput::GetInstance().GetKeyboardState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	if (key.W || pad.IsLeftThumbStickUp()) {
		m_vMove.z += 1.f;
		move = true;
	}
	if (key.A || pad.IsLeftThumbStickLeft()) {
		m_vMove.x -= 1.f;
		move = true;
	}
	if (key.S || pad.IsLeftThumbStickDown()) {
		m_vMove.z -= 1.f;
		move = true;
	}
	if (key.D || pad.IsLeftThumbStickRight()) {
		m_vMove.x += 1.f;
		move = true;
	}

	if (move) { return true; }
	else { return false; }
}

const bool Character::IsBoost()
{
	auto& key = KeyInput::GetInstance().GetKeyboardState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	if (key.LeftShift || key.RightShift || pad.IsXPressed())
	{
		return true;
	}

	return false;
}

const bool Character::IsAttack()
{
	auto& mouse = KeyInput::GetInstance().GetMouseState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	if (mouse.rightButton || pad.IsRightTriggerPressed())
	{
		ChangeEnableRightAttack(true);

		return true;
	}

	return false;
}

const bool Character::IsFlow()
{
	auto& key = KeyInput::GetInstance().GetKeyboardState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	if (key.Space || pad.IsBPressed())
	{
		return true;
	}
	return false;
}

const bool Character::IsGuard()
{
	auto& mouse = KeyInput::GetInstance().GetMouseState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	if (mouse.leftButton || pad.IsLeftTriggerPressed())
	{
		return true;
	}
	return false;
}

bool Character::Move(float speed, const Math::Vector3& dir, const KdCollider::Type type, bool ray, bool camera, bool step)
{
	auto direction = dir;
	direction.Normalize();

	auto pos = m_mWorld.Translation();

	if (step == true)
	{
		pos.y += m_stepHigh.y;
	}



	auto deltaTime = KdFPSController::GetInstance().GetDeltaTime();

	Math::Vector3 move = Math::Vector3::Zero;

	auto deltaSpeed = speed * deltaTime;

	bool isHit = RayCast(pos, direction, deltaSpeed, type, move);
	if (ray == true)
	{
		return isHit;
	}

	if (isHit == true)
	{
		auto corre = (direction * deltaSpeed) * 0.1f;
		pos = move - corre;
	}
	else
	{
		move = direction * deltaSpeed;
		pos += move;
	}

	if (step == true)
	{
		pos.y -= m_stepHigh.y;
	}

	if (camera == true)
	{
		UpdateRotate(direction);
	}


	Math::Matrix _rotation = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_worldRot.y));
	m_mWorld = m_scale * _rotation * Math::Matrix::CreateTranslation(pos);


	return false;
}

void Character::UpdateRotate(const Math::Vector3& srcMoveVec)
{
	// 何も入力が無い場合は処理しない
	if (srcMoveVec.LengthSquared() == 0.0f) { return; }

	if (srcMoveVec.y < 0.0f) { return; }

	// キャラの正面方向のベクトル
	Math::Vector3 nowDir = GetMatrix().Backward();

	// 移動方向のベクトル
	Math::Vector3 targetDir = srcMoveVec;

	nowDir.Normalize();
	targetDir.Normalize();

	//内積を使って回転する角度を求める
	float d = nowDir.Dot(targetDir);
	//dの中にはコサインΘが入っている

	//角度求める(でも残念ながらラジアン角)11
	float ang = DirectX::XMConvertToDegrees(acos(d));

	//内積から角度を求めて少しでも角度が変わったら
	//ゆっくり回転するようにする
	if (ang >= 0.1f)
	{
		if (ang > m_clampSize)
		{
			ang = m_clampSize;
		}

		//外積を求める（どっっちに回転するのか調べる）
		Math::Vector3 c = targetDir.Cross(nowDir);

		if (c.y >= 0)
		{
			//右回転
			m_worldRot.y -= ang;
		}
		else
		{
			//左回転
			m_worldRot.y += ang;
		}
	}

	if (m_worldRot.y > 360)
	{
		m_worldRot.y -= 360;
	}
	else if (m_worldRot.y < 0)
	{
		m_worldRot.y += 360;
	}

}

void Character::UpdateCollision()
{


	// その他球による衝突判定
	// ---- ---- ---- ---- ---- ----
	// ①当たり判定(球判定)用の情報を作成
	KdCollider::SphereInfo spherInfo;
	spherInfo.m_sphere.Center = GetPos() + Math::Vector3(0, 2.5f, 0);
	spherInfo.m_sphere.Radius = 5.0f;
	spherInfo.m_type = KdCollider::TypeBump;

	// ②HIT対象オブジェクトに総当たり
	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			std::list<KdCollider::CollisionResult> retBumpList;
			spGameObj->Intersects(spherInfo, nullptr);

			// ③結果を使って座標を補完する
			for (auto& ret : retBumpList)
			{
			}
		}
	}

	spherInfo.m_sphere.Center = GetPos() + Math::Vector3(0, 3.5f, 0);
	spherInfo.m_sphere.Radius = 40.0f;
	spherInfo.m_type = KdCollider::TypeDamage;

	//
	m_wpRockTarget.reset();

	// ②HIT対象オブジェクトに総当たり
	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			std::list<KdCollider::CollisionResult> retList;
			if (spGameObj->Intersects(spherInfo, nullptr))
			{
				m_wpRockTarget = spGameObj;
				color = { 0,0,1,1 };
			}
		}
	}


	DirectX::BoundingOrientedBox box;

	box.Center = GetPos() + Math::Vector3(0.0f, 6.0f, 0.0f) ;
	box.Extents = { 3.0f,5.0f,3.0f };
	UINT type = KdCollider::TypeDamage;
	KdCollider::BoxInfo boxInfo(type, box);
	
	auto translation = m_mWorld.Translation();

	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			std::list<KdCollider::CollisionResult> retList;
			if (spGameObj->Intersects(boxInfo, &retList))
			{
				color = { 1,0,1,1 };
				for (auto& ret : retList)
				{
					Math::Vector3 nowPos = translation + (ret.m_hitDir * ret.m_overlapDistance);
						//	SetPos(nowPos);
				}
			}
		}
	}

	Application::Instance().m_log.AddLog("pos x:%.2f,y:%.2f,z:%.2f\n", translation.x, translation.y, translation.z);

	m_pDebugWire->AddDebugSphere(spherInfo.m_sphere.Center, spherInfo.m_sphere.Radius, color);
	m_pDebugWire->AddDebugBox(m_mWorld, box.Extents, {}, true, color);

}

bool Character::RayCast(const Math::Vector3& startPos, const Math::Vector3& vec, const float length, const KdCollider::Type& type, Math::Vector3& resultPos)
{
	KdCollider::RayInfo rayInfo;

	rayInfo.m_pos = startPos;		// レイの発射位置を設定

	rayInfo.m_dir = vec;				// レイの発射方向を設定

	rayInfo.m_range = length;		// レイの長さ

	// 当たり判定をしたいタイプを設定
	rayInfo.m_type = type;

	if (rayInfo.m_dir.Length() == 0) { return false; }

	bool hit = false;

	// ②HIT判定対象オブジェクトに総当たり
	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			std::list<KdCollider::CollisionResult> retRayList;
			spGameObj->Intersects(rayInfo, &retRayList);

			// ③結果を使って座標を補完する
			// レイに当たったリストから一番近いオブジェクトを検出
			float maxOverLap = 0;
			Math::Vector3 hitPos = Math::Vector3::Zero;
			for (auto& ret : retRayList)
			{
				// レイを遮断しオーバーした長さが
				// 一番長いものを探す
				if (maxOverLap < ret.m_overlapDistance)
				{
					maxOverLap = ret.m_overlapDistance;
					resultPos = ret.m_hitPos;

					hit = true;

				}
			}

			if (type & KdCollider::TypeGround)
			{
				if (hit)
				{
					m_isGround = true;
					m_gravity = 0.0f;
				}
				else
				{
					m_isGround = false;
				}
			}
		}
	}

	return hit;
}

bool Character::SphereCast(const Math::Vector3& center, const Math::Vector3& vec, const float radius, const KdCollider::Type& type, Math::Vector3& resultPos)
{
	// その他球による衝突判定
	// ---- ---- ---- ---- ---- ----
	// ①当たり判定(球判定)用の情報を作成
	KdCollider::SphereInfo spherInfo;
	spherInfo.m_sphere.Center = GetPos() + Math::Vector3(0, 0.5f, 0);
	spherInfo.m_sphere.Radius = 0.5f;
	spherInfo.m_type = KdCollider::TypeGround;

	// ②HIT対象オブジェクトに総当たり
	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			std::list<KdCollider::CollisionResult> retBumpList;
			spGameObj->Intersects(spherInfo, &retBumpList);

			// ③結果を使って座標を補完する
			for (auto& ret : retBumpList)
			{
				Math::Vector3 newPos = GetPos() + (ret.m_hitDir * ret.m_overlapDistance);
				SetPos(newPos);
			}
		}
	}

	return false;
}

void Character::OverTrans(const std::string& nowAnimName, float animProgress)
{
	std::shared_ptr<TransAC> trans = std::make_shared<TransAC>();
	std::string modelpath = "Asset/Models/Grint/Grint.gltf";
	std::string animpath = nowAnimName;
	trans->SetTransACData(modelpath, animpath, animProgress, m_mWorld, 0.3f, 1.0f, { 0.8f,0.0f,0.2f });
	SceneManager::Instance().AddObject(trans);
}

void Character::InitTrail()
{
	std::shared_ptr<TrailParam> newObj = nullptr;
	std::string name = LEFTUP;
	newObj = std::make_shared<TrailParam>();
	newObj->name = name;
	newObj->trail = std::make_shared<KdTrailPolygon>();
	newObj->trail->SetMaterial(KdAssets::Instance().m_textures.LoadData("Asset/Textures/GameObject/Trail.png"));
	//	trail->SetColor(Math::Color{ 3.0f,3.0f,3.0f });
	newObj->trail->SetPattern(KdTrailPolygon::Trail_Pattern::eBillboard);
	newObj->trail->SetWidth(1.6f);
	newObj->trail->SetLength(30);
	newObj->trail->ClearPoints();
	m_spTrails.push_back(newObj);

	name = RIGHTUP;
	newObj = std::make_shared<TrailParam>();
	newObj->name = name;
	newObj->trail = std::make_shared<KdTrailPolygon>();
	newObj->trail->SetMaterial(KdAssets::Instance().m_textures.LoadData("Asset/Textures/GameObject/Trail.png"));
	//	trail->SetColor(Math::Color{ 3.0f,3.0f,3.0f });
	newObj->trail->SetPattern(KdTrailPolygon::Trail_Pattern::eBillboard);
	newObj->trail->SetWidth(1.6f);
	newObj->trail->SetLength(30);
	newObj->trail->ClearPoints();
	m_spTrails.push_back(newObj);

	name = LEFTDOWN;
	newObj = std::make_shared<TrailParam>();
	newObj->name = name;
	newObj->trail = std::make_shared<KdTrailPolygon>();
	newObj->trail->SetMaterial(KdAssets::Instance().m_textures.LoadData("Asset/Textures/GameObject/Trail.png"));
	//	trail->SetColor(Math::Color{ 3.0f,3.0f,3.0f });
	newObj->trail->SetPattern(KdTrailPolygon::Trail_Pattern::eBillboard);
	newObj->trail->SetWidth(1.7f);
	newObj->trail->SetLength(30);
	newObj->trail->ClearPoints();
	m_spTrails.push_back(newObj);

	name = RIGHTDOWN;
	newObj = std::make_shared<TrailParam>();
	newObj->name = name;
	newObj->trail = std::make_shared<KdTrailPolygon>();
	newObj->trail->SetMaterial(KdAssets::Instance().m_textures.LoadData("Asset/Textures/GameObject/Trail.png"));
	//	trail->SetColor(Math::Color{ 3.0f,3.0f,3.0f });
	newObj->trail->SetPattern(KdTrailPolygon::Trail_Pattern::eBillboard);
	newObj->trail->SetWidth(1.4f);
	newObj->trail->SetLength(30);
	newObj->trail->ClearPoints();
	m_spTrails.push_back(newObj);

}

bool Character::EnableTrail()
{
	for (auto& trail : m_spTrails)
	{
		trail->trail->SetEnable(true);
	}
	if (m_spTrails.size() != 0)
		return	true;
	else
		return false;
}

bool Character::UnEnableTrail()
{
	for (auto& trail : m_spTrails)
	{
		trail->trail->SetEnable(false);
		trail->trail->ClearPoints();
	}
	if (m_spTrails.size() != 0)
		return	true;
	else
		return false;
}

bool Character::AddTrail()
{
	for (auto& trail : m_spTrails)
	{
		if (trail->name == LEFTUP)
		{
			const KdModelWork::Node* _pNode = m_spModelWork->FindWorkNode("LSUBP");
			if (_pNode)
			{

				auto mat = _pNode->m_worldTransform;
				mat = mat * m_mWorld;
				trail->trail->AddPoint(mat);
			}
			else
				return false;
		}
		else if (trail->name == LEFTDOWN)
		{
			const KdModelWork::Node* _pNode = m_spModelWork->FindWorkNode("LSDBP");
			if (_pNode)
			{
				auto mat = _pNode->m_worldTransform * m_mWorld;
				trail->trail->AddPoint(mat);
			}
			else
				return false;
		}
		else if (trail->name == RIGHTUP)
		{
			const KdModelWork::Node* _pNode = m_spModelWork->FindWorkNode("RSUBP");
			if (_pNode)
			{
				auto mat = _pNode->m_worldTransform * m_mWorld;
				trail->trail->AddPoint(mat);
			}
			else
				return false;
		}
		else if (trail->name == RIGHTDOWN)
		{
			const KdModelWork::Node* _pNode = m_spModelWork->FindWorkNode("RSDBP");
			if (_pNode)
			{
				auto mat = _pNode->m_worldTransform * m_mWorld;
				trail->trail->AddPoint(mat);
			}
			else
				return false;
		}
	}

	return true;
}


//ここからステートパターン関係

const Math::Vector3 Character::ActionStateBase::Direct(std::weak_ptr<Character>& owner, bool isCamera)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	const std::shared_ptr<const CameraBase> _spCamera = spOwner->m_wpCamera.lock();

	Math::Vector3 direction = spOwner->m_vMove;

	//もし何も移動キーの入力が無かったら
	if (direction.LengthSquared() == 0.0f) {
		//プレイヤーの正面のベクトルを取る
		direction = spOwner->GetMatrix().Backward();
	}
	else
	{
		if (_spCamera)
		{
			direction = direction.TransformNormal(direction, _spCamera->GetRotationYMatrix());
		}
	}

	if (isCamera == false)
	{
		direction = spOwner->GetMatrix().Backward();

	}

	direction.Normalize();

	return direction;

}

void Character::ActionStateBase::Trans(std::weak_ptr<Character>& owner, float animProgress)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	static float i = 0.0f;
	if (spOwner->m_transAC == true)
	{
		i += KdFPSController::GetInstance().GetDeltaTime();
		if (i >= 0.2f)
		{
			i = 0.0f;
			spOwner->OverTrans(m_animName, animProgress);
		}
	}
}

void Character::ActionStateBase::EffectUpdate(std::weak_ptr<Character>& owner)
{
	auto spOwner = owner.lock();

	for (auto& eff : m_spEffects)
	{
		auto spefct = eff->wpEffect.lock();
		auto mat = eff->pNodeMat;

		if (spefct == nullptr)
		{
			eff->wpEffect = KdEffekseerManager::GetInstance().SerchEffect(eff->name);
			spefct = eff->wpEffect.lock();
			eff->handle = spefct->GetHandle();
		}

		KdEffekseerManager::GetInstance().SetWorldMatrix(eff->handle, mat * spOwner->m_mWorld);
	}
}

void Character::ActionStateBase::EffectExit()
{
	for (auto& eff : m_spEffects)
	{
		KdEffekseerManager::GetInstance().StopEffect(eff->name);
		KdEffekseerManager::GetInstance().SetScale(eff->handle, 0.0f);
		if (eff->wpEffect.expired() == false)
		{
			eff->wpEffect.lock()->SetScale(0.0f);
			eff->wpEffect.lock()->SetLoop(false);
		}
	}
	m_spEffects.clear();
}


void Character::ActionStateBase::Checkkey(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	m_isMove = spOwner->IsMove();
	m_isBoost = spOwner->IsBoost();
	m_isRightAttack = spOwner->IsAttack();
	m_isFlow = spOwner->IsFlow();
	m_isGuard = spOwner->IsGuard();

}

void Character::ChangeActionState(std::shared_ptr<ActionStateBase> nextAction)
{
	if (m_nowAction)m_nowAction->Exit(m_wpThis);
	m_prvAction = m_nowAction;
	m_nowAction = nextAction;
	m_nowAction->Enter(m_wpThis);
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//待機状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionIdle::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Stand"), 6.0f);

	m_direction = Math::Vector3::Zero;

	m_speed = 0.0f * spOwner->m_speedMag;

	m_animName = "Stand";
}

void Character::ActionIdle::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("Stand\n");

	std::shared_ptr<Character> spOwner = owner.lock();


	Checkkey(owner);

	if (m_isFlow)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}

	if (m_isRightAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}

	if (m_isMove && !m_isBoost) {
		spOwner->ChangeActionState(std::make_shared<ActionMove>());
		return;
	}

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandShield>());
		return;
	}



}

void Character::ActionIdle::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());

}

void Character::ActionIdle::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//立ち上がり状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionStandUp::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandUp"), 10.0f, false);

	m_direction = Math::Vector3::Zero;

	m_speed = 0.0f * spOwner->m_speedMag;

	m_animName = "StandUp";
}

void Character::ActionStandUp::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("StandUp\n");


	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	if (m_isMove && !m_isBoost)
	{
		if (spOwner->GetPrvAction().lock()->GetName() == "BoostEnd")
		{
			spOwner->ChangeActionState(std::make_shared<ActionBoostDush>());
			return;
		}
		spOwner->ChangeActionState(std::make_shared<ActionMove>());
		return;
	}

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isRightAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}


}

void Character::ActionStandUp::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 30.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());

}

void Character::ActionStandUp::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//立ちシールド構え状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionStandShield::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandShield"), 5.0f, false);

	m_direction = Math::Vector3::Zero;

	m_speed = 0.0f * spOwner->m_speedMag;

	m_animName = "StandShield";

	spOwner->ChangeEnableLeftAttack(true);
}

void Character::ActionStandShield::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("StandShield\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	if (m_isGuard == true)
	{
		return;
	}

	if (m_isMove && !m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionMove>());
		return;
	}

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isRightAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}

	//	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}

}

void Character::ActionStandShield::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());

}

void Character::ActionStandShield::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->ChangeEnableLeftAttack(false);
}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//飛行状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionJump::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Stand"), 10.0f);

	m_speed = spOwner->m_jumpSpeed * spOwner->m_speedMag;

	Checkkey(owner);

	if (m_isMove)
	{
		m_direction += Direct(owner, true);
	}
	m_direction += Math::Vector3::Up;

	m_direction.Normalize();

	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false, false, true);

	m_animName = "Stand";

}

void Character::ActionJump::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("Fly\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isRightAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}

	if (m_isFlow)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}

	if (spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}

}

void Character::ActionJump::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());

}


void Character::ActionJump::Exit(std::weak_ptr<Character>& owner)
{
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//空中盾構え状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionJumpShield::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandShield"), 5.0f, false);

	m_direction += Math::Vector3::Up;

	m_speed = spOwner->m_jumpSpeed * spOwner->m_speedMag;

	m_animName = "StandShield";
}

void Character::ActionJumpShield::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("StandShield\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	if (m_isMove && !m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionMove>());
		return;
	}

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isRightAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}

	if (!m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}

	if (spOwner->m_spAnimator->IsAnimationEnd() && spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}

}

void Character::ActionJumpShield::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());

}

void Character::ActionJumpShield::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//歩行状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionMove::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Walk"), 6.0f);

	m_speed = spOwner->m_walkSpeed * spOwner->m_speedMag;

	m_animName = "Walk";
}

void Character::ActionMove::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("Walk\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	if (spOwner->m_spAnimator->GetProgress() == 3.0f || spOwner->m_spAnimator->GetProgress() == 6.0f)
	{
		//	KdAudioManager::Instance().Play("Asset/Sounds/Walk.wav");
	}

	if (m_isFlow) {
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}


	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isRightAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}

	if (m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionMoveShield>());
		return;
	}



	//移動中に何も入力がなければ待機に移行
	if (m_isMove == false) {
		spOwner->ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}
	else
	{

		m_direction = ActionStateBase::Direct(owner, true);


		spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);

	}



}

void Character::ActionMove::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}


void Character::ActionMove::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 歩行シールド状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionMoveShield::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("WalkShield"), 10.0f);

	m_speed = spOwner->m_walkSpeed * spOwner->m_speedMag;

	m_animName = "WalkShield";
}

void Character::ActionMoveShield::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("WalkShield\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	if (spOwner->m_spAnimator->GetProgress() == 3.0f || spOwner->m_spAnimator->GetProgress() == 6.0f)
	{
		//	KdAudioManager::Instance().Play("Asset/Sounds/Walk.wav");
	}

	if (m_isFlow) {
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}


	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isRightAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}



	//移動中に何も入力がなければ待機に移行
	if (m_isMove == false) {
		spOwner->ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}
	else
	{

		m_direction = ActionStateBase::Direct(owner, true);


		spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);

	}

}

void Character::ActionMoveShield::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}


void Character::ActionMoveShield::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

}



//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionBoost::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Boost"), 10.0f, false);

	m_direction = ActionStateBase::Direct(owner, false);

	m_speed = spOwner->m_boostSpeed * spOwner->m_speedMag;

	//ラジアルブラーセット
	KdShaderManager::Instance().m_postProcessShader.SetRadialBlurInfo(4, 0.6f, { 0.5f,0.5f }, 0.4f, 0, 0.0f);
	UINT kind = KdShaderManager::Instance().m_postProcessShader.RadialBlur;
	KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);


	//エフェクシア
	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("CBP");
		if (pNode)
		{

			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "Thruster.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			auto mat = effect->pNodeMat * spOwner->m_mWorld;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("Thruster.efkefc", mat.Translation(), 1.0f, 3.0f);
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);

			//効果音
			KdAudioManager::Instance().Play("Asset/Sounds/Thruster2.wav");
			/*	auto instance = KdAudioManager::Instance().Play3D("Asset/Sounds/Thruster2.wav", spOwner->GetPos());
				auto vec = CameraManager::Instance().ToCameraVec(mat.Translation());
				instance->SetEmitterMatrix(mat,mat.Forward());
				instance->SetVelocity(m_direction);
				instance->SetCurveDistanceScaler(1.0f);
				instance->SetInnerRadiusAngle(45);*/
		}
	}

	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("LLBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "BarniaL.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaL.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	{

		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("RLBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "BarniaR.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaR.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	m_animName = "Boost";

}

void Character::ActionBoost::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("Boost\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	if (spOwner->m_spAnimator->GetProgress() > 0.2f)
	{
		spOwner->EnableTrail();
	}

	if (spOwner->m_spAnimator->GetProgress() > 0.4f) {
		KdShaderManager::Instance().m_postProcessShader.UndoRadialBlur();
		auto kind = KdShaderManager::Instance().m_postProcessShader.Normal;
		KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	}


	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		if (m_isRightAttack) {
			spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
			return;
		}

		if (spOwner->m_isGround)
		{
			spOwner->ChangeActionState(std::make_shared<ActionBoostEnd>());
			return;
		}
		else
		{
			spOwner->ChangeActionState(std::make_shared<ActionBoostNow>());
			return;
		}

	}


	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);

	//エフェクト
	EffectUpdate(owner);

}

void Character::ActionBoost::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}


void Character::ActionBoost::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	//エフェクト
	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト中状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionBoostNow::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Hoboor"), 4.0f);

	m_speed = spOwner->m_boostEndSpeed * spOwner->m_speedMag;


	//エフェクト
	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("CBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "Thruster.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("Thruster.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 1.0f, 3.0f);
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("LLBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "BarniaL.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaL.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	{

		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("RLBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "BarniaR.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaR.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	m_animName = "Hoboor";
}

void Character::ActionBoostNow::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("BoostNow\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	if (spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoostEnd>());
		return;
	}

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isRightAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}


	if (m_isFlow)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}


	if (!m_isMove)
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}
	else
	{

		m_direction = ActionStateBase::Direct(owner, true);

		spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);
	}



	//エフェクト
	EffectUpdate(owner);

}

void Character::ActionBoostNow::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}


void Character::ActionBoostNow::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	//エフェクト
	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト終わり状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionBoostEnd::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostEnd"), 5.0f, false);

	m_speed = spOwner->m_bladeAttackSpeed * spOwner->m_speedMag;
	m_easeSpeed = spOwner->m_bladeAttackSpeed * spOwner->m_speedMag;

	//エフェクト

	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("LLBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "BarniaL.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaL.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	{

		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("RLBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "BarniaR.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaR.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}

		m_animName = "BoostEnd";
	}

}


void Character::ActionBoostEnd::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("BoostEnd\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	if (m_isMove && !m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoostDush>());
		return;
	}

	if (m_isRightAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isFlow)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}


	m_direction = ActionStateBase::Direct(owner, true);

	// イージング
	m_easeSpeed = m_speed - (m_speed * m_ease.OutSine(spOwner->m_spAnimator->GetProgress()));

	spOwner->Move(m_easeSpeed, m_direction, KdCollider::TypeGround, false);

	//エフェクト
	EffectUpdate(owner);

}

void Character::ActionBoostEnd::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());

}


void Character::ActionBoostEnd::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	//エフェクト
	EffectExit();

	spOwner->UnEnableTrail();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト移動状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionBoostDush::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostDush"), 2.0f);
	m_speed = spOwner->m_boostDushSpeed * spOwner->m_speedMag;

	spOwner->EnableTrail();


	//エフェクト
	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("LLBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "BarniaL.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaL.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	{

		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("RLBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "BarniaR.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaR.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	std::shared_ptr<Effect> effect = std::make_shared<Effect>();
	effect->name = "Spark.efkefc";
	effect->pNodeMat = Math::Matrix::Identity;
	effect->wpEffect = KdEffekseerManager::GetInstance().Play("Spark.efkefc", spOwner->m_mWorld.Translation(), 0.10f, 5.0f);
	effect->handle = effect->wpEffect.lock()->GetHandle();
	m_spEffects.push_back(effect);


	m_animName = "BoostDush";
}

void Character::ActionBoostDush::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("BoostDush\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	if (m_isFlow)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}


	if (m_isRightAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoostShield>());
		return;
	}

	//移動中に何も入力がなければ待機に移行
	if (!m_isMove) {
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}
	else
	{
		m_direction = ActionStateBase::Direct(owner, true);
	}


	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround);

	//エフェクト
	EffectUpdate(owner);

}

void Character::ActionBoostDush::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}


void Character::ActionBoostDush::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->UnEnableTrail();

	//エフェクト
	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブーストシールド状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionBoostShield::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostDushShield"), 8.0f);
	m_speed = spOwner->m_boostDushSpeed * spOwner->m_speedMag;

	spOwner->EnableTrail();


	//エフェクト
	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("LLBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "BarniaL.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaL.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	{

		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("RLBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "BarniaR.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaR.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	std::shared_ptr<Effect> effect = std::make_shared<Effect>();
	effect->name = "Spark.efkefc";
	effect->pNodeMat = Math::Matrix::Identity;
	effect->wpEffect = KdEffekseerManager::GetInstance().Play("Spark.efkefc", spOwner->m_mWorld.Translation(), 0.10f, 5.0f);
	effect->handle = effect->wpEffect.lock()->GetHandle();
	m_spEffects.push_back(effect);


	m_animName = "BoostDushShield";
}

void Character::ActionBoostShield::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("BoostDushShield\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	if (m_isFlow)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}


	if (m_isRightAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	//移動中に何も入力がなければ待機に移行
	if (!m_isMove) {
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}
	else
	{
		m_direction = ActionStateBase::Direct(owner, true);
	}


	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround);

	//エフェクト
	EffectUpdate(owner);


}

void Character::ActionBoostShield::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}


void Character::ActionBoostShield::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->UnEnableTrail();

	//エフェクト
	EffectExit();
}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//攻撃状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionRightAttack::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("RightBladeAttackBef"), 10.0f, false);

	m_direction = ActionStateBase::Direct(owner, false);

	m_speed = spOwner->m_bladeAttackSpeed * spOwner->m_speedMag;


	KdAudioManager::Instance().Play("Asset/Sounds/Thruster2.wav");

	//エフェクト
	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("CBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "Thruster.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("Thruster.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	m_animName = "RightBladeAttackBef";
}

void Character::ActionRightAttack::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("Attack\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	if (spOwner->m_spAnimator->GetProgress() > 0.6f)
	{
		if (m_isBoost)
		{
			spOwner->ChangeActionState(std::make_shared<ActionBoost>());
			return;
		}
	}

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttackAf>());
		return;
	}

	//敵が一定範囲内なら敵のほうに向いて敵に
	std::shared_ptr<KdGameObject> target = spOwner->m_wpRockTarget.lock();
	if (target)
	{
		m_direction = target->GetPos() - spOwner->GetPos();
		m_direction.Normalize();
	}

	//owner.Move(m_speed, m_direction, KdCollider::TypeDamage, true);
	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround);

	if (spOwner->m_spAnimator->GetProgress() > 0.2f)
	{
		spOwner->EnableTrail();
	}

	//エフェクト
	EffectUpdate(owner);

}

void Character::ActionRightAttack::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}


void Character::ActionRightAttack::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	//エフェクト
	EffectExit();

}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//攻撃後状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionRightAttackAf::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("RightBladeAttack"), 10.0f, false);

	m_speed = spOwner->m_bladeAttackSpeed * spOwner->m_speedMag;

	m_direction = ActionStateBase::Direct(owner, false);

	spOwner->Move(m_speed, m_direction, KdCollider::TypeDamage, true);

	//エフェクト
	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("CBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "Thruster.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("Thruster.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	m_animName = "RightBladeAttack";
}

void Character::ActionRightAttackAf::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("AttackAf\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	//移動
	if (spOwner->m_spAnimator->GetProgress() < 0.4f)
	{
		spOwner->Move(m_speed, m_direction, KdCollider::TypeGround);
	}

	if (spOwner->m_spAnimator->GetProgress() > 0.8f)
	{
		spOwner->UnEnableTrail();
	}

	//エフェクト
	EffectUpdate(owner);


	//もしアニメーションが終わってないなら強制的に終了
	if (spOwner->m_spAnimator->IsAnimationEnd() == false)
	{
		return;
	}
	Checkkey(owner);

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isMove)
	{
		spOwner->ChangeActionState(std::make_shared<ActionMove>());
		return;
	}


	spOwner->ChangeActionState(std::make_shared<ActionIdle>());
	return;

}

void Character::ActionRightAttackAf::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());

}


void Character::ActionRightAttackAf::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	//エフェクト
	EffectExit();

	spOwner->ChangeEnableRightAttack(false);

}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ヒット状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionHited::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Hited"), 5.0f, false);

	m_speed = spOwner->m_hitedSpeed;

	m_direction = spOwner->m_mWorld.Forward();

	KdAudioManager::Instance().Play("Asset/Sounds/Noize-1.wav");

	m_animName = "Hited";
}

void Character::ActionHited::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround);

	//グリッチ表現
	auto time = KdFPSController::GetInstance().GetFPS();

	UINT kind = KdShaderManager::Instance().m_postProcessShader.Glitch;
	KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	KdShaderManager::Instance().m_postProcessShader.
		SetGlitch({ 10,10 }, time, 5.0f, 0.8f, 0, 1, { 0.5f,0.5f });

	if (spOwner->m_spAnimator->IsAnimationEnd() == false) { return; }

	Checkkey(owner);

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}
	if (m_isRightAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}

	if (m_isMove)
	{
		spOwner->ChangeActionState(std::make_shared<ActionMove>());
		return;
	}

	spOwner->ChangeActionState(std::make_shared<ActionIdle>());
	return;


}

void Character::ActionHited::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}


void Character::ActionHited::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	UINT kind = KdShaderManager::Instance().m_postProcessShader.Normal;
	KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	KdShaderManager::Instance().m_postProcessShader.UndoGlitch();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//破壊状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionDestroyed::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Destroyed"), 10.0f, false);

	m_speed = spOwner->m_stopSpeed;

	m_direction = spOwner->m_mWorld.Forward();

}

void Character::ActionDestroyed::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	//グリッチ表現
	auto time = KdFPSController::GetInstance().GetFPS();

	UINT kind = KdShaderManager::Instance().m_postProcessShader.Glitch;
	KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	KdShaderManager::Instance().m_postProcessShader.
		SetGlitch({ 30,30 }, time, 5.0f, 0.8f, 0, 1, { 0.5f,0.5f });

	if (spOwner->m_spAnimator->IsAnimationEnd() == false) { return; }


	spOwner->ChangeActionState(std::make_shared<ActionIdle>());
	return;
}

void Character::ActionDestroyed::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	if (spOwner->m_spAnimator->GetProgress() < 0.4f)
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
	else
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);
	}
}


void Character::ActionDestroyed::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	UINT kind = KdShaderManager::Instance().m_postProcessShader.Normal;
	KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	KdShaderManager::Instance().m_postProcessShader.UndoGlitch();
}