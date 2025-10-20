#include "FLAC.h" 

#include "../../../Scene/SceneManager.h"
#include "../../../main.h"


void FLAC::Init()
{
	m_name = "FLAC";

	m_hp = 5800;
	m_maxHp = m_hp;
	
	m_spMrkModel = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Marker/Balt.gltf");

	m_spAnimator = std::make_shared<KdAnimator>();

	/*auto rifle = std::make_shared<Rifle>();
	rifle->SetParent(m_wpThis.lock());
	rifle->SetAttackTrigger(WeaponBase::LeftHand);
	rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
	rifle->SetGunsParam("Asset/Models/Weapon/Gatring/Gatring.gltf", 0.2f, 2.0f, 0.0f, 1, 36, 250);
	rifle->SetAttachPath("LeftWeapon");
	rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", 5.0f, 56, 800, 200, 20.0f, 0.9f);
	rifle->SetBulletsTrailParam("Asset/Textures/GameObject/ClockHand.png", Math::Color(0.7f, 0.4f, 0.1f), 1.7f, 20);
	rifle->SetShotSoundPath("Asset/Sounds/SE/Weapon/Shot_Raifl.wav");
	SceneManager::Instance().AddObject(rifle);
	m_wpWeapons.push_back(rifle);

	rifle = std::make_shared<Rifle>();
	rifle->SetParent(m_wpThis.lock());
	rifle->SetAttackTrigger(WeaponBase::RightHand);
	rifle->SetTag(KdGameObject::tEnemyAttack);
	rifle->Init();
	rifle->SetGunsParam("Asset/Models/Weapon/Rifle/Rifle.gltf", 0.2f, 2.0f, 0.0f, 1, 36, 250);
	rifle->SetAttachPath("RightWeapon");
	rifle->SetBulletsParam("Asset/Models/Weapon/Bullet/Bullet-Live.gltf", 5.0f, 56, 800, 200, 20.0f, 0.9f);
	rifle->SetBulletsTrailParam("Asset/Textures/GameObject/ClockHand.png", Math::Color(0.7f, 0.4f, 0.1f), 1.7f, 20);
	rifle->SetShotSoundPath("Asset/Sounds/SE/Weapon/Shot_Raifl.wav");
	SceneManager::Instance().AddObject(rifle);
	m_wpWeapons.push_back(rifle);*/

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape("Enemy", m_spModelWork, KdCollider::TypeDamage);

	ChangeActionState(std::make_shared<Stand>());

	m_name = "FLAC";

	m_burnPath = "Asset/Textures/GameObject/Burn.png";
}

void FLAC::Update()
{
	Application::Instance().m_log.AddLog("FLAC\n");
	//各種「状態に応じた」更新処理を実行する
	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpCharacterTarget.lock();

		std::shared_ptr<FLAC> spThis = m_wpThis.lock();

		if (spThis == nullptr)
		{
			return;
		}
		m_nowAction->Update(m_wpThis, spTarget);

	}
	auto pos = m_mWorld.Translation();
	//auto flg = Gravity(pos, Math::Vector3::Down, m_gravity);
	//auto flg = Move(m_gravity, Math::Vector3::Down, KdCollider::TypeGround, false, false);
	auto flg = Move(m_gravity, Math::Vector3::Down, KdCollider::TypeGround, false, false, false, true);

	m_gravity += m_gravityPow * KdFPSController::GetInstance().GetDeltaTime();

	if (flg) {
		Application::Instance().m_log.AddLog("AnotherGround\n");
	}
	else {
		Application::Instance().m_log.AddLog("AnotherFallNow\n");
	}
	UpdateCollision();

	auto translation = m_mWorld.Translation();
	Application::Instance().m_log.AddLog("HP%0.f\n", m_hp);
	Application::Instance().m_log.AddLog("pos x:%.2f,y:%.2f,z:%.2f\n", translation.x, translation.y, translation.z);

}

void FLAC::PostUpdate()
{
	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpCharacterTarget.lock();


		std::shared_ptr<FLAC> spThis = m_wpThis.lock();

		if (spThis == nullptr)
		{
			return;
		}
		m_nowAction->PostUpdate(m_wpThis, spTarget);
	}

	if (m_spModelWork) {
		m_spModelWork->CalcNodeMatrices();
	}
}

void FLAC::DrawLit()
{
	if (!m_spModelWork) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld, m_modelColor, m_emissiveColor);

}

void FLAC::OnHit()
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

void FLAC::Editor_ImGui()
{
	CharacterBase::Editor_ImGui();
}

void FLAC::Deserialize(const nlohmann::json& jsonObj)
{
	CharacterBase::Deserialize(jsonObj);
}

void FLAC::Serialize(nlohmann::json& outJson) const
{
	CharacterBase::Serialize(outJson);

	
}

bool FLAC::CheckLengthBulPlay()
{
	float minlength = 100.0f;
	for (auto&obj : SceneManager::Instance().GetObjList())
	{
		if (obj->GetTag() != tEnemyAttack) {
			auto dist = obj->GetMatrix().Translation() - m_mWorld.Translation();
			float length = dist.Length();
			if (minlength > length) {
				minlength = length;
			}
		}
	}
	auto target = m_wpTarget.lock();
	if (target) {
		auto targetDist = target->GetMatrix().Translation() - m_mWorld.Translation();
		float length = targetDist.Length();
		if (minlength < length && length-minlength >10.0f) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
	return false;
	}
	return false;
}

const Math::Matrix& FLAC::UpdateMatrix() {
	auto pos = m_mWorld.Translation();
	Math::Matrix _rotation = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rot.y));
	m_mWorld = m_mScale * _rotation * Math::Matrix::CreateTranslation(pos);
	return m_mWorld;
}

void FLAC::ActionStateBase::ChangeStateWithDistance(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto target = spObj.lock();

	if (!spOwner) { return; }
	if (!target) { return; }

	auto& trans = spOwner->GetMatrix();
	auto& targetTrans = target->GetMatrix();

	auto dist = targetTrans.Translation() - trans.Translation();
	auto length = dist.Length();
	auto halfLen = (spOwner->m_dist.y + spOwner->m_dist.x) / 2.0f;

	auto flg = spOwner->CheckLengthBulPlay();
	if (flg) {
		spOwner->ChangeActionState(std::make_shared<Avoid>());
		return;
	}

	if (length > spOwner->m_dist.y) {
		spOwner->ChangeActionState(std::make_shared<FrontMove>());
		return;
	}
	else if (length > halfLen) {
		spOwner->ChangeActionState(std::make_shared<LeftMove>());
		return;
	}
	else if (length > spOwner->m_dist.x) {
		spOwner->ChangeActionState(std::make_shared<RightMove>());
		return;
	}
	else {
		spOwner->ChangeActionState(std::make_shared<BackMove>());
		return;
	}

}

void FLAC::ActionStateBase::ChangeStateWithPrev(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto objTarget = spObj.lock();
	auto target = spOwner->m_wpCharacterTarget.lock();

	auto nowVec = spOwner->m_mWorld.Backward();
	nowVec.Normalize();

	auto targetVec = target->GetMatrix().Backward();
	targetVec.Normalize();

	UINT side = Serch(nowVec, targetVec);

	auto distance = target->GetPos() - spOwner->GetPos();


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
		if (spOwner->GetPrevState() == FLACStateType::LeftMove)
		{
			spOwner->ChangeActionState(std::make_shared<LeftMoveAttack>());
			return;
		}

		if (spOwner->GetPrevState() == FLACStateType::BackMove)
		{
			spOwner->ChangeActionState(std::make_shared<BackMoveAttack>());
			return;
		}

		if (spOwner->GetPrevState() ==  FLACStateType::RightMove)
		{
			spOwner->ChangeActionState(std::make_shared<FrontMoveAttack>());

			auto vec = distance;
			vec.Normalize();
			spOwner->BoostRotate(vec);
			spOwner->UpdateMatrix();

		}

		if (spOwner->GetPrevState() == FLACStateType::Avoid) {
			if (spOwner->m_rand.GetInt(1, 2) == 1)
			{
				spOwner->ChangeActionState(std::make_shared<Avoid>());
				return;
			}
			else {
				spOwner->ChangeActionState(std::make_shared<LeftMoveAttack>());
				return;
			}
		}

		if (spOwner->GetPrevState() == FLACStateType::Hited)
		{
			if (spOwner->m_rand.GetInt(1, 2) == 1)
			{
				spOwner->ChangeActionState(std::make_shared<Avoid>());
				return;
			}
			else {
				spOwner->ChangeActionState(std::make_shared<LeftMove>());
				return;
			}
		}

	}

	else if (side == ActionStateBase::TargetSide::Right)
	{
		if (spOwner->GetPrevState() == FLACStateType::LeftMove) {
			spOwner->ChangeActionState(std::make_shared<RightMoveAttack>());
			return;
		}

		if (spOwner->GetPrevState() == FLACStateType::LeftMove)
		{
			spOwner->ChangeActionState(std::make_shared<Avoid>());

			auto vec = distance;
			vec.Normalize();
			spOwner->BoostRotate(vec);
			spOwner->UpdateMatrix();

			spOwner->m_nowAction->SetParam(100.0f, spOwner->m_mWorld.Left());
			return;
		}

		if (spOwner->GetPrevState() == FLACStateType::Avoid) {
			spOwner->ChangeActionState(std::make_shared<LeftMoveAttack>());
			return;
		}

		if (spOwner->GetPrevState() == FLACStateType::BackMove)
		{
			if (spOwner->m_rand.GetInt(1, 2) == 1)
			{
				spOwner->ChangeActionState(std::make_shared<RightMoveAttack>());
				return;
			}
			else {
				spOwner->ChangeActionState(std::make_shared<FrontMoveAttack>());
				return;
			}
		}


	}

	else if (side == ActionStateBase::TargetSide::Back)
	{
		if (spOwner->GetPrevState() == FLACStateType::Avoid) {
			spOwner->ChangeActionState(std::make_shared<BackMoveAttack>());
			return;
		}

		if (spOwner->GetPrevState() == FLACStateType::LeftMove)
		{
			spOwner->ChangeActionState(std::make_shared<Avoid>());
			spOwner->m_nowAction->SetParam(130.0f, spOwner->m_mWorld.Right());
			return;
		}

		if (spOwner->GetPrevState() == FLACStateType::RightMove) {
			spOwner->ChangeActionState(std::make_shared<RightMoveAttack>());
			return;
		}

		if (spOwner->GetPrevState() == FLACStateType::FrontMove)
		{
			spOwner->ChangeActionState(std::make_shared<Avoid>());
			auto vec = distance;
			vec.Normalize();
			spOwner->BoostRotate(vec);
			spOwner->UpdateMatrix();
			spOwner->m_nowAction->SetParam(100.0f, spOwner->m_mWorld.Forward());
			return;
		}

		if (spOwner->GetPrevState() == FLACStateType::Avoid) {
			if (spOwner->m_rand.GetInt(1, 2) == 1)
			{
				spOwner->ChangeActionState(std::make_shared<BackMoveAttack>());
				return;
			}
			else {
				spOwner->ChangeActionState(std::make_shared<FrontMoveAttack>());
				return;
			}
		}

		if (spOwner->GetPrevState() == FLACStateType::Avoid)
		{
			spOwner->ChangeActionState(std::make_shared<FrontMove>());
			return;
		}
	}

	else if (side == ActionStateBase::TargetSide::Front)
	{
		if (spOwner->GetPrevState() == FLACStateType::FrontMove)
		{
			spOwner->ChangeActionState(std::make_shared<Avoid>());
			auto vec = distance;
			vec.Normalize();
			spOwner->BoostRotate(vec);
			spOwner->UpdateMatrix();
			spOwner->m_nowAction->SetParam(100.0f, spOwner->m_mWorld.Forward());
			return;
		}

		if (spOwner->GetPrevState() == FLACStateType::Avoid) {
			if (spOwner->m_rand.GetInt(1, 2) == 1)
			{
				spOwner->ChangeActionState(std::make_shared<Stand>());
				return;
			}
			else {
				spOwner->ChangeActionState(std::make_shared<BackMoveAttack>());
				return;
			}
		}

		if (spOwner->GetPrevState() == FLACStateType::Avoid)
		{
			spOwner->ChangeActionState(std::make_shared<LeftMoveAttack>());
			return;
		}
	}

	if (spOwner->GetPrevState() == FLACStateType::BackMove)
	{
		spOwner->ChangeActionState(std::make_shared<BackMoveAttack>());
		return;

	}

	{
		spOwner->ChangeActionState(std::make_shared<Fly>());
		return;
	}




}

void FLAC::ActionStateBase::SetParam(float speed, const Math::Vector3& direct)
{
	m_speed = speed;
	m_direct = direct;
}

UINT FLAC::ActionStateBase::Serch(const Math::Vector3& nowVec, const Math::Vector3& targetVec)
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
		side = ActionStateBase::Front;
	}

	return side;
}



void FLAC::UpdateRotate(const Math::Vector3& srcMoveVec)
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

void FLAC::ActionStateBase::CheckAttackLevel(std::weak_ptr<FLAC>& owner)
{
	auto spOwner = owner.lock();
	auto hp = spOwner->m_hp;
	auto max = spOwner->m_maxHp;

	if (hp > ((max / 4.0f) * 3.0f))
	{
		auto Int = spOwner->m_rand.GetInt(0, 3);
		if (Int == 0) {
			spOwner->ChangeEnableLeftAttack(true);
		}
		else if (Int == 1) {
			spOwner->ChangeEnableLeftShoulderAttack(true);
		}
		else if (Int == 2) {
			spOwner->ChangeEnableRightAttack(true);
		}
		else {
			spOwner->ChangeEnableRightShoulderAttack(true);
		}
	}
	else if (hp > ((max / 4.0f) * 2.0f))
	{
		spOwner->ChangeEnableRightAttack(true);
		auto Int = spOwner->m_rand.GetInt(0, 2);
		if (Int == 0) {
			spOwner->ChangeEnableLeftAttack(true);
		}
		else if (Int == 1) {
			spOwner->ChangeEnableLeftShoulderAttack(true);
		}
		else {
			spOwner->ChangeEnableRightShoulderAttack(true);
		}
	}
	else if (hp > ((max / 4.0f))) {
		auto Int = spOwner->m_rand.GetInt(0, 1);
		if (Int == 0) {
			spOwner->ChangeEnableLeftAttack(true);
		}
		else{
			spOwner->ChangeEnableRightAttack	(true);
		}
		spOwner->ChangeEnableLeftShoulderAttack(true);
		spOwner->ChangeEnableRightShoulderAttack(true);
	}
	else {
		spOwner->ChangeEnableLeftAttack(true);
		spOwner->ChangeEnableRightAttack(true);
		spOwner->ChangeEnableRightShoulderAttack(true);
		spOwner->ChangeEnableLeftShoulderAttack(true);

	}
}

void FLAC::ActionStateBase::AttackOff(std::weak_ptr<FLAC>& owner)
{
	auto spOwner = owner.lock();

	spOwner->ChangeEnableLeftAttack(true);
	spOwner->ChangeEnableRightAttack(true);
	spOwner->ChangeEnableRightShoulderAttack(true);
	spOwner->ChangeEnableLeftShoulderAttack(true);
}


void FLAC::StandUp::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::Start;

	if (spOwner->m_spModelWork) {
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("StandUp"), false);
	}

}

void FLAC::StandUp::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: StandUp\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}
	
}

void FLAC::StandUp::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

}

void FLAC::StandUp::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

}
void FLAC::Stand::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::Stand;

	if (spOwner->m_spModelWork) { 
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("Stand"), false);
	}


}

void FLAC::Stand::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: Stand\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	bool isFind = spOwner->SearchPlayer();

	if (isFind) {
		spOwner->ChangeActionState(std::make_shared<Fly>());
		return;
	}
}

void FLAC::Stand::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);
}

void FLAC::Stand::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

}
void FLAC::Fly::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::Fly;

	if (spOwner->m_spModelWork) {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("Float"), false);
	}

}

void FLAC::Fly::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: Fly\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	bool isFind = spOwner->SearchPlayer();

	if (isFind) {
		ChangeStateWithDistance(owner,spObj);
		return;
	}

}

void FLAC::Fly::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);
}

void FLAC::Fly::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	if (!spOwner->m_spModelWork) { return; }
}

//void FLAC::Boost::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
//{
//	auto spOwner = owner.lock();
//	auto spTarget = spObj.lock();
//	if (!spOwner) { return; }
//
//	m_type = FLACStateType::boost;
//	
//	if (spOwner->m_spModelWork) {
//		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("Boost"), false);
//	}
//}
//
//void FLAC::Boost::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
//{
//	Application::Instance().m_log.AddLog("FLACnowState: Boost\n");
//
//	auto spOwner = owner.lock();
//	auto spTarget = spObj.lock();
//	if (!spOwner) { return; }
//	
//}
//
//void FLAC::Boost::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
//{
//
//}
//
//void FLAC::Boost::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
//{
//
//}

void FLAC::FrontMove::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::FrontMove;

	if (spOwner->m_spModelWork) {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("FlontMove"), false);
	}

	m_speed = 130.0f;
	m_direct = spOwner->GetMatrix().Backward();
	m_direct.Normalize();

	m_durationState = 0.4f;
}

void FLAC::FrontMove::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: FrontMove\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (spTarget) {
		m_direct = spTarget->GetMatrix().Translation() - spOwner->GetMatrix().Translation();
		m_direct.Normalize();
	}

	spOwner->MoveSwept(m_speed,m_direct,KdCollider::TypeGround);

	if (m_durationState < 0.0f) {
		ChangeStateWithPrev(owner,spObj);
	}

}

void FLAC::FrontMove::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);
}

void FLAC::FrontMove::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

}

void FLAC::FrontMoveAttack::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::FrontMoveAttack;

	if (spOwner->m_spModelWork) {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("FlontMoveAttack"), false);
	}

	m_speed = 100.0f;
	m_direct = spOwner->GetMatrix().Backward();
	m_direct.Normalize();

	m_durationState = 1.0f;

	CheckAttackLevel(owner);
}

void FLAC::FrontMoveAttack::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: FrontMoveAttack\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (spTarget) {
		m_direct = spTarget->GetMatrix().Translation() - spOwner->GetMatrix().Translation();
		m_direct.Normalize();
	}

	spOwner->MoveSwept(m_speed,m_direct,KdCollider::TypeGround);

	if (m_durationState < 0.0f) {
		ChangeStateWithDistance(owner,spObj);
	}
}

void FLAC::FrontMoveAttack::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

}

void FLAC::FrontMoveAttack::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	AttackOff(owner);
}

void FLAC::BackMove::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::BackMove;

	if (spOwner->m_spModelWork) {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("BackMove"), false);
	}

	m_speed = 120.0f;
	m_direct = spOwner->GetMatrix().Forward();
	m_direct.Normalize();

	m_durationState = 1.0f;
}

void FLAC::BackMove::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: BackMove\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (spTarget) {
		m_direct = spOwner->GetMatrix().Translation() - spTarget->GetMatrix().Translation();
		m_direct.Normalize();
	}

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround);

	if (m_durationState < 0.0f) {
		ChangeStateWithPrev(owner, spObj);
	}
}

void FLAC::BackMove::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

}

void FLAC::BackMove::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
}

void FLAC::BackMoveAttack::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::BackMoveAttack;

	if (spOwner->m_spModelWork) {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("BackMoveAttack"), false);
	}

	m_speed = 90.0f;
	m_direct = spOwner->GetMatrix().Forward();
	m_direct.Normalize();

	m_durationState = 1.4f;

	CheckAttackLevel(owner);
}

void FLAC::BackMoveAttack::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: BackMoveAttack\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (spTarget) {
		m_direct = spTarget->GetMatrix().Translation() - spOwner->GetMatrix().Translation();
		m_direct.Normalize();
	}

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround);

	if (m_durationState < 0.0f) {
		ChangeStateWithDistance(owner, spObj);
	}
}

void FLAC::BackMoveAttack::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

}

void FLAC::BackMoveAttack::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	AttackOff(owner);
}

void FLAC::LeftMove::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::LeftMove;

	if (spOwner->m_spModelWork) {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("LeftMove"), false);
	}

	m_speed = 130.0f;
	m_direct = spOwner->GetMatrix().Left();
	m_direct.Normalize();

	m_durationState = 0.4f;
}

void FLAC::LeftMove::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: LeftMove\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (spTarget) {
		m_direct = spTarget->GetMatrix().Translation() - spOwner->GetMatrix().Translation();
		m_direct.Normalize();
	}

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround);

	if (m_durationState < 0.0f) {
		ChangeStateWithPrev(owner, spObj);
	}
}

void FLAC::LeftMove::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

}

void FLAC::LeftMove::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

}

void FLAC::LeftMoveAttack::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::LeftMoveAttack;

	if (spOwner->m_spModelWork) {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("LeftMoveAttack"), false);
	}

	m_speed = 100.0f;
	m_direct = spOwner->GetMatrix().Left();
	m_direct.Normalize();

	m_durationState = 1.7f;

	CheckAttackLevel(owner);
}

void FLAC::LeftMoveAttack::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: LeftMoveAttack\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (spTarget) {
		m_direct = spTarget->GetMatrix().Translation() - spOwner->GetMatrix().Translation();
		m_direct.Normalize();
	}

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround);

	if (m_durationState < 0.0f) {
		ChangeStateWithDistance(owner, spObj);
	}
}

void FLAC::LeftMoveAttack::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

}

void FLAC::LeftMoveAttack::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	AttackOff(owner);
}

void FLAC::RightMove::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::RightMove;

	if (spOwner->m_spModelWork) {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("RightMove"), false);
	}

	m_speed = 130.0f;
	m_direct = spOwner->GetMatrix().Right();
	m_direct.Normalize();

	m_durationState = 0.4f;
}

void FLAC::RightMove::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: RightMove\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (spTarget) {
		m_direct = spTarget->GetMatrix().Translation() - spOwner->GetMatrix().Translation();
		m_direct.Normalize();
	}

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround);

	if (m_durationState < 0.0f) {
		ChangeStateWithPrev(owner, spObj);
	}
}

void FLAC::RightMove::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

}

void FLAC::RightMove::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	AttackOff(owner);
}

void FLAC::RightMoveAttack::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::RightMoveAttack;

	if (spOwner->m_spModelWork) {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("RightMoveAttack"), false);
	}

	m_speed = 100.0f;
	m_direct = spOwner->GetMatrix().Right();
	m_direct.Normalize();

	m_durationState = 0.9f;

	CheckAttackLevel(owner);
}

void FLAC::RightMoveAttack::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: RightMoveAttack\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (spTarget) {
		m_direct = spTarget->GetMatrix().Translation() - spOwner->GetMatrix().Translation();
		m_direct.Normalize();
	}

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround);

	if (m_durationState < 0.0f) {
		ChangeStateWithDistance(owner, spObj);
	}
}

void FLAC::RightMoveAttack::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

}

void FLAC::RightMoveAttack::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	AttackOff(owner);
}

void FLAC::Avoid::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::Avoid;

	if (spOwner->m_spModelWork) {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("Avoid"), false);
	}

	
	m_speed = 200.0f;
	auto in = spOwner->m_rand.GetInt(1, 3);
	if (in == 1) {
		m_avoidSide = TargetSide::Left;
		m_direct = spOwner->GetMatrix().Left();
	}
	else if(in == 2){
		m_avoidSide = TargetSide::Back;
		m_direct = spOwner->GetMatrix().Forward();
	}
	else {
		m_avoidSide = TargetSide::Right;
		m_direct = spOwner->GetMatrix().Right();
	}
	m_direct.Normalize();

	m_durationState = 0.7f;
}

void FLAC::Avoid::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: Avoid\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround);

	if (spOwner->m_spAnimator->IsAnimationEnd()) {
		ChangeStateWithDistance(owner, spObj);
	}
}

void FLAC::Avoid::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

}

void FLAC::Avoid::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
}
void FLAC::Hited::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::Hited;

	if (spOwner->m_spModelWork) {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("Hited"), false);
	}

	m_speed = 40.0f;
	m_direct = spOwner->GetMatrix().Forward();
	m_direct.Normalize();

	
}

void FLAC::Hited::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: Hited\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	spOwner->MoveSwept(m_speed, m_direct, KdCollider::TypeGround);

	if (m_durationState < 0.0f) {
		ChangeStateWithDistance(owner, spObj);
	}
}

void FLAC::Hited::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

}

void FLAC::Hited::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

}
void FLAC::Destroyed::Enter(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	m_type = FLACStateType::Destroyed;

	if (spOwner->m_spModelWork) {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetAnimation("Destroyed"), false);
	}

	m_speed = 0.0f;
	
	m_durationState = 0.4f;
	spOwner->m_isDestroy = true;
}

void FLAC::Destroyed::Update(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	Application::Instance().m_log.AddLog("FLACnowState: Destroyed\n");

	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{

		m_durationState -= KdFPSController::GetInstance().GetDeltaTime();
		if (m_durationState < 0.0f) {
			spOwner->m_isExpired = true;
		}
	}
}

void FLAC::Destroyed::PostUpdate(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (!spOwner->m_spModelWork) { return; }

	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f);

}

void FLAC::Destroyed::Exit(std::weak_ptr<FLAC>& owner, const std::weak_ptr<KdGameObject>& spObj)
{
	auto spOwner = owner.lock();
	auto spTarget = spObj.lock();
	if (!spOwner) { return; }
	if (!spOwner->m_spModelWork) { return; }
}

void FLAC::ChangeActionState(std::shared_ptr<ActionStateBase> nextAction)
{
	if (m_nowAction)m_nowAction->Exit(m_wpThis,m_wpTarget);
	m_prevAction = m_nowAction;
	m_nowAction = nextAction;
	m_nowAction->Enter(m_wpThis,m_wpTarget);
}
