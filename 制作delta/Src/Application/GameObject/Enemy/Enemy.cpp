#include"Enemy.h"


#include"../../main.h"

#include "../../Scene/SceneManager.h"

#include"../Character/CharacterBase.h"

void Enemy::Init()
{
	if (!m_spModelWork)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData("Asset/Models/Another/Another.gltf");
		// 初期のアニメーションをセットする
		m_spAnimator = std::make_shared<KdAnimator>();
		m_spAnimator->SetAnimation(m_spModelWork->GetData()->GetAnimation("StandUp"), 00.0f, false);
	}

	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();

		DirectX::BoundingOrientedBox box;

		box.Center = m_mWorld.Translation();
		box.Extents = { 3,5,3 };

	//	m_pCollider->RegisterCollisionShape("Enemy", box, KdCollider::TypeDamage);
		m_pCollider->RegisterCollisionShape("Enemy", m_spModelWork, KdCollider::TypeDamage);
	}

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	SetPos({ 0.0f,0.0f,10.0f });

	//初期状態を「待機状態」へ設定
	ChangeActionState(std::make_shared<Start>());

	m_dist = { 10.0f,40.0f };
}

void Enemy::Update()
{
	//各種「状態に応じた」更新処理を実行する
	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();

		std::shared_ptr<Enemy> spThis = m_wpThis.lock();

		if (spThis == nullptr)
		{
			return;
		}
		m_nowAction->Update(m_wpThis, spTarget);

	}
}

void Enemy::PostUpdate()
{
	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();


		std::shared_ptr<Enemy> spThis = m_wpThis.lock();

		if (spThis == nullptr)
		{
			return;
		}
		m_nowAction->PostUpdate(m_wpThis, spTarget);
	}

	m_pDebugWire->AddDebugBox(m_mWorld, { 3,5,3 }, {}, true, { 1,0,0,1 });


	auto translation = m_mWorld.Translation();
	Application::Instance().m_log.AddLog("pos x:%.2f,y:%.2f,z:%.2f\n", translation.x, translation.y, translation.z);

}


void Enemy::DrawLit()
{
	if (!m_spModelWork) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld);
}

bool Enemy::Search()
{
	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = m_mWorld.Translation() + m_currection;
	sphere.m_sphere.Radius = m_radius;
	sphere.m_type = KdCollider::TypeDamage;
	
	std::list< KdCollider::CollisionResult> retList;
	std::list<std::shared_ptr<KdGameObject>> objList;

	for (auto& obj : SceneManager::Instance().GetPlayerList())
	{

		if (obj->Intersects(sphere, &retList))
		{
			objList.push_back(obj);
		}
	}

	if (retList.empty() == true) { return false; }

	Math::Vector3 hitPos = {};
	float overRap = 0.0f;
	bool isHit = false;

	std::shared_ptr<KdGameObject> obj;
	auto it = objList.begin();

	for (auto& ret : retList)
	{
		if (overRap < ret.m_overlapDistance)
		{
			hitPos = ret.m_hitPos;
			overRap = ret.m_overlapDistance;
			isHit = true;
			obj = *it;
		}
		// 増加
		it++;
	}

	if (isHit)
	{
		// 視界内にいるかどうかの判定
		 bool flg = SearchDetect(hitPos, m_mWorld, m_viewAngle);

		 if (flg)
		 {
			 m_wpTarget = obj;
		 }
		 else
		 {
			 m_wpTarget.reset();
		 }
	}

	return true;
}

void Enemy::Editor_ImGui()
{

	ImGui::DragFloat2("distance", &m_dist.x, 0.1f);
}


void Enemy::ChangeActionState(std::shared_ptr<ActionStateBase> nextAction)
{
	if (nextAction == nullptr) { return; }

	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();

	if (spTarget == nullptr) { return; }

	if (m_nowAction) {
		m_nowAction->Exit(m_wpThis, spTarget);
	}

	m_nowAction = nextAction;
	m_nowAction->Enter(m_wpThis, spTarget);

}

void Enemy::ActionStateBase::EffectUpdate(std::weak_ptr<Enemy>& owner)
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

void Enemy::ActionStateBase::EffectExit()
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

void Enemy::ActionStateBase::ChangeStateWithDisttance(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();

	auto spTarget = spObj;

	if (spTarget == nullptr) { return; }

	auto distance = spTarget->GetPos() - spOwner->GetPos();
	auto length = distance.Length();
	auto halfOnwenrLength = (spOwner->m_dist.y - spOwner->m_dist.x) / 2.0f;

	// プレイヤーと自身との差がどれくらいか
	if (length > spOwner->m_dist.y)
	{
		spOwner->ChangeActionState(std::make_shared<Boost>());
		return;
	}
	else if (length > halfOnwenrLength)
	{
		spOwner->ChangeActionState(std::make_shared<MoveForward>());
		return;
	}
	else if (length > halfOnwenrLength)
	{
		spOwner->ChangeActionState(std::make_shared<AttackStand>());
		return;
	}
	else {
		spOwner->ChangeActionState(std::make_shared<MoveBack>());
		return;
	}

}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//待機状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::Start::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandUp"), 100.0f);
}

void Enemy::Start::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{

	Application::Instance().m_log.AddLog("EnemynowState: Start\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();
	auto difference = spObj->GetMatrix().Translation() - spOwner->m_mWorld.Translation();


	bool isFind = spOwner->Search();

	if (isFind) {
		spOwner->ChangeActionState(std::make_shared<StandUp>());
		return;
	}
}

void Enemy::Start::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 0.0f);
}

void Enemy::Start::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//立ち上がり
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::StandUp::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandUp"), 100.0f,false);
}

void Enemy::StandUp::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	// 立ち上がるだけ
	Application::Instance().m_log.AddLog("EnemynowState: StandUp\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	if (spOwner->m_spAnimator->GetProgress() >= 1.0f)
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}

}

void Enemy::StandUp::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 7.0f);
}

void Enemy::StandUp::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
}



//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//スタンド状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::Stand::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Stand"), 3.0f);
}

void Enemy::Stand::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{

	Application::Instance().m_log.AddLog("EnemynowState: Stand\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();
	auto difference = spObj->GetMatrix().Translation() - spOwner->m_mWorld.Translation();


	if (difference.Length() <= spOwner->m_dist.y)
	{
		spOwner->ChangeActionState(std::make_shared<Boost>());
		return;
	}

}

void Enemy::Stand::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}


void Enemy::Stand::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::Boost::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Boost"), 5.0f, false);

	m_speed = 100.0f;

	KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("CBP");
	if (pNode)
	{
		std::shared_ptr<Effect> effect = std::make_shared<Effect>();
		effect->name = "ThrusterE.efkefc";
		effect->pNodeMat = pNode->m_worldTransform;
		effect->wpEffect = KdEffekseerManager::GetInstance().Play("ThrusterE.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation());
		m_spEffects.push_back(effect);
	}
}

void Enemy::Boost::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: Boost\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	auto deltaTime = KdFPSController::GetInstance().GetDeltaTime();

	//現在の座標
	Math::Vector3 nowPos = spOwner->GetMatrix().Translation();

	//追尾対象の座標
	Math::Vector3 targetPos = spObj->GetMatrix().Translation();

	//対象への長さ
	Math::Vector3 difference = targetPos - nowPos;

	auto distHalf = (spOwner->m_dist.y - spOwner->m_dist.x) / 2.0f;

	if (spOwner->m_spAnimator->IsAnimationEnd() == true)
	{
		if (difference.LengthSquared() <= distHalf)
		{
			auto& key = KeyInput::GetInstance().GetKeyboardStateData();
			auto& pad = KeyInput::GetInstance().GetGamePadStateData();

			int flg = 0;

			for (auto& data : key)
			{
				if (data.A)
				{
					flg = 2;
					break;
				}
				else if (data.D)
				{
					flg = 3;
					break;
				}
				else if (data.W)
				{
					flg = 4;
					break;
				}
				else if (data.S)
				{
					flg = 1;
					break;
				}
			}

			if(flg == 0)
			{ 
				for (auto& data : pad)
				{
					if (data.IsLeftThumbStickLeft())
					{
						flg = 2;
						break;
					}
					else if (data.IsLeftThumbStickRight())
					{
						flg = 3;
						break;
					}
					else if (data.IsLeftThumbStickUp())
					{
						flg = 4;
						break;
					}
					else if (data.IsLeftThumbStickDown())
					{
						flg = 1;
						break;
					}
				}
			}

			//前進
			if (flg == 1)
			{
				spOwner->ChangeActionState(std::make_shared<MoveForward>());
				return;
			}
			// 右回り
			else if(flg == 2)
			{
				spOwner->ChangeActionState(std::make_shared<MoveRightRotate>());
				return;
			}
			// 左回り
			else if(flg == 3)
			{
				spOwner->ChangeActionState(std::make_shared<MoveLeftRotate>());
				return;
			}
			// 後退
			else if (flg == 4)
			{
				spOwner->ChangeActionState(std::make_shared<MoveBack>());
				return;
			}
		}
		else
		{
			spOwner->ChangeActionState(std::make_shared<BoostStop>());
			return;
		}

	}

	//ベクトル
	Math::Vector3 nowVec = spOwner->GetMatrix().Backward();
	Math::Vector3 targetVec = difference;

	nowVec.Normalize();
	targetVec.Normalize();

	//内積を使って回転する角度を求める
	float d = nowVec.Dot(targetVec);
	//dの中にはコサインΘが入っている

	//角度求める
	float ang = DirectX::XMConvertToDegrees(acos(d));

	if (ang >= 0.1f)
	{
		if (ang > 10)
		{
			ang = 10.0f;
		}

		Math::Vector3 c = targetVec.Cross(nowVec);

		if (c.y >= 0)
		{
			//右回転
			spOwner->m_worldRot.y -= ang;
		}
		else
		{
			//左回転
			spOwner->m_worldRot.y += ang;
		}
	}

	if (spOwner->m_worldRot.y > 360)
	{
		spOwner->m_worldRot.y -= 360;
	}
	else if (spOwner->m_worldRot.y < 0)
	{
		spOwner->m_worldRot.y += 360;
	}

	Math::Matrix rotMat = Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(spOwner->m_worldRot.y),
		DirectX::XMConvertToRadians(spOwner->m_worldRot.x),
		DirectX::XMConvertToRadians(spOwner->m_worldRot.z));

	Math::Vector3 vec = {};

	Math::Matrix mat = rotMat * Math::Matrix::CreateTranslation(nowPos);

	vec = mat.Backward();
	vec.Normalize();

	nowPos += vec * m_speed * deltaTime;

	Math::Matrix transMat = Math::Matrix::CreateTranslation(nowPos);
	spOwner->m_mWorld = rotMat * transMat;


	//エフェクト
	EffectUpdate(owner);

}

void Enemy::Boost::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f);

}

void Enemy::Boost::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト停止
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::BoostStop::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostStop"), 6.0f, false);

	m_direct = spOwner->m_mWorld.Backward();
	m_direct.Normalize();

	m_speed = 50.0f;

}

void Enemy::BoostStop::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: BoostStop\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();
	auto difference = spObj->GetMatrix().Translation() - spOwner->m_mWorld.Translation();

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		if (difference.Length() <= spOwner->m_dist.y)
		{

			spOwner->ChangeActionState(std::make_shared<MoveForward>());
			return;
		}

		int i = rand() % 5;

		if (i < 2)
		{
			
			return;
		}
		else if (i < 4)
		{
			spOwner->ChangeActionState(std::make_shared<MoveBack>());
			return;
		}
		else
		{
			spOwner->ChangeActionState(std::make_shared<Boost>());
			return;
		}
	}

	auto deltaTime = KdFPSController::GetInstance().GetDeltaTime();

	//ベクトル
	Math::Vector3 nowVec = spOwner->GetMatrix().Backward();
	Math::Vector3 targetVec = difference;

	//現在の座標
	Math::Vector3 nowPos = spOwner->GetMatrix().Translation();


	nowVec.Normalize();
	targetVec.Normalize();


	Math::Matrix rotMat = Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(spOwner->m_worldRot.y),
		DirectX::XMConvertToRadians(spOwner->m_worldRot.x),
		DirectX::XMConvertToRadians(spOwner->m_worldRot.z));

	Math::Vector3 vec = {};


	vec = m_direct;

	nowPos += vec * m_speed * deltaTime;

	Math::Matrix transMat = Math::Matrix::CreateTranslation(nowPos);

	spOwner->m_mWorld = rotMat * transMat;

}

void Enemy::BoostStop::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f);
}

void Enemy::BoostStop::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	//エフェクト
	for (auto& eff : m_spEffects)
	{
		auto spefct = eff->wpEffect.lock();
		auto mat = eff->pNodeMat;
		KdEffekseerManager::GetInstance().StopEffect(eff->name);
		KdEffekseerManager::GetInstance().SetScale(spefct->GetHandle(), 0.0f);
		spefct->SetScale(0.0f);
		spefct->SetLoop(false);
	}
	m_spEffects.clear();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 正面移動
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::MoveForward::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostDush"), 10.0f);

	m_speed = 30.0f;
}

void Enemy::MoveForward::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: MoveForward\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	auto myMat = spOwner->GetMatrix();
	auto targetMat = spObj->GetMatrix();

	auto difference = targetMat.Translation() - myMat.Translation();

	if (spOwner->m_spAnimator->IsAnimationEnd() == true)
	{
		auto distHalf = (spOwner->m_dist.y - spOwner->m_dist.x) / 2.0f;

		if (difference.LengthSquared() < distHalf)
		{
			spOwner->ChangeActionState(std::shared_ptr<AttackForWard>());
			return;
		}


		bool move = false;
		for (auto& key : KeyInput::GetInstance().GetKeyboardStateData())
		{
			if (key.A || key.W || key.S || key.D)
			{
				move = true;
				break;
			}
		}

		if (move == false)
		{
			for (auto& pad : KeyInput::GetInstance().GetGamePadStateData())
			{
				if (pad.IsLeftThumbStickDown() || pad.IsLeftThumbStickLeft() || pad.IsLeftThumbStickRight() || pad.IsLeftThumbStickUp())
				{
					move = true;
					break;
				}
			}
		}

		if (move == true)
		{
			spOwner->ChangeActionState(std::make_shared<MoveBack>());
			return;
		}
	}

	auto deltaTime = KdFPSController::GetInstance().GetDeltaTime();

	//現在の座標
	Math::Vector3 nowPos = spOwner->GetMatrix().Translation();

	//ベクトル
	Math::Vector3 nowVec = spOwner->GetMatrix().Backward();
	Math::Vector3 targetVec = difference;

	nowVec.Normalize();
	targetVec.Normalize();

	//内積を使って回転する角度を求める
	//ベクトルA*ベクトルB*cosΘ(ベクトルAとベクトルBのなす角)
	//			1	*	1	* cosΘ			
	float d = nowVec.Dot(targetVec);
	//dの中にはコサインΘが入っている

	//角度求める(でも残念ながらラジアン角)11
	float ang = DirectX::XMConvertToDegrees(acos(d));

	//内積から角度を求めて少しでも角度が変わったら
	//ゆっくり回転するようにする
	if (ang >= 0.1f)
	{
		if (ang > 10)
		{
			ang = 10.0f;
		}

		//外積を求める（どっっちに回転するのか調べる）
		Math::Vector3 c = targetVec.Cross(nowVec);

		if (c.y >= 0)
		{
			//右回転
			spOwner->m_worldRot.y -= ang;
		}
		else
		{
			//左回転
			spOwner->m_worldRot.y += ang;
		}
	}

	if (spOwner->m_worldRot.y > 360)
	{
		spOwner->m_worldRot.y -= 360;
	}
	else if (spOwner->m_worldRot.y < 0)
	{
		spOwner->m_worldRot.y += 360;
	}

	Math::Matrix rotMat = Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(spOwner->m_worldRot.y),
		DirectX::XMConvertToRadians(spOwner->m_worldRot.x),
		DirectX::XMConvertToRadians(spOwner->m_worldRot.z));

	//	if (difference.Length() > spOwner->m_dist.x)
	{
		Math::Vector3 vec = {};

		Math::Matrix mat = rotMat * Math::Matrix::CreateTranslation(nowPos);

		vec = mat.Backward();

		vec.Normalize();

		nowPos += vec * m_speed * deltaTime;
	}


	Math::Matrix transMat = Math::Matrix::CreateTranslation(nowPos);

	spOwner->m_mWorld = rotMat * transMat;

}

void Enemy::MoveForward::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 30.0f);
}

void Enemy::MoveForward::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 後退
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::MoveBack::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostBack"), 4.0f);

	m_speed = 20.0f;

}

void Enemy::MoveBack::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: MoveBack\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();


	auto deltaTime = KdFPSController::GetInstance().GetDeltaTime();

	//現在の座標
	Math::Vector3 nowPos = spOwner->GetMatrix().Translation();

	//追尾対象の座標
	Math::Vector3 targetPos = spObj->GetMatrix().Translation();

	//対象への長さ
	Math::Vector3 difference = targetPos - nowPos;

	auto distHalf = (spOwner->m_dist.y - spOwner->m_dist.x) / 2.0f;

	// 範囲内の半分より遠い時
	if (difference.LengthSquared() > distHalf)
	{
		spOwner->ChangeActionState(std::make_shared<MoveRightRotate>());
		return;
	}
	else
	{
		// 
		auto& keyData = KeyInput::GetInstance().GetKeyboardStateData();
		auto& padData = KeyInput::GetInstance().GetGamePadStateData();

		//if()

	}


	//ベクトル
	Math::Vector3 nowVec = spOwner->GetMatrix().Backward();
	Math::Vector3 targetVec = difference;

	nowVec.Normalize();
	targetVec.Normalize();

	//内積を使って回転する角度を求める
	//ベクトルA*ベクトルB*cosΘ(ベクトルAとベクトルBのなす角)
	//			1	*	1	* cosΘ			
	float d = nowVec.Dot(targetVec);
	//dの中にはコサインΘが入っている

	//角度求める(でも残念ながらラジアン角)11
	float ang = DirectX::XMConvertToDegrees(acos(d));

	//内積から角度を求めて少しでも角度が変わったら
	//ゆっくり回転するようにする
	if (ang >= 0.1f)
	{
		if (ang > 10)
		{
			ang = 10.0f;
		}

		//外積を求める（どっっちに回転するのか調べる）
		Math::Vector3 c = targetVec.Cross(nowVec);

		if (c.y >= 0)
		{
			//右回転
			spOwner->m_worldRot.y -= ang;
		}
		else
		{
			//左回転
			spOwner->m_worldRot.y += ang;
		}
	}

	if (spOwner->m_worldRot.y > 360)
	{
		spOwner->m_worldRot.y -= 360;
	}
	else if (spOwner->m_worldRot.y < 0)
	{
		spOwner->m_worldRot.y += 360;
	}

	Math::Matrix rotMat = Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(spOwner->m_worldRot.y),
		DirectX::XMConvertToRadians(spOwner->m_worldRot.x),
		DirectX::XMConvertToRadians(spOwner->m_worldRot.z));

	//	if (difference.Length() > spOwner->m_dist.x)
	{
		Math::Vector3 vec = {};

		Math::Matrix mat = rotMat * Math::Matrix::CreateTranslation(nowPos);

		vec = mat.Forward();

		vec.Normalize();

		nowPos += vec * m_speed * deltaTime;
	}


	Math::Matrix transMat = Math::Matrix::CreateTranslation(nowPos);

	spOwner->m_mWorld = rotMat * transMat;
}

void Enemy::MoveBack::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::MoveBack::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 右回り込み
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::MoveRightRotate::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostRight"), 3.0f);

	m_speed = 30.0f;
}

void Enemy::MoveRightRotate::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: MoveRightRotate\n");

	auto spOwner = owner.lock();

	auto ownerPos = spOwner->GetMatrix().Translation();
	auto targetPos = spObj->GetMatrix().Translation();

	auto difference = targetPos - ownerPos;

	//ステート変化
	if (difference.Length() < spOwner->m_dist.x)
	{
		spOwner->ChangeActionState(std::make_shared<AttackRight>());
		return;
	}


	auto deltaTime = KdFPSController::GetInstance().GetDeltaTime();

	auto nowPos = spOwner->GetMatrix().Translation();

	//ベクトル
	Math::Vector3 nowVec = spOwner->GetMatrix().Backward();
	Math::Vector3 targetVec = difference;

	nowVec.Normalize();
	targetVec.Normalize();

	//内積を使って回転する角度を求める
	//ベクトルA*ベクトルB*cosΘ(ベクトルAとベクトルBのなす角)
	//			1	*	1	* cosΘ			
	float d = nowVec.Dot(targetVec);
	//dの中にはコサインΘが入っている

	//角度求める(でも残念ながらラジアン角)11
	float ang = DirectX::XMConvertToDegrees(acos(d));

	//内積から角度を求めて少しでも角度が変わったら
	//ゆっくり回転するようにする
	if (ang >= 0.1f)
	{
		if (ang > 10)
		{
			ang = 10.0f;
		}

		//外積を求める（どっっちに回転するのか調べる）
		Math::Vector3 c = targetVec.Cross(nowVec);

		if (c.y >= 0)
		{
			//右回転
			spOwner->m_worldRot.y -= ang;
		}
		else
		{
			//左回転
			spOwner->m_worldRot.y += ang;
		}
	}

	if (spOwner->m_worldRot.y > 360)
	{
		spOwner->m_worldRot.y -= 360;
	}
	else if (spOwner->m_worldRot.y < 0)
	{
		spOwner->m_worldRot.y += 360;
	}

	Math::Matrix rotMat = Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(spOwner->m_worldRot.y),
		DirectX::XMConvertToRadians(spOwner->m_worldRot.x),
		DirectX::XMConvertToRadians(spOwner->m_worldRot.z));

	//	if (difference.Length() > spOwner->m_dist.x)
	{
		Math::Vector3 vec = {};

		Math::Matrix mat = rotMat * Math::Matrix::CreateTranslation(nowPos);

		vec = mat.Right();

		vec.Normalize();

		nowPos += vec * m_speed * deltaTime;
	}


	Math::Matrix transMat = Math::Matrix::CreateTranslation(nowPos);

	spOwner->m_mWorld = rotMat * transMat;

}
void Enemy::MoveRightRotate::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::MoveRightRotate::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 左回り込み
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::MoveLeftRotate::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostLeft"), 3.0f);

	m_speed = 30.0f;
}

void Enemy::MoveLeftRotate::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: MoveLeftRotate\n");

	auto spOwner = owner.lock();

	auto ownerPos = spOwner->GetMatrix().Translation();
	auto targetPos = spObj->GetMatrix().Translation();

	auto difference = targetPos - ownerPos;

	//ステート変化
	if (difference.Length() < spOwner->m_dist.x)
	{
		spOwner->ChangeActionState(std::make_shared<AttackLeft>());
		return;
	}


	auto deltaTime = KdFPSController::GetInstance().GetDeltaTime();

	auto nowPos = spOwner->GetMatrix().Translation();

	//ベクトル
	Math::Vector3 nowVec = spOwner->GetMatrix().Backward();
	Math::Vector3 targetVec = difference;

	nowVec.Normalize();
	targetVec.Normalize();

	//内積を使って回転する角度を求める
	//ベクトルA*ベクトルB*cosΘ(ベクトルAとベクトルBのなす角)
	//			1	*	1	* cosΘ			
	float d = nowVec.Dot(targetVec);
	//dの中にはコサインΘが入っている

	//角度求める(でも残念ながらラジアン角)11
	float ang = DirectX::XMConvertToDegrees(acos(d));

	//内積から角度を求めて少しでも角度が変わったら
	//ゆっくり回転するようにする
	if (ang >= 0.1f)
	{
		if (ang > 10)
		{
			ang = 10.0f;
		}

		//外積を求める（どっっちに回転するのか調べる）
		Math::Vector3 c = targetVec.Cross(nowVec);

		if (c.y >= 0)
		{
			//右回転
			spOwner->m_worldRot.y -= ang;
		}
		else
		{
			//左回転
			spOwner->m_worldRot.y += ang;
		}
	}

	if (spOwner->m_worldRot.y > 360)
	{
		spOwner->m_worldRot.y -= 360;
	}
	else if (spOwner->m_worldRot.y < 0)
	{
		spOwner->m_worldRot.y += 360;
	}

	Math::Matrix rotMat = Math::Matrix::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(spOwner->m_worldRot.y),
		DirectX::XMConvertToRadians(spOwner->m_worldRot.x),
		DirectX::XMConvertToRadians(spOwner->m_worldRot.z));

	//	if (difference.Length() > spOwner->m_dist.x)
	{
		Math::Vector3 vec = {};

		Math::Matrix mat = rotMat * Math::Matrix::CreateTranslation(nowPos);

		vec = mat.Left();

		vec.Normalize();

		nowPos += vec * m_speed * deltaTime;
	}


	Math::Matrix transMat = Math::Matrix::CreateTranslation(nowPos);

	spOwner->m_mWorld = rotMat * transMat;

}

void Enemy::MoveLeftRotate::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::MoveLeftRotate::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 直立攻撃状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::AttackStand::Enter(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandAttack"), 3.0f);
}

void Enemy::AttackStand::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemyowState: StandAttack\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}
}

void Enemy::AttackStand::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::AttackStand::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 前進攻撃状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::AttackForWard::Enter(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("FrontAttack"), 3.0f);
}

void Enemy::AttackForWard::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemyowState: FrontAttack\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}
}

void Enemy::AttackForWard::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::AttackForWard::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 後退攻撃
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::AttackBack::Enter(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BackAttack"), 3.0f);
}

void Enemy::AttackBack::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemyowState: Attack\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}
}

void Enemy::AttackBack::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::AttackBack::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 左回り攻撃
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::AttackLeft::Enter(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("LeftAttack"), 3.0f);
}

void Enemy::AttackLeft::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemyowState: Attack\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}
}

void Enemy::AttackLeft::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::AttackLeft::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 右回り攻撃
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::AttackRight::Enter(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("RightAttack"), 3.0f);
}

void Enemy::AttackRight::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemyowState: Attack\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}
}

void Enemy::AttackRight::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::AttackRight::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// ダメージ状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::Hited::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Hited"), 10.0f,false);
}

void Enemy::Hited::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: Hited\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	//攻撃をくらったとき
	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}
}

void Enemy::Hited::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::Hited::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 死亡状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::Destoroy::Enter(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Destroyed"), 5.0f, false);
}

void Enemy::Destoroy::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: Destory\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	//死亡時
	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}

}

void Enemy::Destoroy::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	if(spOwner->m_spAnimator->GetProgress()<=0.3)
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 2.0f);
	else {
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);
	}
}

void Enemy::Destoroy::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

}
