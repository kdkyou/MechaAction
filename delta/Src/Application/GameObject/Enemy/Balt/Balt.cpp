#include"Balt.h"


#include"../../../main.h"

#include "../../../Scene/SceneManager.h"

#include"../../Character/CharacterBase.h"

#include "../../Camera/CameraManager.h"
#include "../../UI/Alert/Alert.h"

// 攻撃はleftでバルカン
// rightでブレード
// shoulderでミサイル

void Balt::Init()
{
	m_name = "Balt";

	if (!m_spModelWork)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData("Asset/Models/Leg/BaltLeg.gltf");
		// 初期のアニメーションをセットする
	}

	m_spAnimator = std::make_shared<KdAnimator>();
	m_spAnimator->SetAnimation(m_spModelWork->GetData()->GetAnimation("StandUp"), 1.0f, false);

	m_spMrkModel = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Marker/Enemy.gltf");


	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();

		DirectX::BoundingOrientedBox box;

		box.Center = m_mWorld.Translation();
		box.Extents = { 3,5,3 };

		//	m_pCollider->RegisterCollisionShape("Balt", box, KdCollider::TypeDamage);
		m_pCollider->RegisterCollisionShape("Balt", m_spModelWork, KdCollider::TypeDamage);
	}


	m_correction = { 0.0f,5.0f,0.0f };
	m_correctionMat = Math::Matrix::CreateTranslation({ 0.0f,5.0f,0.0f });

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	//初期状態を「待機状態」へ設定
	ChangeActionState(std::make_shared<Start>());

	m_dist = { 90.0f,400.0f };

	m_clampSize = 20.0f;

	m_burnPath = "Asset/Textures/GameObject/Burn.png";

}

void Balt::Update()
{
	Application::Instance().m_log.AddLog("Balt\n");
	//各種「状態に応じた」更新処理を実行する
	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpCharacterTarget.lock();

		std::shared_ptr<Balt> spThis = m_wpThis.lock();

		if (spThis == nullptr)
		{
			return;
		}
		m_nowAction->Update(m_wpThis, spTarget);

	}

	UpdateCollision();

	SetWeapon();

	auto translation = m_mWorld.Translation();
	Application::Instance().m_log.AddLog("HP%0.f\n", m_hp);
	Application::Instance().m_log.AddLog("pos x:%.2f,y:%.2f,z:%.2f\n", translation.x, translation.y, translation.z);

}

void Balt::PostUpdate()
{


	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpCharacterTarget.lock();


		std::shared_ptr<Balt> spThis = m_wpThis.lock();

		if (spThis == nullptr)
		{
			return;
		}
		m_nowAction->PostUpdate(m_wpThis, spTarget);
	}

	if (m_spModelWork) {
		m_spModelWork->CalcNodeMatrices();
	}

	CharacterBase::PostUpdate();
	auto pos = m_mMarker.Translation();
	Application::Instance().m_log.AddLog("Balt:X.%.1fY.%.1fZ.%.1f\n", pos.x, pos.y, pos.z);

}


void Balt::DrawLit()
{
	if (!m_spModelWork) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld, m_modelColor, m_emissiveColor);
}


void Balt::OnHit()
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

void Balt::UpdateRotate(const Math::Vector3& srcMoveVec)
{
	auto nowVec = GetMatrix().Backward();

	//内積を使って回転する角度を求める
	float d = nowVec.Dot(srcMoveVec);
	//dの中にはコサインΘが入っている

	//角度求める
	float ang = DirectX::XMConvertToDegrees(acos(d));

	if (ang >= 5.0f)
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



void Balt::UpdateCollision()
{
	KdCollider::SphereInfo sphereInfo;
	sphereInfo.m_sphere.Center = GetPos() + Math::Vector3(0, 1.8f, 0);
	sphereInfo.m_sphere.Radius = 1.6f;
	sphereInfo.m_type = KdCollider::TypeGround;

	Math::Vector3 pos;
	if (SphereCast(sphereInfo.m_sphere.Center, sphereInfo.m_sphere.Radius, KdCollider::TypeGround, pos)) {
		SetPos(pos);
	}

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

	m_pDebugWire->AddDebugBox(m_mWorld, { 3,5,3 }, m_correction);
}

const Math::Matrix& Balt::UpdateMatrix()
{
	auto pos = m_mWorld.Translation();
	Math::Matrix _rotation = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rot.y));
	m_mWorld = m_mScale * _rotation * Math::Matrix::CreateTranslation(pos);
	return m_mWorld;
}

bool Balt::Search(bool areaOnly)
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

void Balt::AttackOff()
{
	ChangeEnableAttack(false);
	ChangeEnableLeftAttack(false);
	ChangeEnableRightAttack(false);
	ChangeEnableLeftShoulderAttack(false);
	ChangeEnableRightShoulderAttack(false);
}



void Balt::Editor_ImGui()
{

	CharacterBase::Editor_ImGui();

	ImGui::DragFloat2("distance", &m_dist.x, 0.1f);
	ImGui::DragFloat("SerchRadius", &m_radius, 0.1f);
	ImGui::DragFloat((const char*)u8"視野角", &m_viewAngle, 0.1f, 0.0f, 180.0f);
}

void Balt::Deserialize(const nlohmann::json& jsonObj)
{
	CharacterBase::Deserialize(jsonObj);
}

void Balt::Serialize(nlohmann::json& outJson) const
{
	CharacterBase::Serialize(outJson);
}


void Balt::ChangeActionState(std::shared_ptr<ActionStateBase> nextAction)
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

void Balt::ActionStateBase::CreateEffect(std::weak_ptr<Balt>& owner, const std::string& effectName, const std::string& nodeName)
{
	auto spOwner = owner.lock();
	if (spOwner->m_spModelWork)
	{
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
}

void Balt::ActionStateBase::EffectUpdate(std::weak_ptr<Balt>& owner)
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

void Balt::ActionStateBase::EffectExit()
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

UINT Balt::ActionStateBase::Serch(const Math::Vector3& nowVec, const Math::Vector3& targetVec)
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
		side = Balt::ActionStateBase::Front;
	}

	return side;
}



void Balt::ActionStateBase::ChangeStateWithPrev(std::weak_ptr<Balt>& owner, const std::weak_ptr<KdGameObject>& spObj)
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
			int i = spOwner->m_rand.GetInt(1, 5);
			if (i == 1) {
			spOwner->ChangeActionState(std::make_shared<AttackLeft>());
			return;
			}
			else if (i == 2) {
			spOwner->ChangeActionState(std::make_shared<AttackRight>());
			return;
			}
			else if (i == 3) {
			spOwner->ChangeActionState(std::make_shared<MoveRightRotate>());
			return;
			}
			else if (i == 4) {
			spOwner->ChangeActionState(std::make_shared<AttackBack>());
			return;
			}
		}

		if (spOwner->GetPrevState() == tMoveBack)
		{
			spOwner->ChangeActionState(std::make_shared<AttackBack>());
			return;
		}

		if (spOwner->GetPrevState() == tMoveForward)
		{
			spOwner->ChangeActionState(std::make_shared<AttackForWard>());
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
				spOwner->ChangeActionState(std::make_shared<MoveForward>());
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
			spOwner->ChangeActionState(std::make_shared<AttackForWard>());
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

void Balt::ActionStateBase::SetParam(float speed, const Math::Vector3& direct)
{
	m_speed = speed;
	m_direct = direct;
}

void Balt::ActionStateBase::ChangeStateWithDistance(std::weak_ptr<Balt>& owner, const std::weak_ptr<KdGameObject>& spObj)
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
void Balt::Start::Enter(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	if (!spOwner) { return; }
	auto spTarget = spObj.lock();

	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandUp"), 100.0f);
	}
	m_type = tStart;
}

void Balt::Start::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{

	Application::Instance().m_log.AddLog("BaltnowState: Start\n");

	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	bool isFind = spOwner->SearchPlayer();

	if (isFind) {
		spOwner->ChangeActionState(std::make_shared<StandUp>());
		return;
	}
}

void Balt::Start::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 0.0f);
	}
}

void Balt::Start::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//立ち上がり
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::StandUp::Enter(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandUp"), 100.0f, false);
	}

	m_type = tStandUp;
}

void Balt::StandUp::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	// 立ち上がるだけ
	Application::Instance().m_log.AddLog("BaltnowState: StandUp\n");

	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }


	if (spOwner->m_spAnimator->GetProgress() >= 1.0f)
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}

}

void Balt::StandUp::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
	}
}

void Balt::StandUp::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();

	if (!spOwner) { return; }

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//スタンド状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::Stand::Enter(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	if (!spOwner) { return; }
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Stand"), 3.0f);
	}

	m_type = tStand;
}

void Balt::Stand::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{

	Application::Instance().m_log.AddLog("BaltnowState: Stand\n");

	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spOBj = spObj.lock();
	if (!spOwner) { return; }


	auto difference = spOBj->GetMatrix().Translation() - spOwner->m_mWorld.Translation();


	ChangeStateWithDistance(owner, spOBj);

}

void Balt::Stand::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	if (!spOwner) { return; }

	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
	}
}


void Balt::Stand::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	if (!spOwner) { return; }

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::Boost::Enter(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Boost"), 60.0f, false);
	}
	m_speed = 170.0f;


	m_type = tBoost;
}

void Balt::Boost::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltnowState: Boost\n");

	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();


	if (spOwner->m_spAnimator->IsAnimationEnd() == true)
	{
		spOwner->ChangeActionState(std::make_shared<BoostStop>());
		spOwner->m_nowAction->SetParam(m_speed / 2.0f, m_direct);
		return;
	}


	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround);

	//エフェクト
	EffectUpdate(owner);

}

void Balt::Boost::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);
	}
}

void Balt::Boost::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト停止
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::BoostStop::Enter(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	//spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostStop"), 60.0f, false);

	m_direct = spOwner->m_mWorld.Backward();
	m_direct.Normalize();

	m_speed = 100.0f;

	m_type = tBoostStop;

	m_durationState = 0.3f;
}

void Balt::BoostStop::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltnowState: BoostStop\n");

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	// イージング
	auto progress = spOwner->m_spAnimator->GetProgress() * DirectX::XM_PI;

	auto easeSpeed = m_speed - (m_speed * progress);

	spOwner->MoveSwept(easeSpeed, m_direct, KdCollider::TypeGround);



	if (m_durationState < 0.0f)
	{
		ChangeStateWithPrev(owner, spObj);
		return;
	}

}

void Balt::BoostStop::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);
	}
}

void Balt::BoostStop::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	//エフェクト
	EffectExit();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 正面移動
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::MoveForward::Enter(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Move"), 10.0f, false);
	}
	m_durationState = 1.0f;

	m_speed = 90.0f;

	m_type = tMoveForward;

	//	CreateEffect(owner, "ThrusterE.efkefc", "CBP");
}

void Balt::MoveForward::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltnowState: MoveForward\n");

	std::shared_ptr<Balt> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	if (spTarget == nullptr) { return; }

	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	m_direct = mat.Backward();

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithPrev(owner, spObj);
		return;
	}

	EffectUpdate(owner);
}

void Balt::MoveForward::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 30.0f);
	}
}

void Balt::MoveForward::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 後退
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::MoveBack::Enter(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostBack"), 4.0f);
	}
	m_durationState = 0.9f;

	m_speed = 70.0f;

	m_type = tMoveBack;

	//CreateEffect(owner, "ThrusterE.efkefc", "CBP");

}

void Balt::MoveBack::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltnowState: MoveBack\n");

	std::shared_ptr<Balt> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();

	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	auto vect = mat.Forward();

	m_direct = mat.Forward();

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithPrev(owner, spObj);
		return;
	}

	EffectUpdate(owner);

}

void Balt::MoveBack::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
	}
}

void Balt::MoveBack::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 右回り込み
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::MoveRightRotate::Enter(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostRight"), 3.0f);
	}

	m_speed = 80.0f;

	m_durationState = 1.0f;

	m_type = tRotateRight;

	//CreateEffect(owner, "ThrusterE.efkefc", "CBP");
}

void Balt::MoveRightRotate::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltnowState: MoveRightRotate\n");

	auto spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	m_direct = mat.Right();

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithPrev(owner, spObj);
		return;
	}

	EffectUpdate(owner);

}
void Balt::MoveRightRotate::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
	}
}

void Balt::MoveRightRotate::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	EffectExit();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 左回り込み
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::MoveLeftRotate::Enter(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostLeft"), 3.0f);
	}
	m_speed = 70.0f;

	m_durationState = 1.0f;

	m_type = tRotateLeft;

	//CreateEffect(owner, "ThrusterE.efkefc", "CBP");

}

void Balt::MoveLeftRotate::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltnowState: MoveLeftRotate\n");

	auto spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	spOwner->UpdateMatrix();

	auto& mat = spOwner->UpdateMatrix();

	m_direct = mat.Left();

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithPrev(owner, spObj);
		return;
	}

	EffectUpdate(owner);
}

void Balt::MoveLeftRotate::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
	}
}

void Balt::MoveLeftRotate::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 直立攻撃状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::AttackStand::Enter(std::weak_ptr<Balt>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Stand"), 3.0f);
	}

	spOwner->ChangeEnableLeftShoulderAttack(true);
	spOwner->ChangeEnableRightShoulderAttack(true);

	m_durationState = 0.7f;

	m_type = tStandAttack;

	m_speed = 0.0f;


	auto alert = std::make_shared<Alert>();
	auto pos = CameraManager::Instance().GetLocalDirectionTo(spOwner->GetMatrix().Translation());
	alert->CalcPos(pos);
	alert->Init();
	SceneManager::Instance().AddObject(alert);

}

void Balt::AttackStand::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltowState: StandAttack\n");

	std::shared_ptr<Balt> spOwner = owner.lock();

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

void Balt::AttackStand::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
	}
}

void Balt::AttackStand::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	spOwner->AttackOff();
}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 前進攻撃状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::AttackForWard::Enter(std::weak_ptr<Balt>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BladeAttack"), 3.0f, false);
	}

	m_speed = 90.0f;

	m_type = tFrontAttack;

	m_durationState = 0.7f;

	spOwner->ChangeEnableAttack(true);
	spOwner->ChangeEnableRightAttack(true);

	auto alert = std::make_shared<Alert>();
	auto pos = CameraManager::Instance().GetLocalDirectionTo(spOwner->GetMatrix().Translation());
	alert->CalcPos(pos);
	alert->Init();
	SceneManager::Instance().AddObject(alert);

}


void Balt::AttackForWard::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltowState: BladeAttack\n");

	std::shared_ptr<Balt> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	auto vect = mat.Backward();
	Application::Instance().m_log.AddLog("drect x:%.2f,y:%.2f,z:%.2f\n", vec.x, vec.y, vec.z);

	m_direct = mat.Backward();

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround, false, false);

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

		if (m_durationState <= 0.0f)
		{
			ChangeStateWithDistance(owner, spObj);
			return;
		}
	}

	EffectUpdate(owner);
}

void Balt::AttackForWard::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 6.0f);
	}
}

void Balt::AttackForWard::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	spOwner->AttackOff();

	EffectExit();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 後退攻撃
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::AttackBack::Enter(std::weak_ptr<Balt>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("AttackBack"), 3.0f);
	}
	m_speed = 60.0f;

	m_type = tBackAttack;

	m_durationState = 0.5f;

	spOwner->ChangeEnableLeftAttack(true);
	spOwner->ChangeEnableLeftShoulderAttack(true);
	spOwner->ChangeEnableRightShoulderAttack(true);

	auto alert = std::make_shared<Alert>();
	auto pos = CameraManager::Instance().GetLocalDirectionTo(spOwner->GetMatrix().Translation());
	alert->CalcPos(pos);
	alert->Init();
	SceneManager::Instance().AddObject(alert);

	//	CreateEffect(owner, "ThrusterE.efkefc", "CBP");


}

void Balt::AttackBack::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltowState: BackAttack\n");

	std::shared_ptr<Balt> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	auto vect = mat.Forward();

	m_direct = mat.Forward();

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithDistance(owner, spObj);
		return;
	}

	EffectUpdate(owner);
}

void Balt::AttackBack::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
	}
}

void Balt::AttackBack::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	spOwner->AttackOff();

	EffectExit();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 左回り攻撃
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::AttackLeft::Enter(std::weak_ptr<Balt>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("AttackLeft"), 3.0f);
	}

	m_durationState = 0.63f;

	m_speed = 70.0f;

	spOwner->ChangeEnableLeftAttack(true);


	auto alert = std::make_shared<Alert>();
	auto pos = CameraManager::Instance().GetLocalDirectionTo(spOwner->GetMatrix().Translation());
	alert->CalcPos(pos);
	alert->Init();
	SceneManager::Instance().AddObject(alert);

	CreateEffect(owner, "ThrusterE.efkefc", "CBP");

}

void Balt::AttackLeft::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltowState: LeftAttack\n");

	std::shared_ptr<Balt> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();

	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	m_direct = mat.Left();

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithDistance(owner, spObj);
		return;
	}

	EffectUpdate(owner);
}

void Balt::AttackLeft::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
	}
}

void Balt::AttackLeft::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	spOwner->AttackOff();

	EffectExit();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 右回り攻撃
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::AttackRight::Enter(std::weak_ptr<Balt>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("AttackRight"), 3.0f);
	}

	m_durationState = 0.4f;

	m_speed = 70.0f;

	spOwner->ChangeEnableLeftAttack(true);
	spOwner->ChangeEnableLeftShoulderAttack(true);


	auto alert = std::make_shared<Alert>();
	auto pos = CameraManager::Instance().GetLocalDirectionTo(spOwner->GetMatrix().Translation());
	alert->CalcPos(pos);
	alert->Init();
	SceneManager::Instance().AddObject(alert);

}

void Balt::AttackRight::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltowState: RightAttack\n");

	std::shared_ptr<Balt> spOwner = owner.lock();

	std::shared_ptr<KdGameObject> spTarget = spObj.lock();
	auto vec = spTarget->GetPos() - spOwner->GetMatrix().Translation();
	vec.Normalize();

	spOwner->UpdateRotate(vec);
	auto& mat = spOwner->UpdateMatrix();

	m_direct = mat.Right();

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround, false, false);

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_durationState <= 0.0f)
	{
		ChangeStateWithDistance(owner, spObj);
		return;
	}
}

void Balt::AttackRight::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
	}
}

void Balt::AttackRight::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	EffectExit();

	spOwner->AttackOff();
}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// ダメージ状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::Hited::Enter(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	m_speed = 40.0f;

	m_direct = spOwner->GetHitDir();

	if (m_direct.Length() == 1e-6f)
	{
		m_direct = spOwner->GetMatrix().Forward();
	}
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Hited"), 20.0f, false);
	}
}

void Balt::Hited::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltnowState: Hited\n");

	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround, false, false);

	//攻撃をくらったとき
	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}
}

void Balt::Hited::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);
	}
}

void Balt::Hited::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
// 死亡状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Balt::Destoroy::Enter(std::weak_ptr<Balt>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Destroyed"), 5.0f, false);
	}

	spOwner->m_isDestroy = true;

	auto& am = KdAudioManager::Instance();
	am.Play("Asset/Sounds/Sound/down_Enemy.wav")->SetVolume(am.GetSEVolume());

}

void Balt::Destoroy::Update(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("BaltnowState: Destory\n");

	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();

	//死亡時
	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->Burn();
		spOwner->m_isExpired = true;
		return;
	}

}

void Balt::Destoroy::PostUpdate(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (spOwner->m_spModelWork)
	{
		if (spOwner->m_spAnimator->GetProgress() <= 0.3)
			spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f);
		else {
			spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f);
		}
	}
}

void Balt::Destoroy::Exit(std::weak_ptr<Balt>& owner, const  std::weak_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Balt> spOwner = owner.lock();
	auto spTarget = spObj.lock();
}


