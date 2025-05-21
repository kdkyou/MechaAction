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

	m_Gravity = 0;
	SetPos({ 0, 5.0f, 0 });


	//初期状態を「待機状態」へ設定
	ChangeActionState(std::make_shared<ActionIdle>());
}

void Character::Update()
{
	// 重力の更新()
	m_Gravity += 0.01f;
	m_mWorld._42 -= m_Gravity;

	//各種「状態に応じた」更新処理を実行する
	if (m_nowAction)
	{
		m_nowAction->Update(*this);
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

void Character::DrawBright()
{
	if (!m_spModel) return;

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

bool Character::Move(float speed, const Math::Vector3& dir, const KdCollider::Type type, bool ray)
{
	auto pos = m_mWorld.Translation();

	auto deltaTime = KdFPSController::GetInstance().GetDeltaTime();

	Math::Vector3 move = Math::Vector3::Zero;

	auto deltaSpeed = speed * deltaTime;

	bool isHit = RayCast(pos, dir, deltaSpeed, type, move);
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

	UpdateRotate(dir);

	Math::Matrix _rotation = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_worldRot.y));
	m_mWorld = m_scale * _rotation * Math::Matrix::CreateTranslation(pos);


	return false;
}

void Character::UpdateRotate(const Math::Vector3& srcMoveVec)
{
	// 何も入力が無い場合は処理しない
	if (srcMoveVec.LengthSquared() == 0.0f) { return; }

	// キャラの正面方向のベクトル
	Math::Vector3 _nowDir = GetMatrix().Backward();

	// 移動方向のベクトル
	Math::Vector3 _targetDir = srcMoveVec;

	_nowDir.Normalize();
	_targetDir.Normalize();

	float _nowAng = atan2(_nowDir.x, _nowDir.z);
	_nowAng = DirectX::XMConvertToDegrees(_nowAng);

	float _targetAng = atan2(_targetDir.x, _targetDir.z);
	_targetAng = DirectX::XMConvertToDegrees(_targetAng);

	// 角度の差分を求める
	float _betweenAng = _targetAng - _nowAng;
	if (_betweenAng > 180)
	{
		_betweenAng -= 360;
	}
	else if (_betweenAng < -180)
	{
		_betweenAng += 360;
	}

	float rotateAng = std::clamp(_betweenAng, -m_clampSize, m_clampSize);
	m_worldRot.y += rotateAng;
}

void Character::UpdateCollision()
{
	// 地面判定

	// ①当たり判定(レイ判定)用の情報作成
	KdCollider::RayInfo rayInfo;
	// レイの発射位置を設定
	rayInfo.m_pos = GetPos();
	// 少し高い所から飛ばす(段差の許容範囲)
	rayInfo.m_pos.y += 0.2f;

	// レイの発射方向を設定
	rayInfo.m_dir = Math::Vector3::Down;

	// レイの長さ
	rayInfo.m_range = m_Gravity + 0.2f;

	// 当たり判定をしたいタイプを設定
	rayInfo.m_type = KdCollider::TypeGround;

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
			bool hit = false;
			for (auto& ret : retRayList)
			{
				// レイを遮断しオーバーした長さが
				// 一番長いものを探す
				if (maxOverLap < ret.m_overlapDistance)
				{
					maxOverLap = ret.m_overlapDistance;
					hitPos = ret.m_hitPos;
					hit = true;
				}
			}
			// 何かしらの上に乗ってる
			if (hit)
			{
				SetPos(hitPos);
				m_Gravity = 0;

				m_isGround = true;
			}
			else
			{

				m_isGround = false;
			}
		}
	}

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
}

bool Character::RayCast(const Math::Vector3& startPos, const Math::Vector3& vec, const float length, const KdCollider::Type& type, Math::Vector3& resultPos)
{
	KdCollider::RayInfo rayInfo;

	rayInfo.m_pos = startPos;		// レイの発射位置を設定
	rayInfo.m_dir = vec;				// レイの発射方向を設定
	rayInfo.m_range = length;		// レイの長さ

	// 当たり判定をしたいタイプを設定
	rayInfo.m_type = type;

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

		}
	}

	return hit;
}


//ここからステートパターン関係

void Character::ActionStateBase::Direct(Character& owner)
{
	m_direction = owner.m_vMove;

	const std::shared_ptr<const CameraBase> _spCamera = owner.m_wpCamera.lock();
	if (_spCamera)
	{
		m_direction = m_direction.TransformNormal(m_direction, _spCamera->GetRotationYMatrix());
	}


	m_direction.Normalize();

}

void Character::ActionStateBase::Checkkey(Character& owner)
{
	m_isMove = owner.IsMove();
	m_isBoost = owner.IsBoost();
	m_isLeftAttack = owner.IsLeftAttack();
	m_isFlow = owner.IsFlow();

}


void Character::ChangeActionState(std::shared_ptr<ActionStateBase> nextAction)
{
	if (m_nowAction)m_nowAction->Exit(*this);
	m_nowAction = nextAction;
	m_nowAction->Enter(*this);
}

void Character::ActionIdle::Enter(Character& owner)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("Stand"), 2.0f);

	m_speed = 0.0f;
}

void Character::ActionIdle::Update(Character& owner)
{

	Checkkey(owner);

	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {

		if (owner.m_isGround)
		{
			owner.ChangeActionState(std::make_shared<ActionJump>());
			return;
		}
	}

	if (m_isLeftAttack)
	{
		owner.ChangeActionState(std::make_shared<ActionLeftAttack>());
		return;
	}

	if (m_isMove && !m_isBoost) {
		owner.ChangeActionState(std::make_shared<ActionMove>());
		return;
	}

	if (m_isBoost)
	{
		owner.ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}
}

void Character::ActionIdle::Exit(Character& owner)
{
	owner.m_spAnimator->IsAnimationEnd();
}

void Character::ActionJump::Enter(Character& owner)
{
	auto delta = KdFPSController::GetInstance().GetDeltaTime();

	owner.m_Gravity += -0.5f;

	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("Stand"), 8.0f);
}

void Character::ActionJump::Update(Character& owner)
{
	if (owner.m_isGround)
	{
		owner.ChangeActionState(std::make_shared<ActionBoostEnd>());
		return;
	}
	else
	{
		owner.ChangeActionState(std::make_shared<ActionBoostNow>());
		return;
	}
}

void Character::ActionJump::Exit(Character& owner)
{
}

void Character::ActionMove::Enter(Character& owner)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("Walk"), 4.0f);

	m_speed = 5.0f;

}

void Character::ActionMove::Update(Character& owner)
{

	Checkkey(owner);

	if (m_isFlow) {
		owner.ChangeActionState(std::make_shared<ActionJump>());
		return;
	}


	if (m_isBoost)
	{
		owner.ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isLeftAttack)
	{
		owner.ChangeActionState(std::make_shared<ActionLeftAttack>());
		return;
	}


	//移動中に何も入力がなければ待機に移行
	if (m_isMove == false) {
		owner.ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}
	else
	{

		m_direction = owner.m_vMove;

		const std::shared_ptr<const CameraBase> _spCamera = owner.m_wpCamera.lock();
		if (_spCamera)
		{
			m_direction = m_direction.TransformNormal(m_direction, _spCamera->GetRotationYMatrix());
		}


		m_direction.Normalize();


		owner.Move(m_speed, m_direction, KdCollider::TypeGround, false);

	}

}

void Character::ActionMove::Exit(Character& owner)
{

}

void Character::ActionBoostNow::Enter(Character& owner)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("Hoboor"), 4.0f);

	m_speed = 5.0f;

}

void Character::ActionBoostNow::Update(Character& owner)
{
	Checkkey(owner);

	if (owner.m_isGround)
	{
		owner.ChangeActionState(std::make_shared<ActionBoostEnd>());
		return;
	}

	if (m_isBoost)
	{
		owner.ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isLeftAttack)
	{
		owner.ChangeActionState(std::make_shared<ActionLeftAttack>());
		return;
	}


	if (m_isFlow)
	{
		owner.ChangeActionState(std::make_shared<ActionJump>());
		return;
	}


	if (m_isMove)
	{

		m_direction = owner.m_vMove;
		const std::shared_ptr<const CameraBase> _spCamera = owner.m_wpCamera.lock();

		if (_spCamera)
		{
			//もし何も移動キーの入力が無かったら
			if (m_direction.LengthSquared() == 0) {
				//プレイヤーの正面のベクトルを取る
				m_direction = owner.GetMatrix().Backward();
			}
			else
			{
				m_direction = m_direction.TransformNormal(m_direction, _spCamera->GetRotationYMatrix());
			}
		}


		owner.Move(m_speed, m_direction, KdCollider::TypeGround, false);
	}



}

void Character::ActionBoostNow::Exit(Character& owner)
{
}

void Character::ActionBoostEnd::Enter(Character& owner)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("BoostEnd"), 9.0f, false);
}

void Character::ActionBoostEnd::Update(Character& owner)
{
	Checkkey(owner);

	if (m_isMove)
	{
		owner.ChangeActionState(std::make_shared<ActionBoostDush>());
		return;
	}

	if (m_isLeftAttack)
	{
		owner.ChangeActionState(std::make_shared<ActionLeftAttack>());
		return;
	}

	if (owner.m_spAnimator->IsAnimationEnd())
	{
		owner.ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}

}

void Character::ActionBoostEnd::Exit(Character& owner)
{
}

void Character::ActionLeftAttack::Enter(Character& owner)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("LeftBladeAttackBef"), 10.0f, false);
	m_speed = 50.0f;
}

void Character::ActionLeftAttack::Update(Character& owner)
{
	Checkkey(owner);

	if (m_isBoost)
	{
		owner.ChangeActionState(std::make_shared<ActionBoost>());
	}

	if (owner.m_spAnimator->IsAnimationEnd())
	{
		owner.ChangeActionState(std::make_shared<ActionLeftAttackAf>());
		return;
	}

	//座標更新

	m_direction = owner.GetMatrix().Backward();

	//敵が一定範囲内なら敵のほうに向いて敵に
	//if()


	m_direction.Normalize();

	//owner.Move(m_speed, m_direction, KdCollider::TypeDamage, true);
	owner.Move(m_speed, m_direction, KdCollider::TypeGround, false);

}

void Character::ActionLeftAttack::Exit(Character& owner)
{
}

void Character::ActionStandUp::Enter(Character& owner)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("StandUp"), 2.0f, false);
}

void Character::ActionStandUp::Update(Character& owner)
{
	Checkkey(owner);

	if (m_isMove && !m_isBoost)
	{
		owner.ChangeActionState(std::make_shared<ActionMove>());
		return;
	}

	if (m_isBoost)
	{
		owner.ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	if (m_isLeftAttack)
	{
		owner.ChangeActionState(std::make_shared<ActionLeftAttack>());
		return;
	}

	if (owner.m_spAnimator->IsAnimationEnd())
	{
		owner.ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}
}

void Character::ActionStandUp::Exit(Character& owner)
{
}

void Character::ActionBoostDush::Enter(Character& owner)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("BoostDush"), 10.0f);
	m_speed = 20.0f;
}

void Character::ActionBoostDush::Update(Character& owner)
{

	Checkkey(owner);

	m_direction = owner.m_vMove;

	if(m_isFlow)
	{
		owner.ChangeActionState(std::make_shared<ActionJump>());
		return;
	}


	if (m_isLeftAttack)
	{
		owner.ChangeActionState(std::make_shared<ActionLeftAttack>());
		return;
	}

	if (m_isBoost)
	{
		owner.ChangeActionState(std::make_shared<ActionBoost>());
		return;
	}

	//移動中に何も入力がなければ待機に移行
	if (m_isMove == false) {
		owner.ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}
	else
	{

		const std::shared_ptr<const CameraBase> _spCamera = owner.m_wpCamera.lock();
		if (_spCamera)
		{
			m_direction = m_direction.TransformNormal(m_direction, _spCamera->GetRotationYMatrix());
		}
		m_direction.Normalize();

	}

	owner.Move(m_speed, m_direction, KdCollider::TypeGround, false);
}

void Character::ActionBoostDush::Exit(Character& owner)
{
}

void Character::ActionBoost::Enter(Character& owner)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("Boost"), 20.0f, false);
	const std::shared_ptr<const CameraBase> _spCamera = owner.m_wpCamera.lock();
	m_direction = owner.m_vMove;
	if (_spCamera)
	{
		//もし何も移動キーの入力が無かったら
		if (m_direction.LengthSquared() == 0) {
			//プレイヤーの正面のベクトルを取る
			m_direction = owner.GetMatrix().Backward();
		}
		else
		{
			m_direction = m_direction.TransformNormal(m_direction, _spCamera->GetRotationYMatrix());
		}
	}

	m_direction.Normalize();

	m_speed = 40.0f;
}

void Character::ActionBoost::Update(Character& owner)
{
	Checkkey(owner);

	//owner.Move(m_speed, m_direction, KdCollider::TypeDamage, true);

	if (owner.m_spAnimator->IsAnimationEnd())
	{
		if (m_isLeftAttack) {
			owner.ChangeActionState(std::make_shared<ActionLeftAttack>());
			return;
		}

		if (owner.m_isGround)
		{
			owner.ChangeActionState(std::make_shared<ActionBoostEnd>());
			return;
		}
		else
		{
			if (m_isMove)
			{
				owner.ChangeActionState(std::make_shared<ActionBoostNow>());
				return;
			}

		}
	}

	owner.Move(m_speed, m_direction, KdCollider::TypeGround, false);

}

void Character::ActionBoost::Exit(Character& owner)
{

}


void Character::ActionLeftAttackAf::Enter(Character& owner)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("LeftBladeAttack"), 10.0f, false);

	m_speed = 50.0f;

	m_direction = owner.GetMatrix().Backward();

}

void Character::ActionLeftAttackAf::Update(Character& owner)
{

	if (owner.m_spAnimator->GetAdvanceTime() < 0.6f)
	{
		//owner.Move(m_speed,m_direction,KdCollider::TypeDamage,true);
		owner.Move(m_speed, m_direction, KdCollider::TypeGround, false);
	}

	if (owner.m_spAnimator->IsAnimationEnd() == true)
	{
		Checkkey(owner);

		if (m_isBoost)
		{
			owner.ChangeActionState(std::make_shared<ActionBoost>());
			return;
		}

		owner.ChangeActionState(std::make_shared<ActionIdle>());
		return;

	}

}

void Character::ActionLeftAttackAf::Exit(Character& owner)
{
}
