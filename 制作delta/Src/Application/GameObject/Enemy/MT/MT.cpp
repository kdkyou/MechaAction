#include "MT.h"

#include "../../../Scene/SceneManager.h"

#include "../../../main.h"

#include "../../UI/Alert/Alert.h"
#include "../../Camera/CameraManager.h"

void MT::Init()
{
	m_limEnable = true;
	m_limColor = { 0.52f,0.5f,0.58f };
	m_limPow = 1.0f;

	m_spAnimator = std::make_shared<KdAnimator>();


	m_correction = { 0.0f,5.0f,0.0f };

	m_correctionMat = Math::Matrix::CreateTranslation(m_correction);

	m_boxExtents = { 4.0f,5.0f,3.0f };

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape("Enemy", m_spModelWork, KdCollider::TypeDamage);

	m_dist = { 30.0f,200.0f };

	ChangeActionState(std::make_shared<StandUp>());

	m_name = "MT";

	m_viewAngle = 140.0f;

	m_nockBackDamage = 800;
}

void MT::Update()
{


	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpCharacterTarget.lock();

		if (spTarget == nullptr)
		{
			spTarget = m_wpTarget.lock();
		}

		std::shared_ptr<MT> spThis = m_wpThis.lock();

		if (spThis == nullptr)
		{
			return;
		}
		m_nowAction->Update(m_wpThis, spTarget);

	}

	Move(m_gravity, Math::Vector3::Down, KdCollider::TypeGround, false, false, true, false);
	m_gravity += m_gravityPow * KdFPSController::GetInstance().GetDeltaTime();

	UpdateCollision();


}

void MT::PostUpdate()
{
	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpCharacterTarget.lock();

		if (spTarget == nullptr)
		{
			spTarget = m_wpTarget.lock();
		}

		std::shared_ptr<MT> spThis = m_wpThis.lock();

		if (spThis == nullptr)
		{
			return;
		}
		m_nowAction->PostUpdate(m_wpThis, spTarget);
	}

}

void MT::DrawLit()
{
	if (!m_spModelWork) return;

	if (m_limEnable)
	{
		KdShaderManager::Instance().m_StandardShader.SetLimLightEnable(true);
		KdShaderManager::Instance().m_StandardShader.SetLimlightParam(m_limColor, m_limPow);

	}

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld);

	KdShaderManager::Instance().m_StandardShader.SetLimLightEnable(false);
}

void MT::OnHit()
{
	if (m_hp <= 0)
	{
		ChangeActionState(std::make_shared<Destroyed>());
		m_parameter = 0;
		return;
	}

	if (m_parameter >= m_nockBackDamage)
	{
		ChangeActionState(std::make_shared<Hited>());
		m_parameter = 0;
		return;
	}
}

void MT::UpdateRotate(const Math::Vector3& srcMoveVec)
{
	auto nowVec = GetMatrix().Backward();

	//内積を使って回転する角度を求める
	float d = nowVec.Dot(srcMoveVec);
	//dの中にはコサインΘが入っている

	//角度求める
	float ang = DirectX::XMConvertToDegrees(acos(d));

	if (ang >= 0.1f)
	{
		if (ang > m_clampSize)
		{
			ang = m_clampSize;
		}

		Math::Vector3 c = srcMoveVec.Cross(nowVec);

		if (c.y >= 0)
		{
			//右回転
			m_rot.y -= ang;
		}
		else
		{
			//左回転
			m_rot.y += ang;
		}
	}

	if (m_rot.y > 360)
	{
		m_rot.y -= 360;
	}
	else if (m_rot.y < 0)
	{
		m_rot.y += 360;
	}

}

void MT::UpdateCollision()
{
	DirectX::BoundingOrientedBox box;

	box.Center = GetPos();
	box.Extents = m_boxExtents;
	UINT type = KdCollider::TypeDamage;
	KdCollider::BoxInfo boxInfo(type, box);

	auto translation = m_mWorld.Translation();

	for (auto obj : SceneManager::Instance().GetObjList())
	{
		if (obj->Intersects(boxInfo, nullptr))
		{
			if (obj->GetTag() == tPlayerAttack)
			{
				obj->OnHit();
				HitDamage(obj->GetParameter());
				m_parameter = obj->GetParameter();
				OnHit();
			}
		}
	}



}

bool MT::Search(bool areaOnly)
{
	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = m_mWorld.Translation() + m_correction;
	sphere.m_sphere.Radius = m_dist.y;
	sphere.m_type = KdCollider::TypeDamage;

	std::list< KdCollider::CollisionResult> retList;
	std::list<std::shared_ptr<KdGameObject>> objList;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{

		if (obj->Intersects(sphere, &retList))
		{
			objList.push_back(obj);
		}
	}

	if (retList.empty() == true) {
		return false;
	}

	Math::Vector3 hitPos = {};
	m_overRap = 0.0f;
	bool isHit = false;

	std::shared_ptr<KdGameObject> obj;
	auto it = objList.begin();

	for (auto& ret : retList)
	{
		if (m_overRap < ret.m_overlapDistance)
		{
			obj = *it;

			if (obj->GetTag() == tPlayer || obj->GetTag() == tPlayerAttack)
			{
				hitPos = ret.m_hitPos;
				m_overRap = ret.m_overlapDistance;
				isHit = true;

				// 索敵のみ
				if (areaOnly == true)
				{
					if (obj->GetTag() == tPlayer)
					{
						m_wpTarget = obj;
						return true;
					}
				}
			}
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

			Math::Vector3 vec = hitPos - m_mWorld.Translation();
			vec.Normalize();

			bool isClear = SeaarchObstacle(hitPos, vec, m_overRap);

			if (isClear) {
				return true;
			}
			else
			{
				return false;
			}

		}
	}

	return false;
}



void MT::Editor_ImGui()
{
	CharacterBase::Editor_ImGui();

}

void MT::Deserialize(const nlohmann::json& jsonObj)
{
	CharacterBase::Deserialize(jsonObj);
}

void MT::Serialize(nlohmann::json& outJson) const
{
	CharacterBase::Serialize(outJson);
}



void MT::ChangeActionState(std::shared_ptr<ActionStateBase> nextAction)
{
	if (m_nowAction)m_nowAction->Exit(m_wpThis, m_wpTarget);
	m_prevAction = m_nowAction;
	m_nowAction = nextAction;
	m_nowAction->Enter(m_wpThis, m_wpTarget);
}

void MT::ActionStateBase::CreateEffect(std::weak_ptr<MT>& owner, const std::string& effectName, const std::string& nodeName)
{
	auto spOwner = owner.lock();
	KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode(nodeName);
	if (pNode)
	{
		std::shared_ptr<Effect> effect = std::make_shared<Effect>();
		effect->name = effectName;
		effect->pNodeMat = pNode->m_worldTransform;
		effect->wpEffect = KdEffekseerManager::GetInstance().Play(effectName, pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 1.0f, 3.0f);
		m_spEffects.push_back(effect);
	}
}

void MT::ActionStateBase::EffectUpdate(std::weak_ptr<MT>& owner)
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

void MT::ActionStateBase::EffectExit()
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

void MT::ActionStateBase::ChangeStateWithDistance(std::weak_ptr<MT>& owner, float targetLength)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }

	auto target = spOwner->GetCharacterTarget().lock();

	if (target) {

		if (target->IsDestroy())
		{
			spOwner->ChangeActionState(std::make_shared<Idle>());
			return;
		}
	}

	float length = spOwner->GetDist().y - spOwner->GetDist().x;

	float halfLength = length / 2.0f;
	if (targetLength > length) { return; }

	if (targetLength > halfLength)
	{
		spOwner->ChangeActionState(std::make_shared<MoveMent>());
		return;
	}
	else if (targetLength< halfLength && targetLength> spOwner->GetDist().x)
	{
		spOwner->ChangeActionState(std::make_shared<Attack>());
		return;
	}
	else {

		//spOwner->SeaarchObstacle();

		spOwner->ChangeActionState(std::make_shared<Backed>());
		return;
	}
}

bool MT::ActionStateBase::ChangeStateObstacle(std::weak_ptr<MT>& owner)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return false; }

	auto target = spOwner->GetCharacterTarget().lock();

	if (target == nullptr) { return false; }

	auto vec = target->GetPos() - spOwner->GetPos();
	auto overRap = vec.Length();
	vec.Normalize();

	bool isClear = spOwner->SeaarchObstacle(spOwner->GetPos(), vec, overRap);

	if (!isClear)
	{

		if (spOwner->SeaarchObstacle(spOwner->GetPos(), vec.Left, overRap))
		{
			spOwner->ChangeActionState(std::make_shared<MoveMent>());
			spOwner->m_nowAction->SetMoveDir(Left);
			return true;
		}
		else if (spOwner->SeaarchObstacle(spOwner->GetPos(), vec.Right, overRap))
		{
			spOwner->ChangeActionState(std::make_shared<MoveMent>());
			spOwner->m_nowAction->SetMoveDir(Right);
			return true;
		}

	}

	return false;

}


void MT::StandUp::Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	m_durationState = 5.0f;

	m_speed = 0.0f;

	if (spOwner->m_spAnimator)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("WakeUp"), 5.0f, false);
	}
}

void MT::StandUp::Update(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }

	// 範囲内にプレイヤーがいるか
	if (!spOwner->SearchPlayer()) { return; }
	else {
		m_anyFlg = true;
	}

	if (spOwner->m_spAnimator)
	{
		if (spOwner->m_spAnimator->IsAnimationEnd())
		{
			auto target = obj.lock();

			if (target == nullptr) { return; }

			auto dist = target->GetPos() - spOwner->GetPos();
			float len = dist.Length();

			spOwner->ChangeActionState(std::make_shared<Idle>());
			return;
		}
	}
}

void MT::StandUp::PostUpdate(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{

	auto spOwner = owner.lock();
	auto animator = spOwner->m_spAnimator;
	// おそらくエフェクト関連

	if (animator)
	{
		if (m_anyFlg)
		{
			animator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f);
		}

	}
}

void MT::StandUp::Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{

}
void MT::Idle::Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	m_durationState = 1.0f;

	m_speed = 0.0f;

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("Stand"), 10.0f, false);
}

void MT::Idle::Update(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	//if (spOwner->SearchPlayer())
	{
		if (m_durationState < 0)
		{
			auto target = obj.lock();

			if (target == nullptr) { return; }

			auto dist = target->GetPos() - spOwner->GetPos();
			float len = dist.Length();

			ChangeStateWithDistance(owner, len);
		}

	}
}

void MT::Idle::PostUpdate(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto animator = spOwner->m_spAnimator;
	// おそらくエフェクト関連
	if (animator)
	{
		animator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f);

	}
}

void MT::Idle::Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{

}

void MT::MoveMent::Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	m_speed = 30.0f;

	m_durationState = 0.95f;

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("Move"), 10.0f, false);

	 int i =spOwner->m_rand.GetInt(1, 3);
	 if (i == 1)
	 {
		 m_side = Left;
	 }
	 else if (i == 2)
	 {
		 m_side = Front;
	 }
	 else {
		 m_side = Right;
	 }

}

void MT::MoveMent::Update(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }

	auto spTarget = obj.lock();

	if (spTarget == nullptr) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	auto pos = spOwner->GetMatrix().Translation();

	auto targetPos = spTarget->GetMatrix().Translation();

	auto diff = targetPos - pos;

	diff.Normalize();

	diff.x = sin(DirectX::XMConvertToRadians(m_sin));
	m_sin += 10 * KdFPSController::GetInstance().GetDeltaTime();

	Math::Vector3 vec = diff;

	if (m_side == TargetSide::Left)
	{
		vec = diff.Left;
	}
	else if (m_side == TargetSide::Right)
	{
		vec = diff.Right;
	}


	spOwner->Move(m_speed, vec, KdCollider::TypeGround);

	if (m_durationState < 0)
	{
		float len = (targetPos - pos).Length();
		ChangeStateWithDistance(owner, len);
		return;
	}

}

void MT::MoveMent::PostUpdate(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto animator = spOwner->m_spAnimator;
	// おそらくエフェクト関連
	if (animator)
	{
		animator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f);

	}
}

void MT::MoveMent::Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
}

void MT::Attack::Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }

	m_speed = 20.0f;

	m_durationState = 0.36f;

	bool flg = ChangeStateObstacle(owner);
	if (!flg)
	{
		spOwner->ChangeEnableRightAttack(true);

		auto alert = std::make_shared<Alert>();
		auto pos = CameraManager::Instance().GetLocalDirectionTo(spOwner->GetMatrix().Translation());
		alert->CalcPos(pos);
		alert->Init();
		SceneManager::Instance().AddObject(alert);

		int i = spOwner->m_rand.GetInt(1, 3);
		if (i == 1)
		{
			m_side = Left;
		}
		else if (i == 2)
		{
			m_side = Front;
		}
		else {
			m_side = Right;
		}
	}



	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("Attack"), 10.0f, false);

}

void MT::Attack::Update(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }

	auto spTarget = obj.lock();

	if (spTarget == nullptr) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	auto pos = spOwner->GetMatrix().Translation();

	auto targetPos = spTarget->GetMatrix().Translation();

	auto diff = targetPos - pos;

	diff.y = 0.0f;
	diff.Normalize();
	auto vec = diff;

	if (m_side == TargetSide::Left)
	{
		vec = diff.Left;
	}
	else if (m_side == TargetSide::Right)
	{
		vec = diff.Right;
	}

	spOwner->Move(m_speed, diff, KdCollider::TypeGround);

	if (m_durationState < 0)
	{
		float len = (targetPos - pos).Length();
		ChangeStateWithDistance(owner, len);
		return;
	}
}

void MT::Attack::PostUpdate(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto animator = spOwner->m_spAnimator;
	// おそらくエフェクト関連
	if (animator)
	{
		animator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f);

	}
}

void MT::Attack::Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }

	spOwner->ChangeEnableRightAttack(false);

}

void MT::Hited::Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }

	m_speed = 20.0f;

	m_durationState = 0.36f;

	bool flg = ChangeStateObstacle(owner);

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("Hited"), 10.0f, false);

}

void MT::Hited::Update(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }

	auto spTarget = obj.lock();

	if (spTarget == nullptr) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	auto pos = spOwner->GetMatrix().Translation();

	auto targetPos = spTarget->GetMatrix().Translation();

	auto diff = targetPos - pos;

	diff.y = 0.0f;
	diff.Normalize();

	spOwner->Move(m_speed, diff, KdCollider::TypeGround);

	if (m_durationState < 0)
	{
		float len = (targetPos - pos).Length();
		ChangeStateWithDistance(owner, len);
		return;
	}
}

void MT::Hited::PostUpdate(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto animator = spOwner->m_spAnimator;
	// おそらくエフェクト関連
	if (animator)
	{
		animator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f);

	}
}

void MT::Hited::Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }

	spOwner->ChangeEnableRightAttack(false);

}


void MT::Destroyed::Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	m_speed = 30.0f;

	m_durationState = 5.0f;

	auto spOwner = owner.lock();

	spOwner->m_isDestroy = true;
	
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("Destroyed"), 10.0f, false);

	KdAudioManager::Instance().Play("Asset/Sounds/Sound/down_enemy.wav")->SetVolume(0.3f);

}

void MT::Destroyed::Update(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }


	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	spOwner->Move(m_speed, Math::Vector3::Down, KdCollider::TypeGround, false, false);

	if (m_durationState <= 0)
	{

	}

}

void MT::Destroyed::PostUpdate(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
}

void MT::Destroyed::Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
}

void MT::Backed::Enter(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	m_speed = 20.0f;

	m_durationState = 0.85f;

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("Move"), 10.0f, false);

}

void MT::Backed::Update(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }

	auto spTarget = obj.lock();

	if (spTarget == nullptr) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	auto pos = spOwner->GetMatrix().Translation();

	auto targetPos = spTarget->GetMatrix().Translation();

	auto diff = pos - targetPos;

	diff.y = 0.0f;
	diff.Normalize();

	spOwner->Move(m_speed, diff, KdCollider::TypeGround, false, false);

	if (m_durationState < 0)
	{
		float len = (targetPos - pos).Length();
		ChangeStateWithDistance(owner, len);
		return;
	}
}

void MT::Backed::PostUpdate(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
}

void MT::Backed::Exit(std::weak_ptr<MT>& owner, const std::weak_ptr<KdGameObject>& obj)
{
}

