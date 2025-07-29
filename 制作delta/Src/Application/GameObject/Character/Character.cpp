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

	Math::Vector3 pos = { 0.0f,6.0f,0.0f };
	m_correctionMat = Math::Matrix::CreateTranslation(pos);



	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape("Player", m_spModelWork, KdCollider::TypeDamage);

	InitTrail();
	UnEnableTrail();

	//初期状態を「待機状態」へ設定
	ChangeActionState(std::make_shared<ActionIdle>());

	m_tag = tPlayer;

	m_hp = 10000;

	m_nockBackDamage = 200.0f;

	m_name = "Player";

}

void Character::Update()
{

	Application::Instance().m_log.Clear();

	Application::Instance().m_log.AddLog("Player\n");

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
		KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(0.55f);
		// リムライト
		m_limEnable = false;

		m_transAC = false;

		m_speedMag = 1.0f;

	}

	if (key.P || pad.IsLeftStickPressed())
	{
		//ブルーム
		KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(0.15f);
		// リムライト
		m_limEnable = true;

		m_transAC = true;

		m_speedMag = 1.5f;
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

	if (IsIgnoreGravityState() == false)
	{
		Move(m_gravity, Math::Vector3::Down, KdCollider::TypeGround, false, true, true, false);
		m_gravity += m_gravityPow * KdFPSController::GetInstance().GetDeltaTime();
	}

	//1m_pCollider->RegisterCollisionShape(KdCollider::TypeBump)

	Application::Instance().m_log.AddLog("HP%.f\n", m_hp);


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

void Character::OnHit()
{
	if (m_hp <= 0)
	{
		ChangeActionState(std::make_shared<ActionDestroyed>());
		m_parameter = 0;
		return;
	}

	if (m_parameter >= m_nockBackDamage)
	{
		ChangeActionState(std::make_shared<ActionHited>());
		m_parameter = 0;
		return;
	}
}

void Character::Editor_ImGui()
{
	KdGameObject::Editor_ImGui();

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

void Character::Deserialize(const nlohmann::json& jsonObj)
{
	KdGameObject::Deserialize(jsonObj);

}

void Character::Serialize(nlohmann::json& outJson) const
{
	KdGameObject::Serialize(outJson);
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

const bool Character::IsLeftShoulder()
{
	auto& key = KeyInput::GetInstance().GetKeyboardState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	if (key.Q || pad.IsLeftShoulderPressed())
	{
		return true;
	}
	return false;
}

const bool Character::IsRightShoulder()
{
	auto& key = KeyInput::GetInstance().GetKeyboardState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	if (key.E || pad.IsRightShoulderPressed())
	{
		return true;
	}
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

void Character::UpdateCollision()
{


	// その他球による衝突判定
	// ---- ---- ---- ---- ---- ----
	// ①当たり判定(球判定)用の情報を作成
	KdCollider::SphereInfo sphereInfo;
	sphereInfo.m_sphere.Center = GetPos() + Math::Vector3(0, 5.0f, 0);
	sphereInfo.m_sphere.Radius = 3.0f;
	sphereInfo.m_type = KdCollider::TypeGround;

	Math::Vector3 pos;
	SphereCast(sphereInfo.m_sphere.Center, m_mWorld.Backward(), sphereInfo.m_sphere.Radius, KdCollider::TypeGround, pos);



	sphereInfo.m_sphere.Center = GetPos() + Math::Vector3(0, 3.5f, 0);
	sphereInfo.m_sphere.Radius = 300.0f;
	sphereInfo.m_type = KdCollider::TypeDamage;


	//
	m_wpCharacterTarget.reset();

	std::vector<std::shared_ptr<CameraManager::LockTargetInfo>> vec;

	// ②HIT対象オブジェクトに総当たり
	for (auto& obj : SceneManager::Instance().GetEnemyList())
	{
		if (obj->Intersects(sphereInfo, nullptr))
		{
			std::shared_ptr<CameraManager::LockTargetInfo> info = std::make_shared<CameraManager::LockTargetInfo>();
			auto targetPos = obj->GetMatrix().Translation();
			targetPos.y = 0.0f;
			auto pos = m_mWorld.Translation();
			pos.y = 0.0f;
			float distance = (targetPos - pos).Length();

			if (SearchDetect(targetPos, m_mWorld, 100) == true)
			{
				info->wpLockTarget = obj;
				info->distance = distance;
				vec.push_back(info);
			}
		}
	}

	std::sort(vec.begin(), vec.end());
	if (vec.empty() == false) {
		CameraManager::Instance().ResetMultiLocks();
	}

	float dist = 0;


	for (int i = 0; i < std::min((int)vec.size(), CameraManager::Instance().GetMultiLockNum()); i++)
	{
		CameraManager::Instance().SetMultiLocks(vec[i]->wpLockTarget.lock());
	}
	if (vec.empty() == false)
	{
		CameraManager::Instance().SetLockTarget(vec[0]->wpLockTarget.lock());
		dist = vec[0]->distance;
		m_wpCharacterTarget = vec[0]->wpLockTarget;

	}




	DirectX::BoundingOrientedBox box;

	box.Center = GetPos() + m_correctionMat.Translation();
	box.Extents = { 3.0f,5.0f,3.0f };
	UINT type = KdCollider::TypeDamage;
	KdCollider::BoxInfo boxInfo(type, box);

	auto translation = m_mWorld.Translation();

	for (std::weak_ptr<KdGameObject> wpGameObj : SceneManager::Instance().GetEnemyList())
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
					Application::Instance().m_log.AddLog("hitPos x:%.2f,y:%.2f,z:%.2f\n", ret.m_hitPos.x, ret.m_hitPos.y, ret.m_hitPos.z);
					//	SetPos(nowPos);
				}
			}
		}
	}

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (obj->Intersects(boxInfo, nullptr))
		{
			if (obj->GetTag() == tEnemyAttack)
			{
				obj->OnHit();
				m_parameter = obj->GetParameter();
				HitDamage(obj->GetParameter());
				OnHit();
			}
		}
	}

	Application::Instance().m_log.AddLog("length %.2f\n", dist);
	Application::Instance().m_log.AddLog("pos x:%.2f,y:%.2f,z:%.2f\n", translation.x, translation.y, translation.z);

	m_pDebugWire->AddDebugSphere(sphereInfo.m_sphere.Center, sphereInfo.m_sphere.Radius, color);
	m_pDebugWire->AddDebugBox(m_correctionMat * m_mWorld, box.Extents, {}, true, color);

}

bool Character::IsIgnoreGravityState() const
{
	// 判定
	if (m_nowAction->GetState() == CharacterStateName::Fly ||
		m_nowAction->GetState() == CharacterStateName::FlyGuard
		) {
		return true;
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
	newObj->trail->SetLength(20);
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
	newObj->trail->SetLength(20);
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
	newObj->trail->SetLength(20);
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
	newObj->trail->SetLength(20);
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

	spOwner->ChangeEnableLeftAttack(false);
	spOwner->ChangeEnableRightAttack(false);
	spOwner->ChangeEnableLeftShoulderAttack(false);
	spOwner->ChangeEnableRightShoulderAttack(false);


	m_isMove = spOwner->IsMove();
	m_isBoost = spOwner->IsBoost();
	m_isFlow = spOwner->IsFlow();
	m_isRightAttack = spOwner->IsAttack();
	m_isGuard = spOwner->IsGuard();
	m_isLeftShoulder = spOwner->IsLeftShoulder();
	m_isRightShoulder = spOwner->IsRightShoulder();
	if (m_isRightAttack)
	{
		spOwner->ChangeEnableRightAttack(true);
	}
	if (m_isGuard)
	{
		spOwner->ChangeEnableLeftAttack(true);
	}
	if (m_isLeftShoulder)
	{
		spOwner->ChangeEnableLeftShoulderAttack(true);
	}
	if (m_isRightShoulder)
	{
		spOwner->ChangeEnableRightShoulderAttack(true);
	}

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

	m_stateNum = spOwner->CharacterStateName::Stand;

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

	if (m_isLeftShoulder)
	{
		spOwner->ChangeActionState(std::make_shared<ActionLeftShoulderAttack>());
		return;
	}



}

void Character::ActionIdle::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f * spOwner->m_speedMag);

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

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandUp"), 30.0f, false);

	m_direction = Math::Vector3::Zero;

	m_speed = 0.0f * spOwner->m_speedMag;

	m_animName = "StandUp";

	m_stateNum = spOwner->CharacterStateName::StandUp;

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

	if (m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandShield>());
		return;
	}

	if (m_isLeftShoulder)
	{
		spOwner->ChangeActionState(std::make_shared<ActionLeftShoulderAttack>());
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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f * spOwner->m_speedMag);

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

	m_stateNum = spOwner->CharacterStateName::StandGuard;

	spOwner->ChangeEnableLeftAttack(true);
}

void Character::ActionStandShield::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("StandShield\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	if (m_isGuard == true)
	{
		if (m_isMove)
		{
			spOwner->ChangeActionState(std::make_shared<ActionMoveShield>());
			return;
		}

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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f * spOwner->m_speedMag);

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

	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false, false, false, true);

	m_animName = "Stand";

	m_stateNum = spOwner->CharacterStateName::Fly;

	Application::Instance().m_log.AddLog("FlySuccess\n");



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

	spOwner->ChangeActionState(std::make_shared<ActionIdle>());
	return;

}

void Character::ActionJump::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f * spOwner->m_speedMag);

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

	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false, false, false, true);

	m_animName = "StandShield";

	m_stateNum = spOwner->CharacterStateName::FlyGuard;

}

void Character::ActionJumpShield::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("FlyShield\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);



	// ガードしていない状況
	if (m_isMove && !m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}

	// ガードしている状況
	if (m_isMove && m_isGuard)
	{
		if (!m_isFlow)
		{
			spOwner->ChangeActionState(std::make_shared<ActionStandShield>());
			return;
		}
		else
		{
			spOwner->ChangeActionState(std::make_shared<ActionJumpShield>());
			return;
		}
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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f * spOwner->m_speedMag);

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
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Walk"), 6.0f, false);

	m_speed = spOwner->m_walkSpeed * spOwner->m_speedMag;

	m_animName = "Walk";

	m_stateNum = spOwner->CharacterStateName::Walk;

	KdAudioManager::Instance().Play("Asset/Sounds/Sound/walk_2.wav", false);


}

void Character::ActionMove::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("Walk\n");

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

	if (m_isLeftShoulder)
	{
		spOwner->ChangeActionState(std::make_shared<ActionLeftShoulderAttack>());
		return;
	}



	//移動中に何も入力がなければ待機に移行
	if (m_isMove == false) {
		spOwner->ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}
	else
	{
		if (spOwner->m_spAnimator->IsAnimationEnd())
		{
			spOwner->ChangeActionState(std::make_shared<ActionMove>());
			return;
		}

		m_direction = ActionStateBase::Direct(owner, true);


		spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);

	}



}

void Character::ActionMove::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f * spOwner->m_speedMag);

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

	spOwner->ChangeEnableLeftAttack(true);

	m_stateNum = spOwner->CharacterStateName::WalkGuard;

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

	if (m_isFlow && !m_isGuard) {
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}


	if (m_isFlow && m_isGuard) {
		spOwner->ChangeActionState(std::make_shared<ActionJumpShield>());
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

		if (m_isGuard)
		{
			spOwner->ChangeActionState(std::make_shared<ActionStandShield>());
			return;
		}

		spOwner->ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}
	else
	{
		if (!m_isGuard) {
			spOwner->ChangeActionState(std::make_shared<ActionMove>());
			return;
		}

		m_direction = ActionStateBase::Direct(owner, true);


		spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false);

	}

}

void Character::ActionMoveShield::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}


void Character::ActionMoveShield::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->ChangeEnableLeftAttack(false);


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
	KdShaderManager::Instance().m_postProcessShader.SetRadialBlurInfo(6, 0.7f, { 0.5f,0.5f }, 0.4f, 0, 0.02f);
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

	m_stateNum = spOwner->CharacterStateName::Boost;


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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f * spOwner->m_speedMag);

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

	m_stateNum = spOwner->CharacterStateName::BoostNow;

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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f * spOwner->m_speedMag);

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

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostEnd"), 50.0f, false);

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

	}

	m_animName = "BoostEnd";

	m_stateNum = spOwner->CharacterStateName::BoostEnd;

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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f * spOwner->m_speedMag);

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

	m_stateNum = spOwner->CharacterStateName::BoostDush;

}

void Character::ActionBoostDush::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("BoostDush\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	if (m_isFlow && !m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}

	if (m_isFlow && m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJumpShield>());
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

	if (m_isLeftShoulder)
	{
		spOwner->ChangeActionState(std::make_shared<ActionLeftShoulderAttack>());
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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f * spOwner->m_speedMag);

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

	m_stateNum = spOwner->CharacterStateName::BoostDushGuard;


	spOwner->ChangeEnableLeftAttack(true);

}

void Character::ActionBoostShield::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("BoostDushShield\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	if (m_isFlow && !m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}

	if (m_isFlow && m_isGuard)
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
		if (!m_isGuard) {
			spOwner->ChangeActionState(std::make_shared<ActionBoostDush>());
			return;
		}

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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}


void Character::ActionBoostShield::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->UnEnableTrail();

	//エフェクト
	EffectExit();

	spOwner->ChangeEnableLeftAttack(false);

}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//攻撃状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionRightAttack::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("RightBladeAttackBef"), 20.0f, false);

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
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("Thruster.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(),1.0f,3.0f);
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	m_animName = "RightBladeAttackBef";

	m_stateNum = spOwner->CharacterStateName::RightSorwdBef;

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
		spOwner->ChangeActionState(std::make_shared<ActionRightAttackMid>());
		return;
	}

	//敵が一定範囲内なら敵のほうに向いて敵に
	std::shared_ptr<KdGameObject> target = spOwner->m_wpTarget.lock();
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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}


void Character::ActionRightAttack::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	//エフェクト
	EffectExit();

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//攻撃中状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionRightAttackMid::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("RightSowrdMid"), 10.0f, false);

	m_speed = spOwner->m_bladeAttackSpeed * spOwner->m_speedMag;

	m_direction = ActionStateBase::Direct(owner, false);

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

	m_animName = "RightSowrdMid";

	m_stateNum = spOwner->CharacterStateName::RightSorwdMid;


	spOwner->ChangeEnableRightAttack(true);

}

void Character::ActionRightAttackMid::Update(std::weak_ptr<Character>& owner)
{

	Application::Instance().m_log.AddLog("AttackNow\n");

	std::shared_ptr<Character> spOwner = owner.lock();


	{
		spOwner->Move(m_speed, m_direction, KdCollider::TypeGround);
	}

	//エフェクト
	EffectUpdate(owner);



	if (spOwner->m_spAnimator->IsComp() == false)
	{
		spOwner->ChangeActionState(std::make_shared<ActionRightAttackAf>());
		return;
	}
}

void Character::ActionRightAttackMid::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}

void Character::ActionRightAttackMid::Exit(std::weak_ptr<Character>& owner)
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
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("RightSowrd"), 80.0f, false);

	m_speed = spOwner->m_bladeAttackSpeed * spOwner->m_speedMag;

	m_direction = ActionStateBase::Direct(owner, false);

	m_stiffnessTime = 1.0f;

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

	m_animName = "RightSowrd";

	m_stateNum = spOwner->CharacterStateName::RightSorwdAf;

	spOwner->ChangeEnableRightAttack(true);

}

void Character::ActionRightAttackAf::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("AttackAf\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	//移動
	if (m_durationStiffness <= m_stiffnessTime / 4)
	{
		spOwner->Move(m_speed, m_direction, KdCollider::TypeGround);
	}

	if (spOwner->m_spAnimator->IsComp() == false)
	{
		spOwner->UnEnableTrail();
	}

	//エフェクト
	EffectUpdate(owner);



	if (spOwner->m_spAnimator->IsComp() == false)
	{
		//加算
		m_durationStiffness += KdFPSController::GetInstance().GetDeltaTime();
	}

	//もし硬直時間が終了してなければ強制的に終了
	if (m_durationStiffness <= m_stiffnessTime)
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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 40.0f * spOwner->m_speedMag);

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

	m_stateNum = spOwner->CharacterStateName::Hited;

}

void Character::ActionHited::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false, false);

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

	if (m_isMove && !m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionMove>());
		return;
	}

	if (m_isMove && m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionMoveShield>());
		return;
	}



	spOwner->ChangeActionState(std::make_shared<ActionIdle>());
	return;


}

void Character::ActionHited::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f * spOwner->m_speedMag);

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

	m_stateNum = spOwner->CharacterStateName::Destoryed;

	spOwner->m_isDestroy = true;
}

void Character::ActionDestroyed::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	//グリッチ表現
	auto time = KdFPSController::GetInstance().GetFPS();

	UINT kind = KdShaderManager::Instance().m_postProcessShader.Glitch;
	KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	KdShaderManager::Instance().m_postProcessShader.
		SetGlitch({ 1,1 }, time, 5.0f, 0.8f, 0, 0, { 0.5f,0.5f });

	if (spOwner->m_spAnimator->IsAnimationEnd() == false) { return; }


	
}

void Character::ActionDestroyed::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	if (spOwner->m_spAnimator->GetProgress() < 0.4f)
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 20.0f * spOwner->m_speedMag);
	else
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f * spOwner->m_speedMag);
	}
}

void Character::ActionDestroyed::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	UINT kind = KdShaderManager::Instance().m_postProcessShader.Normal;
	KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	KdShaderManager::Instance().m_postProcessShader.UndoGlitch();


}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//左肩状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionLeftShoulderAttack::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Hited"), 50.0f, false);

	//CameraManager::Instance().SetNextType(CameraManager::Animation);

	m_direction = spOwner->m_mWorld.Forward();

	m_speed = 40.0f;

	m_stiffnessTime = 3.0f;

	m_animName = "Hited";

	m_stateNum = spOwner->CharacterStateName::LeftShoulderAttack;
}

void Character::ActionLeftShoulderAttack::Update(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	if (m_isOneShot == false)
	{
		m_isLeftShoulder = spOwner->IsLeftShoulder();
	}

	if (m_isLeftShoulder == true) {
		spOwner->ChangeEnableLeftShoulderAttack(true);
		return;
	}

	if (m_durationStiffness <= 0.3f)
	{
		spOwner->Move(m_speed, m_direction, KdCollider::TypeGround, false, false);
	}

	if (m_isOneShot == false)
	{
		spOwner->ChangeEnableLeftShoulderAttack(false);
		m_isOneShot = true;
	}


	m_durationStiffness += KdFPSController::GetInstance().GetDeltaTime();



	if (m_durationStiffness >= m_stiffnessTime)
	{
		spOwner->ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}
}

void Character::ActionLeftShoulderAttack::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	if (m_durationStiffness > 0.0f)
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 60.0f * spOwner->m_speedMag);
	}
	else
	{
		spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 0.0f * spOwner->m_speedMag);
	}
}

void Character::ActionLeftShoulderAttack::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

}


