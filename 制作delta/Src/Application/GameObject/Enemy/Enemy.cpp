#include"Enemy.h"


#include"../../main.h"

#include "../../Scene/SceneManager.h"

#include"../Character/CharacterBase.h"

#include "../Camera/CameraManager.h"
#include "../UI/Alert/Alert.h"

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

	m_correction = { 0.0f,5.0f,0.0f };
	m_correctionMat = Math::Matrix::CreateTranslation({ 0.0f,5.0f,0.0f });

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	SetPos({ 0.0f,0.0f,600.0f });

	//初期状態を「待機状態」へ設定
	ChangeActionState(std::make_shared<Start>());

	m_dist = { 50.0f,300.0f };

	m_clampSize = 20.0f;

	m_hp = 10320.0f;

	m_nockBackDamage = 800.0f;

	m_burnPath = "Asset/Textures/GameObject/Burn.png";

}

void Enemy::Update()
{
	Application::Instance().m_log.AddLog("Another\n");
	//各種「状態に応じた」更新処理を実行する
	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpCharacterTarget.lock();

		std::shared_ptr<Enemy> spThis = m_wpThis.lock();

		if (spThis == nullptr)
		{
			return;
		}
		m_nowAction->Update(m_wpThis, spTarget);

	}

	Move(m_gravity, Math::Vector3::Down, KdCollider::TypeGround, false, false, true, false);
	m_gravity += m_gravityPow * KdFPSController::GetInstance().GetDeltaTime();

	UpdateCollision();

	auto translation = m_mWorld.Translation();
	Application::Instance().m_log.AddLog("HP%0.f\n", m_hp);
	Application::Instance().m_log.AddLog("pos x:%.2f,y:%.2f,z:%.2f\n", translation.x, translation.y, translation.z);


}

void Enemy::PostUpdate()
{


	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpCharacterTarget.lock();


		std::shared_ptr<Enemy> spThis = m_wpThis.lock();

		if (spThis == nullptr)
		{
			return;
		}
		m_nowAction->PostUpdate(m_wpThis, spTarget);
	}

	m_pDebugWire->AddDebugBox(m_mWorld, { 3,5,3 }, {}, true, { 1,0,0,1 });

}


void Enemy::DrawLit()
{
	if (!m_spModelWork) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld);
}


void Enemy::OnHit()
{
	if (m_hp <= 0)
	{
		ChangeActionState(std::make_shared<Destoroy>());
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

void Enemy::UpdateRotate(const Math::Vector3& srcMoveVec)
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



void Enemy::UpdateCollision()
{
	KdCollider::SphereInfo sphereInfo;
	sphereInfo.m_sphere.Center = GetPos() + Math::Vector3(0, 1.8f, 0);
	sphereInfo.m_sphere.Radius = 1.6f;
	sphereInfo.m_type = KdCollider::TypeGround;

	Math::Vector3 pos;
	if (SphereCast(sphereInfo.m_sphere.Center, sphereInfo.m_sphere.Radius, KdCollider::TypeGround, pos)) {
		SetPos(pos);
	}
	float dist = 0;

	DirectX::BoundingOrientedBox box;

	box.Center = GetPos() + Math::Vector3(0.0f, 6.0f, 0.0f);
	box.Extents = { 2.0f,5.0f,2.0f };
	UINT type = KdCollider::TypeDamage;
	KdCollider::BoxInfo boxInfo(type, box);

	auto translation = m_mWorld.Translation();

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (obj->Intersects(boxInfo, nullptr))
		{
			if (obj->GetTag() == tPlayerAttack)
			{
				obj->OnHit();
				m_hitDir = GetPos() - obj->GetMatrix().Translation();
				m_hitDir.Normalize();
				HitDamage(obj->GetParameter());
 				m_parameter = obj->GetParameter();
				OnHit();
			}
		}
	}
}

const Math::Matrix& Enemy::UpdateMatrix()
{
	auto pos = m_mWorld.Translation();
	Math::Matrix _rotation = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rot.y));
	m_mWorld = m_scale * _rotation * Math::Matrix::CreateTranslation(pos);
	return m_mWorld;
}

bool Enemy::Search(bool areaOnly)
{
	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = m_mWorld.Translation() + m_correction;
	sphere.m_sphere.Radius = m_radius;
	sphere.m_type = KdCollider::TypeDamage;

	std::list< KdCollider::CollisionResult> retList;
	std::list<std::shared_ptr<KdGameObject>> objList;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (obj->GetTag() == tPlayer || obj->GetTag() == tPlayerAttack)
		{
			if (obj->Intersects(sphere, &retList))
			{
				objList.push_back(obj);
			}
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
			obj = *it;

			
			{
				hitPos = ret.m_hitPos;
				overRap = ret.m_overlapDistance;
				isHit = true;

			}
		}
		// 増加
		it++;
	}

	// 索敵のみ
	if (areaOnly == true)
	{
		if (obj != nullptr)
		{
			if (obj->GetTag() == tPlayer)
			{
				m_wpTarget = obj;
				return true;
			}
		}
	}
	
	if (isHit)
	{
		// 視界内にいるかどうかの判定
		bool flg = SearchDetect(hitPos, m_mWorld, m_viewAngle);

		if (flg)
		{
			if (obj->GetTag() == tPlayer)
			{
				m_wpTarget = obj;
			}

			if (obj->GetTag() == tPlayerAttack)
			{
				m_isBullet = true;
			}
		}
	}

	return false;
}



void Enemy::Editor_ImGui()
{

	CharacterBase::Editor_ImGui();

	ImGui::DragFloat2("distance", &m_dist.x, 0.1f);
	ImGui::DragFloat("SerchRadius", &m_radius, 0.1f);
}


void Enemy::ChangeActionState(std::shared_ptr<ActionStateBase> nextAction)
{
	if (nextAction == nullptr) { return; }

	auto spTarget = m_wpTarget.lock();

	//if (spTarget == nullptr) { return; }

	if (m_nowAction) {
		m_nowAction->Exit(m_wpThis, spTarget);
	}
	m_prevAction = m_nowAction;
	m_nowAction = nextAction;
	m_nowAction->Enter(m_wpThis, spTarget);

}

void Enemy::ActionStateBase::CreateEffect(std::weak_ptr<Enemy>& owner,const std::string& effectName, const std::string& nodeName)
{
	auto spOwner = owner.lock();
	KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode(nodeName);
	if (pNode)
	{
		std::shared_ptr<Effect> effect = std::make_shared<Effect>();
		effect->name = effectName;
		effect->pNodeMat = pNode->m_worldTransform;
		effect->wpEffect = KdEffekseerManager::GetInstance().Play(effectName, pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(),1.0f,3.0f);
		m_spEffects.push_back(effect);
	}
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

UINT Enemy::ActionStateBase::Serch(const Math::Vector3& nowVec, const Math::Vector3& targetVec)
{

	UINT side = 0;

	//内積を使って回転する角度を求める
	float d = nowVec.Dot(targetVec);
	//dの中にはコサインΘが入っている

	//角度求める
	float ang = DirectX::XMConvertToDegrees(acos(d));

	if (ang >= 0.1f)
	{

		Math::Vector3 c = targetVec.Cross(nowVec);

		if (c.y >= 0)
		{
			side = ActionStateBase::Right;
		}
		else
		{
			//左回転
			side = ActionStateBase::Left;
		}
	}
	else if (ang > 100)
	{
		side = ActionStateBase::Back;
	}
	else
	{
		side = Enemy::ActionStateBase::Front;
	}

	return side;
}



void Enemy::ActionStateBase::ChangeStateWithPrev(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();

	if (spTarget == nullptr) { return; }

	auto nowVec = spOwner->m_mWorld.Backward();
	nowVec.Normalize();

	auto targetVec = spTarget->GetMatrix().Backward();
	targetVec.Normalize();

	UINT side = Serch(nowVec, targetVec);

	auto distance = spTarget->GetPos() - spOwner->GetPos();

	auto target = spOwner->GetCharacterTarget().lock();

	if (target)
	{
		if (target->IsDestroy())
		{
			spOwner->ChangeActionState(std::make_shared<Stand>());
			return;
		}
	}


	if (side == ActionStateBase::TargetSide::Left)
	{
		if (spOwner->GetPrevState() == tRotateLeft)
		{
			spOwner->ChangeActionState(std::make_shared<AttackLeft>());
			return;
		}

		if (spOwner->GetPrevState() == tMoveBack)
		{
			spOwner->ChangeActionState(std::make_shared<AttackBack>());
			return;
		}

		if (spOwner->GetPrevState() == tRotateRight)
		{
			spOwner->ChangeActionState(std::make_shared<Boost>());

			auto vec = distance;
			vec.Normalize();
			spOwner->BoostRotate(vec);
			spOwner->UpdateMatrix();

			spOwner->m_nowAction->SetParam(100.0f, spOwner->m_mWorld.Right());
			return;
		}

		if (spOwner->GetPrevState() == tBoost) {
			if (spOwner->m_rand.GetInt(1, 2) == 1)
			{
				spOwner->ChangeActionState(std::make_shared<AttackBack>());
				return;
			}
			else {
				spOwner->ChangeActionState(std::make_shared<AttackRight>());
				return;
			}
		}

		if (spOwner->GetPrevState() == tBoostStop)
		{
			if (spOwner->m_rand.GetInt(1, 2) == 1)
			{
				spOwner->ChangeActionState(std::make_shared<MoveRightRotate>());
				return;
			}
			else {
				spOwner->ChangeActionState(std::make_shared<MoveLeftRotate>());
				return;
			}
		}

	}

	else if (side == ActionStateBase::TargetSide::Right)
	{
		if (spOwner->GetPrevState() == tRotateRight) {
			spOwner->ChangeActionState(std::make_shared<AttackRight>());
			return;
		}

		if (spOwner->GetPrevState() == tRotateLeft)
		{
			spOwner->ChangeActionState(std::make_shared<Boost>());

			auto vec = distance;
			vec.Normalize();
			spOwner->BoostRotate(vec);
			spOwner->UpdateMatrix();

			spOwner->m_nowAction->SetParam(100.0f, spOwner->m_mWorld.Left());
			return;
		}

		if (spOwner->GetPrevState() == tBoost) {
			spOwner->ChangeActionState(std::make_shared<AttackLeft>());
			return;
		}

		if (spOwner->GetPrevState() == tBoostStop)
		{
			if (spOwner->m_rand.GetInt(1, 2) == 1)
			{
				spOwner->ChangeActionState(std::make_shared<MoveRightRotate>());
				return;
			}
			else {
				spOwner->ChangeActionState(std::make_shared<MoveLeftRotate>());
				return;
			}
		}


	}

	else if (side == ActionStateBase::TargetSide::Back)
	{
		if (spOwner->GetPrevState() == tBoostStop) {
			spOwner->ChangeActionState(std::make_shared<AttackBack>());
			return;
		}

		if (spOwner->GetPrevState() == tRotateLeft)
		{
			spOwner->ChangeActionState(std::make_shared<Boost>());
			spOwner->m_nowAction->SetParam(100.0f, spOwner->m_mWorld.Right());
			return;
		}

		if (spOwner->GetPrevState() == tRotateRight) {
			spOwner->ChangeActionState(std::make_shared<AttackRight>());
			return;
		}

		if (spOwner->GetPrevState() == tMoveForward)
		{
			spOwner->ChangeActionState(std::make_shared<Boost>());
			auto vec = distance;
			vec.Normalize();
			spOwner->BoostRotate(vec);
			spOwner->UpdateMatrix();
			spOwner->m_nowAction->SetParam(100.0f, spOwner->m_mWorld.Forward());
			return;
		}

		if (spOwner->GetPrevState() == tBoost) {
			if (spOwner->m_rand.GetInt(1, 2) == 1)
			{
				spOwner->ChangeActionState(std::make_shared<AttackStand>());
				return;
			}
			else {
				spOwner->ChangeActionState(std::make_shared<AttackBack>());
				return;
			}
		}

		if (spOwner->GetPrevState() == tBoostStop)
		{
			spOwner->ChangeActionState(std::make_shared<MoveForward>());
			return;
		}
	}

	else if (side == ActionStateBase::TargetSide::Front)
	{
		if (spOwner->GetPrevState() == tMoveForward)
		{
			spOwner->ChangeActionState(std::make_shared<Boost>());
			auto vec = distance;
			vec.Normalize();
			spOwner->BoostRotate(vec);
			spOwner->UpdateMatrix();
			spOwner->m_nowAction->SetParam(100.0f, spOwner->m_mWorld.Forward());
			return;
		}

		if (spOwner->GetPrevState() == tBoost) {
			if (spOwner->m_rand.GetInt(1, 2) == 1)
			{
				spOwner->ChangeActionState(std::make_shared<AttackStand>());
				return;
			}
			else {
				spOwner->ChangeActionState(std::make_shared<AttackBack>());
				return;
			}
		}

		if (spOwner->GetPrevState() == tBoostStop)
		{
			spOwner->ChangeActionState(std::make_shared<AttackForWard>());
			return;
		}
	}

	if (spOwner->GetPrevState() == tMoveBack)
	{
		spOwner->ChangeActionState(std::make_shared<AttackBack>());
		return;

	}

	if (spOwner->GetPrevState() == tStandAttack)
	{
		spOwner->ChangeActionState(std::make_shared<MoveBack>());
		return;
	}


	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}



}

void Enemy::ActionStateBase::SetParam(float speed, const Math::Vector3& direct)
{
	m_speed = speed;
	m_direct = direct;
}

void Enemy::ActionStateBase::ChangeStateWithDistance(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();

	auto spTarget = spObj.lock();

	if (spTarget == nullptr) { return; }

	auto distance = spTarget->GetPos() - spOwner->GetPos();
	auto length = distance.Length();
	auto halfOnwenrLength = (spOwner->m_dist.y - spOwner->m_dist.x) / 2.0f;

	auto target = spOwner->GetCharacterTarget().lock();

	if (target)
	{
		if (target->IsDestroy())
		{
			spOwner->ChangeActionState(std::make_shared<Stand>());
			return;
		}
	}


	// プレイヤーと自身との差がどれくらいか
	if (length > spOwner->m_dist.y)
	{
		spOwner->ChangeActionState(std::make_shared<Boost>());
		auto vec = distance;
		vec.Normalize();
		spOwner->BoostRotate(vec);
		spOwner->UpdateMatrix();

		auto direction = spOwner->m_mWorld.Backward();

		spOwner->m_nowAction->SetParam(150, direction);
		return;
	}
	else if (length > halfOnwenrLength)
	{
		spOwner->ChangeActionState(std::make_shared<MoveForward>());
		return;
	}
	else if (length < halfOnwenrLength)
	{
		if (spOwner->GetPrevState() == tStandAttack)
		{
			spOwner->ChangeActionState(std::make_shared<MoveLeftRotate>());
			return;
		}
		else
		{
			spOwner->ChangeActionState(std::make_shared<AttackStand>());
			return;
		}
	}
	else if (length > halfOnwenrLength / 2)
	{
		spOwner->ChangeActionState(std::make_shared<MoveBack>());
		return;
	}
	else {

		spOwner->ChangeActionState(std::make_shared<Boost>());

		auto vec = distance;
		vec.Normalize();
		spOwner->BoostRotate(vec);
		spOwner->UpdateMatrix();

		auto direction = spOwner->m_mWorld.Forward();

		spOwner->m_nowAction->SetParam(100, direction);
		return;
	}

}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//待機状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::Start::Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandUp"), 100.0f);

	m_type = tStart;
}

void Enemy::Start::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{

	Application::Instance().m_log.AddLog("EnemynowState: Start\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	bool isFind = spOwner->SearchPlayer();

	if (isFind) {
		spOwner->ChangeActionState(std::make_shared<StandUp>());
		return;
	}
}

void Enemy::Start::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 0.0f);
}

void Enemy::Start::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//立ち上がり
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::StandUp::Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandUp"), 100.0f, false);

	m_type = tStandUp;
}

void Enemy::StandUp::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
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

void Enemy::StandUp::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 14.0f);
}

void Enemy::StandUp::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//スタンド状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::Stand::Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Stand"), 3.0f);

	m_type = tStand;
}

void Enemy::Stand::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{

	Application::Instance().m_log.AddLog("EnemynowState: Stand\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();
	auto spOBj = spObj.lock();
	auto difference = spOBj->GetMatrix().Translation() - spOwner->m_mWorld.Translation();


	ChangeStateWithDistance(owner, spOBj);

}

void Enemy::Stand::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}


void Enemy::Stand::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::Boost::Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Boost"), 60.0f, false);


	CreateEffect(owner, "ThrusterE.efkefc", "CBP");

	m_type = tBoost;
}

void Enemy::Boost::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: Boost\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	
	if (spOwner->m_spAnimator->IsAnimationEnd() == true)
	{
		spOwner->ChangeActionState(std::make_shared<BoostStop>());
		spOwner->m_nowAction->SetParam(m_speed / 2.0f, m_direct);
		return;
	}


	spOwner->Move(m_speed, m_direct, KdCollider::TypeGround);

	//エフェクト
	EffectUpdate(owner);

}

void Enemy::Boost::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);

}

void Enemy::Boost::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト停止
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::BoostStop::Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostStop"), 60.0f, false);

	m_direct = spOwner->m_mWorld.Backward();
	m_direct.Normalize();

	m_speed = 100.0f;

	m_type = tBoostStop;
}

void Enemy::BoostStop::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: BoostStop\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	spOwner->Move(m_speed, m_direct, KdCollider::TypeGround);


	if (spOwner->m_spAnimator->IsAnimationEnd() == true)
	{
		ChangeStateWithPrev(owner, spObj);
		return;
	}

}

void Enemy::BoostStop::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);
}

void Enemy::BoostStop::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
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
void Enemy::MoveForward::Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostDush"), 10.0f);

	m_durationState = 2.0f;

	m_speed = 90.0f;

	m_type = tMoveForward;

	CreateEffect(owner, "ThrusterE.efkefc", "CBP");
}

void Enemy::MoveForward::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: MoveForward\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	if (spTarget == nullptr) { return; }

	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	auto vect = mat.Backward();
	

	m_direct = mat.Backward();

	spOwner->Move(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithPrev(owner, spObj);
		return;
	}

	EffectUpdate(owner);
}

void Enemy::MoveForward::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 30.0f);
}

void Enemy::MoveForward::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 後退
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::MoveBack::Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostBack"), 4.0f);

	m_durationState = 1.0f;

	m_speed = 70.0f;

	m_type = tMoveBack;

	CreateEffect(owner, "ThrusterE.efkefc", "CBP");

}

void Enemy::MoveBack::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: MoveBack\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();

	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	auto vect = mat.Forward();
	
	m_direct = mat.Forward();

	spOwner->Move(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithPrev(owner, spObj);
		return;
	}

	EffectUpdate(owner);

}

void Enemy::MoveBack::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::MoveBack::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 右回り込み
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::MoveRightRotate::Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostRight"), 3.0f);

	m_speed = 80.0f;

	m_durationState = 1.0f;

	m_type = tRotateRight;

	CreateEffect(owner, "ThrusterE.efkefc", "CBP");
}

void Enemy::MoveRightRotate::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: MoveRightRotate\n");

	auto spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	m_direct = mat.Right();

	spOwner->Move(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithPrev(owner, spObj);
		return;
	}

	EffectUpdate(owner);

}
void Enemy::MoveRightRotate::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::MoveRightRotate::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	EffectExit();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 左回り込み
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::MoveLeftRotate::Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostLeft"), 3.0f);

	m_speed = 70.0f;

	m_durationState = 1.0f;

	m_type = tRotateLeft;

	CreateEffect(owner, "ThrusterE.efkefc", "CBP");

}

void Enemy::MoveLeftRotate::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: MoveLeftRotate\n");

	auto spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();
	
	vec.Normalize();

	spOwner->UpdateRotate(vec);
	spOwner->UpdateMatrix();

	auto& mat = spOwner->UpdateMatrix();

	m_direct = mat.Left();

	spOwner->Move(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithPrev(owner, spObj);
		return;
	}

	EffectUpdate(owner);
}

void Enemy::MoveLeftRotate::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::MoveLeftRotate::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 直立攻撃状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::AttackStand::Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandAttack"), 3.0f);

	spOwner->ChangeEnableRightAttack(true);
	spOwner->ChangeEnableLeftAttack(true);

	m_durationState = 0.7f;

	m_type = tStandAttack;

	m_speed = 0.0f;


	auto alert = std::make_shared<Alert>();
	auto pos = CameraManager::Instance().GetLocalDirectionTo(spOwner->GetMatrix().Translation());
	alert->CalcPos(pos);
	alert->Init();
	SceneManager::Instance().AddObject(alert);

}

void Enemy::AttackStand::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemyowState: StandAttack\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();
	vec.Normalize();
	
	spOwner->UpdateRotate(vec);
	spOwner->UpdateMatrix();

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithDistance(owner, spObj);
		return;
	}
}

void Enemy::AttackStand::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::AttackStand::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	spOwner->ChangeEnableRightAttack(false);
	spOwner->ChangeEnableLeftAttack(false);
}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 前進攻撃状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::AttackForWard::Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("FrontAttack"), 3.0f);

	m_speed = 90.0f;

	m_type = tFrontAttack;

	m_durationState = 0.7f;

	spOwner->ChangeEnableRightAttack(true);
	spOwner->ChangeEnableLeftAttack(true);

	auto alert = std::make_shared<Alert>();
	auto pos = CameraManager::Instance().GetLocalDirectionTo(spOwner->GetMatrix().Translation());
	alert->CalcPos(pos);
	alert->Init();
	SceneManager::Instance().AddObject(alert);

	CreateEffect(owner, "ThrusterE.efkefc", "CBP");

}


void Enemy::AttackForWard::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemyowState: FrontAttack\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	auto vect = mat.Backward();
	Application::Instance().m_log.AddLog("drect x:%.2f,y:%.2f,z:%.2f\n", vec.x, vec.y, vec.z);

	m_direct = mat.Backward();

	spOwner->Move(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithDistance(owner, spObj);
		return;
	}

	EffectUpdate(owner);
}

void Enemy::AttackForWard::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::AttackForWard::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	spOwner->ChangeEnableRightAttack(false);
	spOwner->ChangeEnableLeftAttack(false);

	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 後退攻撃
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::AttackBack::Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BackAttack"), 3.0f);

	m_speed = 60.0f;

	m_type = tBackAttack;

	m_durationState = 0.5f;

	spOwner->ChangeEnableRightAttack(true);
	spOwner->ChangeEnableLeftAttack(true);


	auto alert = std::make_shared<Alert>();
	auto pos = CameraManager::Instance().GetLocalDirectionTo(spOwner->GetMatrix().Translation());
	alert->CalcPos(pos);
	alert->Init();
	SceneManager::Instance().AddObject(alert);

	CreateEffect(owner, "ThrusterE.efkefc", "CBP");


}

void Enemy::AttackBack::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemyowState: Attack\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	auto vect = mat.Forward();

	m_direct = mat.Forward();

	spOwner->Move(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithDistance(owner, spObj);
		return;
	}

	EffectUpdate(owner);
}

void Enemy::AttackBack::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::AttackBack::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	spOwner->ChangeEnableRightAttack(false);
	spOwner->ChangeEnableLeftAttack(false);

	EffectExit();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 左回り攻撃
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::AttackLeft::Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("LeftAttack"), 3.0f);

	m_durationState = 0.63f;

	m_speed = 70.0f;

	spOwner->ChangeEnableRightAttack(true);
	spOwner->ChangeEnableLeftAttack(true);

	auto alert = std::make_shared<Alert>();
	auto pos = CameraManager::Instance().GetLocalDirectionTo(spOwner->GetMatrix().Translation());
	alert->CalcPos(pos);
	alert->Init();
	SceneManager::Instance().AddObject(alert);

	CreateEffect(owner, "ThrusterE.efkefc", "CBP");

}

void Enemy::AttackLeft::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemyowState: Attack\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	m_direct = mat.Left();

	spOwner->Move(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithDistance(owner, spObj);
		return;
	}

	EffectUpdate(owner);
}

void Enemy::AttackLeft::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::AttackLeft::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	spOwner->ChangeEnableRightAttack(false);
	spOwner->ChangeEnableLeftAttack(false);

	EffectExit();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 右回り攻撃
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::AttackRight::Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("RightAttack"), 3.0f);

	m_durationState = 0.4f;

	m_speed = 70.0f;

	spOwner->ChangeEnableRightAttack(true);
	spOwner->ChangeEnableLeftAttack(true);


	auto alert = std::make_shared<Alert>();
	auto pos = CameraManager::Instance().GetLocalDirectionTo(spOwner->GetMatrix().Translation());
	alert->CalcPos(pos);
	alert->Init();
	SceneManager::Instance().AddObject(alert);

	CreateEffect(owner, "ThrusterE.efkefc", "CBP");

}

void Enemy::AttackRight::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemyowState: Attack\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();
	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	m_direct = mat.Right();

	spOwner->Move(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithDistance(owner, spObj);
		return;
	}
}

void Enemy::AttackRight::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
}

void Enemy::AttackRight::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	EffectExit();

	spOwner->ChangeEnableRightAttack(false);
	spOwner->ChangeEnableLeftAttack(false);
}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// ダメージ状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::Hited::Enter(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	m_speed = 40.0f;

	m_direct = spOwner->GetHitDir();

	if (m_direct.Length() == 1e-6f)
	{
		m_direct = spOwner->GetMatrix().Forward();
	}

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Hited"), 20.0f, false);
}

void Enemy::Hited::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: Hited\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	spOwner->Move(m_speed, m_direct, KdCollider::TypeGround, false, false);

	//攻撃をくらったとき
	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}
}

void Enemy::Hited::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);
}

void Enemy::Hited::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 死亡状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Enemy::Destoroy::Enter(std::weak_ptr<Enemy>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Destroyed"), 5.0f, false);

	spOwner->m_isDestroy = true;

	KdAudioManager::Instance().Play("Asset/Sounds/Sound/down_enemy.wav")->SetVolume(0.3f);

}

void Enemy::Destoroy::Update(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("EnemynowState: Destory\n");

	std::shared_ptr<Enemy> spOwner = owner.lock();

	//死亡時
	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->Burn();
		spOwner->m_isExpired = true;
		return;
	}

}

void Enemy::Destoroy::PostUpdate(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	if (spOwner->m_spAnimator->GetProgress() <= 0.3)
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
	else {
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);
	}
}

void Enemy::Destoroy::Exit(std::weak_ptr<Enemy>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

}


