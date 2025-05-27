#include "Character.h"

#include "../../main.h"
#include "../Camera/CameraBase.h"

#include"../../Scene/SceneManager.h"

void Character::Init()
{
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Grint/grint.gltf");

		// 初期のアニメーションをセットする
		m_spAnimator = std::make_shared<KdAnimator>();
		m_spAnimator->SetAnimation(m_spModel->GetData()->GetAnimation("Stand"), 1.0f, true, true);
	}

	m_gravity = 0;
	SetPos({ 0, 2.0f, 0 });

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
	m_pCollider = std::make_unique<KdCollider>();

	InitTrail();
	UnEnableTrail();

	//初期状態を「待機状態」へ設定
	ChangeActionState(std::make_shared<ActionIdle>());
}

void Character::Update()
{
	// 重力の更新()
	m_gravity += m_gravityPow * KdFPSController::GetInstance().GetDeltaTime();
	Move(m_gravity, Math::Vector3::Down, KdCollider::TypeGround, false);

	auto spThis = m_wpThis.lock();

	color = { 0,1,0,1 };

	AddTrail();

	if (spThis)
	{
		//各種「状態に応じた」更新処理を実行する
		if (m_nowAction)
		{
			m_nowAction->Update(m_wpThis);
		}
	}


	//1m_pCollider->RegisterCollisionShape(KdCollider::TypeBump)

	// キャラクターの座標が確定してからコリジョンによる位置補正を行う
	UpdateCollision();

}

void Character::PostUpdate()
{
	// アニメーションの更新
	m_spAnimator->AdvanceTime(m_spModel->WorkNodes());
	m_spModel->CalcNodeMatrices();
}

void Character::GenerateDepthMapFromLight()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void Character::DrawLit()
{
	if (!m_spModel) return;

	Math::Vector3 emis = { 5,0,0 };

	Math::Color colr = { 1,1,1 };

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld, colr, emis);
}

void Character::DrawUnLit()
{
	if (m_spTrails.size() == 0) { return; }

//		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);
	for (auto& trail : m_spTrails)
	{
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(*trail->trail);
	}
//		KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);

	//	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

const std::weak_ptr<KdModelWork> Character::GetModelWork()const
{
	std::shared_ptr<KdModelWork> spmodel;
	if (m_spModel == nullptr)return spmodel;

	return m_spModel;
}

const bool Character::IsMove()
{
	bool move = false;
	m_vMove = Math::Vector3::Zero;

	if (GetAsyncKeyState('W') & 0x8000) {
		m_vMove.z += 1.f;
		move = true;
	}
	if (GetAsyncKeyState('A') & 0x8000) {
		m_vMove.x -= 1.f;
		move = true;
	}
	if (GetAsyncKeyState('S') & 0x8000) {
		m_vMove.z -= 1.f;
		move = true;
	}
	if (GetAsyncKeyState('D') & 0x8000) {
		m_vMove.x += 1.f;
		move = true;
	}

	if (move) { return true; }
	else { return false; }
}

const bool Character::IsBoost()
{
	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
	{
		return true;
	}

	return false;
}

const bool Character::IsLeftAttack()
{
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
	{
		return true;
	}

	return false;
}

const bool Character::IsFlow()
{
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
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
		pos = move;
	}
	else
	{
		move = dir * deltaSpeed;
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
	spherInfo.m_sphere.Center = GetPos() + Math::Vector3(0, 0.5f, 0);
	spherInfo.m_sphere.Radius = 0.5f;
	spherInfo.m_type = KdCollider::TypeBump;

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

	spherInfo.m_sphere.Center = GetPos() + Math::Vector3(0, 0.5f, 0);
	spherInfo.m_sphere.Radius = 5.0f;
	spherInfo.m_type = KdCollider::TypeDamage;

	//
	m_wpRockTarget.reset();

	// ②HIT対象オブジェクトに総当たり
	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			if (spGameObj->Intersects(spherInfo, nullptr))
			{
				m_wpRockTarget = spGameObj;
			}
		}
	}


	DirectX::BoundingOrientedBox box;
	box.Center = m_mWorld.Translation() + Math::Vector3(0.0f, 5.0f, 0.0f);
	box.Extents = { 3.0f,10.0f,3.0f };
	UINT type = KdCollider::TypeDamage;
	KdCollider::BoxInfo boxInfo(type, box);

	auto mat = Math::Matrix::CreateTranslation(boxInfo.m_Obox.Center);


	for (std::weak_ptr<KdGameObject> wpGameObj : m_wpHitObjectList)
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			if (spGameObj->Intersects(boxInfo, nullptr))
			{
				color = { 0,0,1,1 };
			}
		}
	}

	m_pDebugWire->AddDebugBox(mat, box.Extents, {}, true, color);

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
				if (hit == true)
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
	 newObj->trail->SetWidth(0.4f);
	 newObj->trail->ClearPoints();
	 m_spTrails.push_back(newObj);

	 name = RIGHTUP;
	 newObj = std::make_shared<TrailParam>();
	newObj->name = name;
	 newObj->trail = std::make_shared<KdTrailPolygon>();
	 newObj->trail->SetMaterial(KdAssets::Instance().m_textures.LoadData("Asset/Textures/GameObject/Trail.png"));
	 //	trail->SetColor(Math::Color{ 3.0f,3.0f,3.0f });
	 newObj->trail->SetPattern(KdTrailPolygon::Trail_Pattern::eBillboard);
	 newObj->trail->SetWidth(0.4f);
	 newObj->trail->ClearPoints();
	 m_spTrails.push_back(newObj);

	 name = LEFTDOWN;
	 newObj = std::make_shared<TrailParam>();
	 newObj->name = name;
	 newObj->trail = std::make_shared<KdTrailPolygon>();
	 newObj->trail->SetMaterial(KdAssets::Instance().m_textures.LoadData("Asset/Textures/GameObject/Trail.png"));
	 //	trail->SetColor(Math::Color{ 3.0f,3.0f,3.0f });
	 newObj->trail->SetPattern(KdTrailPolygon::Trail_Pattern::eBillboard);
	 newObj->trail->SetWidth(0.4f);
	 newObj->trail->ClearPoints();
	 m_spTrails.push_back(newObj);

	 name = RIGHTDOWN;
	 newObj = std::make_shared<TrailParam>();
	 newObj->name = name;
	 newObj->trail = std::make_shared<KdTrailPolygon>();
	 newObj->trail->SetMaterial(KdAssets::Instance().m_textures.LoadData("Asset/Textures/GameObject/Trail.png"));
	 //	trail->SetColor(Math::Color{ 3.0f,3.0f,3.0f });
	 newObj->trail->SetPattern(KdTrailPolygon::Trail_Pattern::eBillboard);
	 newObj->trail->SetWidth(0.4f);
	 newObj->trail->ClearPoints();
	 m_spTrails.push_back(newObj);

}

bool Character::EnableTrail()
{
	for (auto& trail:m_spTrails)
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
			const KdModelWork::Node* _pNode = m_spModel->FindWorkNode("LSUBP");
			if (_pNode)
			{
				auto mat = _pNode->m_worldTransform * m_mWorld;
				trail->trail->AddPoint(mat);
			}
			else
				return false;
		}
		else if (trail->name == LEFTDOWN)
		{
			const KdModelWork::Node* _pNode = m_spModel->FindWorkNode("LSDBP");
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
			const KdModelWork::Node* _pNode = m_spModel->FindWorkNode("RSUBP");
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
			const KdModelWork::Node* _pNode = m_spModel->FindWorkNode("RSDBP");
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

	/*if (isCamera==false)
	{
		direction = spOwner->GetMatrix().Backward();

	}*/

	direction.Normalize();

	return direction;

}


void Character::ActionStateBase::Checkkey(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	m_isMove = spOwner->IsMove();
	m_isBoost = spOwner->IsBoost();
	m_isLeftAttack = spOwner->IsLeftAttack();
	m_isFlow = spOwner->IsFlow();

}

void Character::ChangeActionState(std::shared_ptr<ActionStateBase> nextAction)
{
	if (m_nowAction)m_nowAction->Exit(m_wpThis);
	m_nowAction = nextAction;
	m_nowAction->Enter(m_wpThis);
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//待機状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionIdle::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("Stand"), 2.0f);

	m_direction = Math::Vector3::Zero;

	m_speed = 0.0f;
}

void Character::ActionIdle::Update(std::weak_ptr<Character>& owner)
{

	std::shared_ptr<Character> spOwner = owner.lock();


	Checkkey(owner);

	if (m_isFlow)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}

	if (m_isLeftAttack)
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

	if (GetAsyncKeyState('T') & 0x8000)
	{
		spOwner->ChangeActionState(std::make_shared<ActionHited>());
		return;

	}

	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);
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

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("StandUp"), 2.0f, false);

	m_direction = Math::Vector3::Zero;

	m_speed = 0.0f;
}

void Character::ActionStandUp::Update(std::weak_ptr<Character>& owner)
{
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

	if (m_isLeftAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}

	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);

}

void Character::ActionStandUp::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//飛行状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionJump::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->
		m_spAnimator->SetAnimation(spOwner->
			m_spModel->GetData()->GetAnimation("Stand"), 1.0f);

	m_direction = Math::Vector3::Up;
	m_speed = spOwner->m_jumpSpeed;

	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false, false,true);
}

void Character::ActionJump::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false, false, true);

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isLeftAttack)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}
	
	if (spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoostEnd>());
		return;
	}
	

	if (m_isFlow)
	{
		return;
	}

}

void Character::ActionJump::Exit(std::weak_ptr<Character>& owner)
{
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//歩行状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionMove::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("Walk"), 4.0f);

	m_speed = spOwner->m_walkSpeed;

}

void Character::ActionMove::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	if (m_isFlow) {
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}


	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isLeftAttack)
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

void Character::ActionMove::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionBoost::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("Boost"), 20.0f, false);

	m_direction = ActionStateBase::Direct(owner, false);

	m_speed = spOwner->m_boostSpeed;

	KdShaderManager::Instance().m_postProcessShader.SetRadialBlurInfo(4, 0.6f, { 0.5f,0.5f }, 0.45f, 0, 0.01f);
	UINT kind = KdShaderManager::Instance().m_postProcessShader.RadialBlur;
	KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
}

void Character::ActionBoost::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	//owner.Move(m_speed, m_direction, KdCollider::TypeDamage, true);

	if (spOwner->m_spAnimator->GetProgress() > 0.4f) {
		KdShaderManager::Instance().m_postProcessShader.UndoRadialBlur();
		auto kind = KdShaderManager::Instance().m_postProcessShader.Normal;
		KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	}

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		if (m_isLeftAttack) {
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

}

void Character::ActionBoost::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト中状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionBoostNow::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("Hoboor"), 4.0f);

	m_speed = spOwner->m_boostEndSpeed;

}

void Character::ActionBoostNow::Update(std::weak_ptr<Character>& owner)
{
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

	if (m_isLeftAttack)
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
		spOwner->ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}
	else
	{

		m_direction = ActionStateBase::Direct(owner, true);

		spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);
	}

}

void Character::ActionBoostNow::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト終わり状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionBoostEnd::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("BoostEnd"), 9.0f, false);

	m_speed = spOwner->m_bladeAttackSpeed;

}

void Character::ActionBoostEnd::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	if (m_isMove)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoostDush>());
		return;
	}

	if (m_isLeftAttack)
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

	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);

}

void Character::ActionBoostEnd::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト移動状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionBoostDush::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("BoostDush"), 10.0f);
	m_speed = spOwner->m_boostDushSpeed;

	spOwner->EnableTrail();
}

void Character::ActionBoostDush::Update(std::weak_ptr<Character>& owner)
{

	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	if (m_isFlow)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}


	if (m_isLeftAttack)
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
		m_direction = ActionStateBase::Direct(owner, false);

	}

	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);
}

void Character::ActionBoostDush::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->UnEnableTrail();
}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//攻撃状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionRightAttack::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("RightBladeAttackBef"), 10.0f, false);

	m_direction = ActionStateBase::Direct(owner, false);

	m_speed = spOwner->m_bladeAttackSpeed;
}

void Character::ActionRightAttack::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	if (m_isBoost)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoost>());
	}

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttackAf>());
		return;
	}



	//敵が一定範囲内なら敵のほうに向いて敵に
	auto target = spOwner->m_wpRockTarget.lock();
	if (target)
	{
		m_direction = target->GetPos() - spOwner->GetPos();
		m_direction.Normalize();
	}


	//owner.Move(m_speed, m_direction, KdCollider::TypeDamage, true);
	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);

}

void Character::ActionRightAttack::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//攻撃後状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionRightAttackAf::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("RightBladeAttack"), 10.0f, false);

	m_speed = spOwner->m_bladeAttackSpeed;

	m_direction = ActionStateBase::Direct(owner, false);

}

void Character::ActionRightAttackAf::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	if (spOwner->m_spAnimator->GetAdvanceTime() < 0.6f)
	{
		//owner.Move(m_speed,m_direction,KdCollider::TypeDamage,true);
		spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);
	}

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

void Character::ActionRightAttackAf::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ヒット状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionHited::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("Hited"), 3.0f, false);

	m_speed = spOwner->m_hitedSpeed;

	m_direction = spOwner->m_mWorld.Forward();

}

void Character::ActionHited::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false, false);

	//グリッチ表現
	auto time = KdFPSController::GetInstance().GetTime();

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
	if (m_isLeftAttack)
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
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("Destroyed"), 10.0f, false);

	m_speed = spOwner->m_stopSpeed;

	m_direction = spOwner->m_mWorld.Forward();

}

void Character::ActionDestroyed::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	//グリッチ表現
	auto time = KdFPSController::GetInstance().GetTime();

	UINT kind = KdShaderManager::Instance().m_postProcessShader.Glitch;
	KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	KdShaderManager::Instance().m_postProcessShader.
		SetGlitch({ 30,30 }, time, 5.0f, 0.8f, 1, 1, { 0.5f,0.5f });

if (spOwner->m_spAnimator->IsAnimationEnd() == false) { return; }

	
	spOwner->ChangeActionState(std::make_shared<ActionIdle>());
	return;


}

void Character::ActionDestroyed::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	UINT kind = KdShaderManager::Instance().m_postProcessShader.Normal;
	KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	KdShaderManager::Instance().m_postProcessShader.UndoGlitch();
}