#include "Character.h"

#include "../../main.h"
#include "../Camera/CameraBase.h"

#include"../../Scene/SceneManager.h"
#include"../Camera/CameraManager.h"
#include "../UI/UIManager.h"

//#include"../Effect/ConcentrationLine/ConcentrationLine.h"

#include"TransAC.h"
#include "../Effect/ScaleEffect/ScaleEffect.h"

void Character::Init()
{
	if (!m_spModelWork)
	{
		m_spModelWork = std::make_shared<KdModelWork>();
		m_spModelWork->SetModelData("Asset/Models/Grint/Grint.gltf");

		// 初期のアニメーションをセットする
		m_spAnimator = std::make_shared<KdAnimator>();
		m_spAnimator->SetAnimation(m_spModelWork->GetData()->GetAnimation("Start"), 1.0f, true, true);
	}

	m_spMrkModel = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Marker/Player.gltf");

	m_gravity = 0;
	m_mWorld = Math::Matrix::Identity;
	SetPos({ 0, 8.0f, 0 });

	m_correction = { 0.0,8.0f,0.0f };
	Math::Vector3 pos = { 0.0f,8.0f,0.0f };
	m_correctionMat = Math::Matrix::CreateTranslation(pos);

	m_emissiveColor = { 10.0f,1.0f,1.0f };


	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
	m_pCollider = std::make_unique<KdCollider>();


	DirectX::BoundingOrientedBox box;

	box.Center = GetPos() + m_correctionMat.Translation();
	box.Extents = { 3.0f,5.0f,3.0f };
	UINT type = KdCollider::TypeDamage;
	KdCollider::BoxInfo boxInfo(type, box);
	m_pCollider->RegisterCollisionShape("Player", m_spModelWork, KdCollider::TypeDamage);

	InitTrail();
	UnEnableTrail();

	//初期状態を「待機状態」へ設定
	ChangeActionState(std::make_shared<ActionStart>());

	ChangeEnableAttack(false);
	ChangeEnableLeftAttack(false);
	ChangeEnableLeftShoulderAttack(false);
	ChangeEnableRightAttack(false);
	ChangeEnableRightShoulderAttack(false);

	m_tag = tPlayer;

	m_hp = 10030;

	m_nockBackDamage = 600.0f;

	m_name = "Player";

	m_burnPath = "Asset/Textures/GameObject/Burn.png";

	m_limColor = { 0.12f,0.0f,0.23f };

	m_TACColor = { 0.63f,0.1f,0.75f };
	m_TACProg = 0.3f;
	m_TACspeed = 3.5f;

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
		CameraManager::Instance().SetNextType(CameraManager::Lock);
		LockOn();
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
		SceneManager::Instance().SetNextScene(SceneManager::SceneType::Title);
		CameraManager::Instance().ResetMultiLocks();
		return;
	}

	if (key.D8) {
		m_hp = 0;
	}

	if (IsLStick())
	{
		m_transAC = !m_transAC;
		if (m_transAC) {
			//ブルーム
			//KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(0.15f);
			// リムライト
			m_limEnable = true;

			m_transAC = true;

			m_speedMag = 1.5f;
		}
		else {
			//ブルーム
			KdShaderManager::Instance().m_postProcessShader.SetBrightThreshold(0.55f);
			// リムライト
			m_limEnable = false;

			m_transAC = false;

			m_speedMag = 1.0f;
		}
		
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
		//auto flg =Gravity(pos,Math::Vector3::Down,m_gravity);
		auto flg =Move(m_gravity,Math::Vector3::Down,KdCollider::TypeGround,false,true,false,true);

		m_gravity += m_gravityPow * KdFPSController::GetInstance().GetDeltaTime();

		if (flg) {
			Application::Instance().m_log.AddLog("Ground\n");
		}
		else {
			Application::Instance().m_log.AddLog("FallNow\n");
		}
	}
	else {
		m_isGround = false;
	}

	Application::Instance().m_log.AddLog("PlayerGravity:%.2f\n",m_gravity);

	// キャラクターの座標が確定してからコリジョンによる位置補正を行う
	LockOn();

	CreatePolygon();

	UpdateCollision();

	UIManager::GetInstance().SetPlayerHP((int)m_hp);

}

void Character::PostUpdate()
{

	if (m_nowAction)
	{
		m_nowAction->PostUpdate(m_wpThis);
	}

	auto progress = m_spAnimator->GetProgress();
	Application::Instance().m_log.AddLog("AnimProgress:%.1f\n", progress);


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

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld, m_modelColor,m_emissiveColor);

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

	auto& am = KdAudioManager::Instance();
	am.Play("Asset/Sounds/SE/PlayerHit.wav")->SetVolume(am.GetSEVolume());
}

void Character::Editor_ImGui()
{
	CharacterBase::Editor_ImGui();

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

	ImGui::DragFloat3("TransColor", &m_TACColor.x, 0.01f);
	ImGui::DragFloat("TransProgress", &m_TACProg, 0.01f,0.0f,1.0f);
	ImGui::DragFloat("TransSpeed", &m_TACspeed, 0.01f,0.01f);

}

void Character::Deserialize(const nlohmann::json& jsonObj)
{
	CharacterBase::Deserialize(jsonObj);

}

void Character::Serialize(nlohmann::json& outJson) const
{
	CharacterBase::Serialize(outJson);
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

	if (mouse.leftButton || pad.IsRightTriggerPressed())
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

	if (mouse.rightButton || pad.IsLeftTriggerPressed())
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

const bool Character::IsRStick()
{
	auto& mouse = KeyInput::GetInstance().GetMouseState();
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	if (mouse.scrollWheelValue || pad.IsRightStickPressed())
	{
		return true;
	}
	return false;
}

const bool Character::IsLStick()
{
	auto& key = KeyInput::GetInstance().GetKeyboard();
	auto& keyTrack = KeyInput::GetInstance().GetKeyboardTracker();
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	if (keyTrack.IsKeyPressed(key.LeftControl) || pad.IsLeftStickPressed())
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

	auto& cm = CameraManager::Instance();
	if (cm.GetNowType() != CameraManager::Lock)
	{

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
				//if (m_nowAction->GetState() == CharacterStateName::BoostDush)
				//{
				//	ang = 5.0f;
				//	//右回転
				//	m_rot.y -= ang;
				//}
				//else
				{

					//右回転
					m_rot.y -= ang;
				}
			}
			else
			{
			/*	if (m_nowAction->GetState() == CharacterStateName::BoostDush)
				{
					ang = 5.0f;
				
					m_rot.y += ang;
				}
				else*/ 
				{
					//左回転
					m_rot.y += ang;
				}
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
	else {
		auto& rot = cm.GetCurrentCamera().lock()->GetDeg();
		m_rot.y = rot.y;
	}

}

void Character::UpdateCollision()
{
	// その他球による衝突判定
	// ---- ---- ---- ---- ---- ----
	// ①当たり判定(球判定)用の情報を作成
	KdCollider::SphereInfo sphereInfo;
	sphereInfo.m_sphere.Center = m_pos + Math::Vector3(0, 1.79f, 0);
	sphereInfo.m_sphere.Radius = 1.79f;
	sphereInfo.m_type = KdCollider::TypeGround;

	Math::Vector3 pos;
	if (SphereCast(sphereInfo.m_sphere.Center, sphereInfo.m_sphere.Radius, KdCollider::TypeGround, pos)) {
		m_pos = pos;
	}
	
	DirectX::BoundingOrientedBox box;

	box.Center = m_pos + m_correctionMat.Translation();
	box.Extents = { 2.0f,6.0f,2.0f };
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

	Application::Instance().m_log.AddLog("pos x:%.2f,y:%.2f,z:%.2f\n", translation.x, translation.y, translation.z);

	m_pDebugWire->AddDebugSphere(sphereInfo.m_sphere.Center, sphereInfo.m_sphere.Radius, color);
	m_pDebugWire->AddDebugBox(m_correctionMat * m_mWorld, box.Extents, {}, true, color);

}

void Character::LockOn()
{
	KdCollider::SphereInfo sphereInfo;

	sphereInfo.m_sphere.Center = GetPos() + Math::Vector3(0, 3.5f, 0);
	sphereInfo.m_sphere.Radius = 200.0f;
	sphereInfo.m_type = KdCollider::TypeDamage;


	//
	m_wpCharacterTarget.reset();
	CameraManager::Instance().ResetMultiLocks();


	std::vector<std::shared_ptr<CameraManager::LockTargetInfo>> vec;

	// ②HIT対象オブジェクトに総当たり
	for (auto& obj : SceneManager::Instance().GetEnemyList())
	{
		if (obj->IsDestroy() == true) 
		{ continue; }


		if (obj->Intersects(sphereInfo, nullptr))
		{
			std::shared_ptr<CameraManager::LockTargetInfo> info = std::make_shared<CameraManager::LockTargetInfo>();
			auto targetPos = obj->GetMatrix().Translation();
			targetPos.y = 0.0f;
			auto pos = m_mWorld.Translation();
			pos.y = 0.0f;
			float distance = (targetPos - pos).Length();

			auto& camMat = CameraManager::Instance().GetCurrentCamera().lock()->GetMatrix();

			if (SearchDetect(targetPos, camMat, 80) == true)
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
}

bool Character::IsIgnoreGravityState() const
{
	// 判定
	if (m_nowAction->GetState() == CharacterStateName::Fly ||
		m_nowAction->GetState() == CharacterStateName::FlyGuard ||
		m_nowAction->GetState() == CharacterStateName::BoostFloat||
		m_nowAction->GetState() == CharacterStateName::BoostFloatGuard
		) {
		return true;
	}

	return false;
}

void Character::WalkSounds()
{
	if (!m_spAnimator) { return; }
	if (!m_spModelWork) { return; }
	if (m_spAnimator->IsComp()) { return; }
	if (m_spAnimator->GetProgress() == 0.0f || m_spAnimator->GetProgress() >= 0.5f && m_spAnimator->GetProgress() <= 0.51f) {
		m_isWalkSounds = true;
	}

	if (!m_isWalkSounds) { return; }

	bool flg = false;

	KdModelWork::Node* pNode = m_spModelWork->FindWorkNode("RTS");
	if (pNode)
	{
		auto mat = pNode->m_worldTransform * m_mWorld;
		auto pos = mat.Translation();
		Math::Vector3 result = {};
		flg = SphereCast(pos, 0.2f, KdCollider::TypeGround, result);
		m_pDebugWire->AddDebugSphere(pos, 0.2f, color);

	}

	if (flg) {
		m_isWalkSounds = false;

		auto& am = KdAudioManager::Instance(); 
		am.Play("Asset/Sounds/Sound/walk_2.wav", false)->SetVolume(am.GetSEVolume());
		return;
	}

	pNode = m_spModelWork->FindWorkNode("LTS");
	if (pNode)
	{
		auto mat = pNode->m_worldTransform * m_mWorld;
		auto pos = mat.Translation();
		Math::Vector3 result = {};
		flg = SphereCast(pos, 0.2f, KdCollider::TypeGround, result);
		m_pDebugWire->AddDebugSphere(pos, 0.2f, color);
	}

	if (flg) {
		m_isWalkSounds = false;

		auto& am = KdAudioManager::Instance();
		am.Play("Asset/Sounds/Sound/walk_2.wav", false)->SetVolume(am.GetSEVolume());
		return;
	}

}

void Character::CreatePolygon()
{
	auto poly = std::make_shared<ScaleEffect>();
	KdModelWork::Node* pNode =m_spModelWork->FindWorkNode("CBP");
	if (pNode)
	{
		auto pNodeMat = pNode->m_worldTransform;
		auto mat = Math::Matrix::CreateRotationX(87 * KdToRadians) * pNodeMat * m_mWorld;
	//	auto mat =  pNodeMat * m_mWorld;
		poly->SetRotation({90.0f,0.0f,0.0f });
		poly->SetMatrix(mat);
	}
	poly->SetPolygonParam("Asset/Textures/GameObject/Thurster.png", 2.0f, 0.2f);

	SceneManager::Instance().AddObject(poly);
}

const bool Character::SwordRangeCheck()
{
	DirectX::BoundingOrientedBox box;
	box.Center = { 0.0f,5.0f,9.0f };
	box.Extents = { 8.0f, 9.0f, 8.0f };
	auto type = KdCollider::TypeDamage;
	KdCollider::BoxInfo boxInfo(type,box);

	for (auto& obj : SceneManager::Instance().GetEnemyList())
	{
		if (obj->Intersects(boxInfo, nullptr))
		{
			return true;
		}
	}

	return false;
}


void Character::OverTrans(const std::string& nowAnimName,const float animProgress)
{
	m_hp--;
	
	std::shared_ptr<TransAC> trans = std::make_shared<TransAC>();
	std::string modelpath = "Asset/Models/Grint/Grint.gltf";
	std::string animpath = nowAnimName;
	trans->SetTransACData(modelpath, animpath, animProgress, m_mWorld, m_TACProg, m_TACspeed, m_TACColor);
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
	if (direction.LengthSquared() <= 0.01f) {
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

void Character::ActionStateBase::Trans(std::weak_ptr<Character>& owner, const float animProgress)
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

	//spOwner->ChangeEnableLeftAttack(false);
	spOwner->ChangeEnableRightAttack(false);
	//spOwner->ChangeEnableLeftShoulderAttack(false);
	spOwner->ChangeEnableRightShoulderAttack(false);


	m_isMove = spOwner->IsMove();
	m_isBoost = spOwner->IsBoost();
	m_isFlow = spOwner->IsFlow();
	m_isRightAttack = spOwner->IsAttack();
	m_isGuard = spOwner->IsGuard();
	m_isLeftShoulder = spOwner->IsLeftShoulder();
	m_isRightShoulder = spOwner->IsRightShoulder();
	/*if (m_isRightAttack)
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
	}*/
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
void Character::ActionStart::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Start"), 5.0f,false);

	m_direction = Math::Vector3::Zero;

	m_speed = 0.0f * spOwner->m_speedMag;

	m_animName = "Start";

	m_stateNum = spOwner->CharacterStateName::Start;
}

void Character::ActionStart::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("Start\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<ActionIdle>());
		return;
	}
}

void Character::ActionStart::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 60.0f * spOwner->m_speedMag);

}

void Character::ActionStart::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 5.0f * spOwner->m_speedMag);

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

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandUp"), 15.0f, false);

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
		if (spOwner->m_prvAction->GetState() == CharacterStateName::BoostEnd || spOwner->m_prvAction->GetState() == CharacterStateName::BoostDush)
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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 80.0f * spOwner->m_speedMag);

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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 30.0f * spOwner->m_speedMag);

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

	m_animName = "Stand";

	m_stateNum = spOwner->CharacterStateName::Fly;

	std::string str = "burstFligit";

	auto& am = KdAudioManager::Instance();
	spOwner->m_sounds["burstFlight"] = am.Play("Asset/Sounds/Sound/burst_flight.wav", true);
	spOwner->m_sounds["burstFlight"].lock()->SetVolume(am.GetSEVolume());



}

void Character::ActionJump::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("Fly\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	m_direction = {};

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

		if (m_isGuard) {
			spOwner->ChangeActionState(std::make_shared<ActionJumpShield>());
			return;
		}

		if (m_isMove)
		{
			m_direction = Direct(owner, true);
		}
		m_direction += Math::Vector3::Up;

		m_direction.Normalize();

		auto flg = spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround, false, false, false, true);
		if (flg) {
			Application::Instance().m_log.AddLog("FetchSuccess\n");
		}
		else {
			Application::Instance().m_log.AddLog("Move\n");
		}
		return;
	}

	if (spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}

	spOwner->ChangeActionState(std::make_shared<ActionFall>());
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
	std::shared_ptr<Character> spOwner = owner.lock();

	auto flight = spOwner->m_sounds["burstFlight"].lock();

	if (flight) {
		flight->Stop();
	}

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//空中盾構え状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionJumpShield::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandShield"), 5.0f, false);


	m_speed = spOwner->m_jumpSpeed * spOwner->m_speedMag;

	
	m_animName = "StandShield";

	m_stateNum = spOwner->CharacterStateName::FlyGuard;
	spOwner->ChangeEnableLeftAttack(true);

}

void Character::ActionJumpShield::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("FlyShield\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	m_direction = {};

	// ガードしていない状況
	if (m_isFlow && !m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionJump>());
		return;
	}

	// ガードしている状況
	if ( m_isGuard)
	{
		if (!m_isFlow)
		{
			spOwner->ChangeActionState(std::make_shared<ActionBoostFallShield>());
			return;
		}
		else
		{
			if (m_isMove)
			{
				m_direction = Direct(owner, true);
			}

			m_direction += Math::Vector3::Up;
			auto flg = spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround, false, false, false, true);
			if (flg) {
				Application::Instance().m_log.AddLog("FetchSuccess\n");
			}
			else {
				Application::Instance().m_log.AddLog("Move\n");
			}
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
		spOwner->ChangeActionState(std::make_shared<ActionFall>());
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
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 30.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());

}

void Character::ActionJumpShield::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	
	spOwner->ChangeEnableLeftAttack(false);

}

// 落下
void Character::ActionFall::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Stand"), 10.0f);

	m_speed = spOwner->m_jumpSpeed * spOwner->m_speedMag;

	Checkkey(owner);

	m_animName = "Stand";

	m_stateNum = spOwner->CharacterStateName::Fall;

}

void Character::ActionFall::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("Fall\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);
	
	m_direction = {};

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
	
	if (m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoostFallShield>());
		return;
	}


	if (spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}

	if (m_isMove)
	{

		m_direction = Direct(owner, true); 
		auto flg =
			spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);
		if (flg) {
			Application::Instance().m_log.AddLog("FetchSuccess\n");
		}
		else {
			Application::Instance().m_log.AddLog("Move\n");
		}
	}

}

void Character::ActionFall::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 30.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}

void Character::ActionFall::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

}

void Character::ActionBoostFallShield::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("StandShield"), 5.0f, false);


	m_speed = spOwner->m_jumpSpeed * spOwner->m_speedMag;

	m_animName = "StandShield";

	m_stateNum = spOwner->CharacterStateName::FallGuard;
	spOwner->ChangeEnableLeftAttack(true);
}

void Character::ActionBoostFallShield::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("FallGuard\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	m_direction = {};

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

	if (spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}

	if (m_isMove)
	{

		m_direction = Direct(owner, true);
		auto flg =
			spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);
		if (flg) {
			Application::Instance().m_log.AddLog("FetchSuccess\n");
		}
		else {
			Application::Instance().m_log.AddLog("Move\n");
		}
	}
}

void Character::ActionBoostFallShield::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 30.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}

void Character::ActionBoostFallShield::Exit(std::weak_ptr<Character>& owner)
{
	auto spOwner = owner.lock();
	spOwner->ChangeEnableLeftAttack(false);
}


//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//歩行状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionMove::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	if (CameraManager::Instance().GetNowType() != CameraManager::Lock)
	{
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Walk"), 6.0f, true);
	}
	else {
		Checkkey(owner);
		auto& move = spOwner->m_vMove;
		if (move.x > 0) {
			if (move.z < 0) {
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BackWalk"), 6.0f, true);
			}
			else if (move.z > 0) {
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Walk"), 6.0f, true);
			}
			else {
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("RightWalk"), 6.0f, true);
			}
		}
		else if (move.x < 0) {

			if (move.z < 0) {
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BackWalk"), 6.0f, true);
			}
			else if (move.z > 0) {
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Walk"), 6.0f, true);
			}
			else
			{
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("LeftWalk"), 6.0f, true);
			}
		}
		else
		{
			if (move.z < 0) {
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BackWalk"), 6.0f, true);
			}
			else if (move.z > 0) {
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Walk"), 6.0f, true);
			}
		}

	}

	m_speed = spOwner->m_walkSpeed * spOwner->m_speedMag;

	m_animName = "Walk";

	m_stateNum = spOwner->CharacterStateName::Walk;



}

void Character::ActionMove::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("Walk\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	if (CameraManager::Instance().GetNowType() == CameraManager::Lock)
	{	
		auto& move = spOwner->m_vMove;

		m_prevType = m_type;

		if (move.x > 0) {
			if (move.z < 0) {
				m_type = Back;
				if (m_type != m_prevType)
				{
					spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BackWalk"), 6.0f, true);
				}
			}
			else if (move.z > 0) {
				m_type = Front;
				if (m_type != m_prevType)
				{
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Walk"), 6.0f, true);
				}
			}
			else {
				m_type = Right;
				if (m_type != m_prevType)
				{
					spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("RightWalk"), 6.0f, true);
				}
			}
		}
		else if (move.x < 0) {

			if (move.z < 0) {
				m_type = Back;
				if (m_type != m_prevType)
				{
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BackWalk"), 6.0f, true);
				}
			}
			else if (move.z > 0) {
				m_type = Front;
				if (m_type != m_prevType)
				{
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Walk"), 6.0f, true);
				}
			}
			else
			{
				m_type = Left;
				if (m_type != m_prevType)
				{
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("LeftWalk"), 6.0f, true);
				}
			}
		}
		else
		{
			if (move.z < 0) {
				m_type = Back;
				if (m_type != m_prevType)
				{
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BackWalk"), 6.0f, true);
				}
			}
			else if (move.z > 0) {
				m_type = Front;
				if (m_type != m_prevType)
				{
				spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Walk"), 6.0f, true);
				}
			}
		}

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
		m_direction = Direct(owner, true);

		auto flg =
			spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);
		if (flg) {
			Application::Instance().m_log.AddLog("FetchSuccess\n");
		}
		else {
			Application::Instance().m_log.AddLog("Move\n");
		}
	}



}

void Character::ActionMove::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->WalkSounds();
	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 85.0f * spOwner->m_speedMag);


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


		auto flg =
			spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);
		if (flg) {
			Application::Instance().m_log.AddLog("FetchSuccess\n");
		}
		else {
			Application::Instance().m_log.AddLog("Move\n");
		}

	}

}

void Character::ActionMoveShield::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->WalkSounds();
	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 60.0f * spOwner->m_speedMag);

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

	auto& move = spOwner->m_vMove;

	if (CameraManager::Instance().GetNowType() == CameraManager::Lock) {

		if (move.x > 0) {
			spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostRight"), 10.0f, false);
		}
		else if (move.x < 0) {
			spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostLeft"), 10.0f, false);

		}
		if (move.z > 0) {
			spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Boost"), 10.0f, false);
		}
		else if (move.z < 0) {
			spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostBack"), 10.0f, false);
		}
	}
	else {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Boost"), 10.0f, false);
	}


	m_direction = ActionStateBase::Direct(owner, true);

	m_speed = spOwner->m_boostSpeed * spOwner->m_speedMag;

	//エフェクシア
	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("CBP");
		if (pNode)
		{

			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "Thruster.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			auto mat = effect->pNodeMat * spOwner->m_mWorld;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("Thruster.efkefc", mat.Translation(), 3.0f, 1.0f);
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);

			//効果音
			auto& am = KdAudioManager::Instance();
			am.Play("Asset/Sounds/Sound/burst_start.wav")->SetVolume(am.GetSEVolume());

			/*	auto instance = KdAudioManager::Instance().Play3D("Asset/Sounds/Thruster2.wav", spOwner->GetPos());
				auto vec = CameraManager::Instance().ToCameraVec(mat.Translation());
				instance->SetVolume(1.0f);
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

	if (spOwner->m_spAnimator->GetProgress() <= 0.5f)
	{

		auto sin = std::sinf(spOwner->m_spAnimator->GetProgress() * DirectX::XM_2PI);


		KdShaderManager::Instance().m_postProcessShader.SetRadialBlurInfo(8, sin, { 0.5f,0.5f }, 0.15f, 0, 0.0f);
		UINT kind = KdShaderManager::Instance().m_postProcessShader.RadialBlur;
		KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	}

	if (m_isRightAttack) {
		spOwner->ChangeActionState(std::make_shared<ActionRightAttack>());
		return;
	}

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		//if (spOwner->m_isGround)
		{
			spOwner->ChangeActionState(std::make_shared<ActionBoostEnd>());
			return;
		}
		/*else
		{
			spOwner->ChangeActionState(std::make_shared<ActionBoostNow>());
			return;
		}*/

	}

	bool flg = spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);

	if (flg) {
		Application::Instance().m_log.AddLog("FetchSuccess\n");
	}
	else {
		Application::Instance().m_log.AddLog("Move\n");
	}

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

	//	if (spOwner->m_spAnimator->GetProgress() > 0.4f) 
	{
		KdShaderManager::Instance().m_postProcessShader.UndoRadialBlur();
		auto kind = KdShaderManager::Instance().m_postProcessShader.Normal;
		KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	}
	//	else

}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト中状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionBoostNow::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("Hoboor"), 4.0f);

	m_speed = spOwner->m_boostSpeed * spOwner->m_speedMag;


	//エフェクト
	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("CBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "Thruster.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("Thruster.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 3.0f, 1.0f);
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
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaL.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 0.4f);
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
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaR.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 0.4f);
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	m_animName = "BoostNow";

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
		spOwner->ChangeActionState(std::make_shared<ActionBoostFloat>());
		return;
	}



	{

		m_direction = ActionStateBase::Direct(owner, false);

		auto flg = spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);

		if (flg) {
			Application::Instance().m_log.AddLog("FetchSuccess\n");
		}
		else {
			Application::Instance().m_log.AddLog("Move\n");
		}
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

	//エフェクト

	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("LLBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "BarniaL.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaL.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 0.4f);
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
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaR.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 0.4f);
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

	if (!spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoostFall>());
		return;
	}

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
		spOwner->ChangeActionState(std::make_shared<ActionBoostFloat>());
		return;
	}

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}

	if (spOwner->m_prvAction->GetMove() == MoveType::Front)
	{
		m_direction = ActionStateBase::Direct(owner, false);
	}
	else  if (spOwner->m_prvAction->GetMove() == MoveType::Back) {
		auto vec = spOwner->m_mWorld.Forward();
		vec.Normalize();
		m_direction = vec;
	}
	else  if (spOwner->m_prvAction->GetMove() == MoveType::Right) {
		auto vec = spOwner->m_mWorld.Right();
		vec.Normalize();
		m_direction = vec;
	}
	else  if (spOwner->m_prvAction->GetMove() == MoveType::Left) {
		auto vec = spOwner->m_mWorld.Left();
		vec.Normalize();
		m_direction = vec;
	}

	
	// イージング
	auto progress = spOwner->m_spAnimator->GetProgress() * DirectX::XM_PI;

	m_easeSpeed = m_speed - (m_speed * progress);

	auto flg =
		spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);
	if (flg) {
		Application::Instance().m_log.AddLog("FetchSuccess\n");
	}
	else {
		Application::Instance().m_log.AddLog("Move\n");
	}

	//エフェクト
	EffectUpdate(owner);
}

void Character::ActionBoostEnd::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 50.0f * spOwner->m_speedMag);

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

	auto& move = spOwner->m_vMove;

	if (CameraManager::Instance().GetNowType() == CameraManager::Lock) {

		if (move.x > 0) {
			m_type = Right;
			spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("DushRight"), 10.0f, false);
		}
		else if (move.x < 0) {
			m_type = Left;
			spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("DushLeft"), 10.0f, false);

		}
		if (move.z > 0) {
			m_type = Front;
			spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostDush"), 10.0f, false);
		}
		else if (move.z < 0) {
			m_type = Back;
			spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("DushBack"), 10.0f, false);
		}
	}
	else {
		spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostDush"), 2.0f);
	}

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
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaL.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 0.4f);
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
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaR.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 0.4f);
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


	if (CameraManager::Instance().GetNowType() == CameraManager::Lock)
	{
		auto& move = spOwner->m_vMove;

		m_prevType = m_type;

		if (move.x > 0) {
			if (move.z < 0) {
				m_type = Back;
				if (m_type != m_prevType)
				{
					spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("DushBack"), 6.0f, true);
				}
			}
			else if (move.z > 0) {
				m_type = Front;
				if (m_type != m_prevType)
				{
					spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostDush"), 6.0f, true);
				}
			}
			else {
				m_type = Right;
				if (m_type != m_prevType)
				{
					spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("DushRight"), 6.0f, true);
				}
			}
		}
		else if (move.x < 0) {

			if (move.z < 0) {
				m_type = Back;
				if (m_type != m_prevType)
				{
					spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("DushBack"), 6.0f, true);
				}
			}
			else if (move.z > 0) {
				m_type = Front;
				if (m_type != m_prevType)
				{
					spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostDush"), 6.0f, true);
				}
			}
			else
			{
				m_type = Left;
				if (m_type != m_prevType)
				{
					spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("DushLeft"), 6.0f, true);
				}
			}
		}
		else
		{
			if (move.z < 0) {
				m_type = Back;
				if (m_type != m_prevType)
				{
					spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("DushBack"), 6.0f, true);
				}
			}
			else if (move.z > 0) {
				m_type = Front;
				if (m_type != m_prevType)
				{
					spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostDush"), 6.0f, true);
				}
			}
		}

	}


	if (m_isGuard)
	 {
		if (m_isFlow)
		{
			spOwner->ChangeActionState(std::make_shared<ActionBoostFloatShield>());
			return;
		}

		if (!m_isMove)
		{
			spOwner->ChangeActionState(std::make_shared<ActionStandShield>());
			return;
		}

		spOwner->ChangeActionState(std::make_shared<ActionBoostShield>());
		return;
	}
	
	if (m_isFlow)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoostFloat>());
		return;
	}

	if (!spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoostFall>());
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


	if (m_isLeftShoulder)
	{
		spOwner->ChangeActionState(std::make_shared<ActionLeftShoulderAttack>());
		return;
	}

	//移動中に何も入力がなければ待機に移行
	if (!m_isMove) {
		spOwner->ChangeActionState(std::make_shared<ActionBoostDushEnd>());
		return;
	}
	else
	{
		m_direction = ActionStateBase::Direct(owner, true);
	}


	auto flg =
		spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);
	if (flg) {
		Application::Instance().m_log.AddLog("FetchSuccess\n");
	}
	else {
		Application::Instance().m_log.AddLog("Move\n");
	}

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
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaL.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 0.4f);
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
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("BarniaR.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 0.4f);
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
		spOwner->ChangeActionState(std::make_shared<ActionBoostFloat>());
		return;
	}

	if (m_isFlow && m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoostFloatShield>());
		return;
	}

	if (!spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoostFall>());
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
		spOwner->ChangeActionState(std::make_shared<ActionBoostDushEnd>());
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


	auto flg =
		spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);
		if (flg) {
			Application::Instance().m_log.AddLog("FetchSuccess\n");
		}
		else {
			Application::Instance().m_log.AddLog("Move\n");
		}

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
//ブースト上昇状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionBoostFloat::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostDush"), 10.0f);

	m_speed = spOwner->m_boostDushSpeed * spOwner->m_boostFloatMeg * spOwner->m_speedMag;

	Checkkey(owner);

	m_animName = "BoostDush";

	m_stateNum = spOwner->CharacterStateName::BoostFloat;

	std::string str = "burstFligit";

	auto& am = KdAudioManager::Instance();
	spOwner->m_sounds["burstFlight"] = am.Play("Asset/Sounds/Sound/burst_flight.wav", true);
	spOwner->m_sounds["burstFlight"].lock()->SetVolume(am.GetSEVolume());

}

void Character::ActionBoostFloat::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("BoostFly\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	m_direction = {};

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
		if (m_isMove)
		{
			m_direction = Direct(owner, true);
		}
		m_direction += Math::Vector3::Up;

		m_direction.Normalize();

		auto flg =
		spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround, false, false, false, true);
			if (flg) {
				Application::Instance().m_log.AddLog("FetchSuccess\n");
			}
			else {
				Application::Instance().m_log.AddLog("Move\n");
			}

		return;
	}

	if (spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}

	spOwner->ChangeActionState(std::make_shared<ActionBoostFall>());
	return;
}

void Character::ActionBoostFloat::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());

}

void Character::ActionBoostFloat::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	auto flight = spOwner->m_sounds["burstFlight"].lock();

	if (flight) {
		flight->Stop();
	}

	spOwner->UnEnableTrail();
}

//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
//ブースト上昇ガード付き状態
//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
void Character::ActionBoostFloatShield::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostDushShield"), 10.0f);

	m_speed = spOwner->m_boostDushSpeed * spOwner->m_boostFloatMeg * spOwner->m_speedMag;

	Checkkey(owner);

	m_animName = "BoostDushShield";

	m_stateNum = spOwner->CharacterStateName::BoostFloatGuard;

	std::string str = "burstFligit";

	auto& am = KdAudioManager::Instance();
	spOwner->m_sounds["burstFlight"] = am.Play("Asset/Sounds/Sound/burst_flight.wav", true);
	spOwner->m_sounds["burstFlight"].lock()->SetVolume(am.GetSEVolume());

}

void Character::ActionBoostFloatShield::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("BoostFlyShield\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	Checkkey(owner);

	// ガードしていない状況
	if (!m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionBoostFall>());
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
			m_direction += Direct(owner, true);
			m_direction += Math::Vector3::Up;

			m_direction.Normalize();

			auto flg =
				spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround, false, false, false, true);
			if (flg) {
				Application::Instance().m_log.AddLog("FetchSuccess\n");
			}
			else {
				Application::Instance().m_log.AddLog("Move\n");
			}

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

	if (spOwner->m_spAnimator->IsAnimationEnd() && spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}
}

void Character::ActionBoostFloatShield::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 10.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());

}

void Character::ActionBoostFloatShield::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	auto flight = spOwner->m_sounds["burstFlight"].lock();

	if (flight) {
		flight->Stop();
	}

	
}

void Character::ActionBoostFall::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BoostDush"), 10.0f);

	m_speed = spOwner->m_boostDushSpeed * spOwner->m_boostFloatMeg * spOwner->m_speedMag;

	Checkkey(owner);

	m_animName = "BoostDush";

	m_stateNum = spOwner->CharacterStateName::BoostFall;
}

void Character::ActionBoostFall::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("BoostFall\n");

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
		spOwner->ChangeActionState(std::make_shared<ActionBoostFloat>());
		return;
	}
	if (m_isGuard)
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandShield>());
		return;
	}

	if (spOwner->m_isGround)
	{
		spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
		return;
	}

	if (m_isMove)
	{
		m_direction = Direct(owner, true);
		auto flg =
		spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround, false);
			if (flg) {
				Application::Instance().m_log.AddLog("FetchSuccess\n");
			}
			else {
				Application::Instance().m_log.AddLog("Move\n");
			}
	}
}

void Character::ActionBoostFall::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 30.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());

}

void Character::ActionBoostFall::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->UnEnableTrail();
}



//ブースト移動終了
void Character::ActionBoostDushEnd::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("DushEnd"), 15.0f, false);
	m_speed = spOwner->m_boostDushSpeed * spOwner->m_speedMag;

	//エフェクト

	std::shared_ptr<Effect> effect = std::make_shared<Effect>();
	effect->name = "Spark.efkefc";
	effect->pNodeMat = Math::Matrix::Identity;
	effect->wpEffect = KdEffekseerManager::GetInstance().Play("Spark.efkefc", spOwner->m_mWorld.Translation(), 0.10f, 5.0f);
	effect->handle = effect->wpEffect.lock()->GetHandle();
	m_spEffects.push_back(effect);

	m_direction = Direct(owner, false);

	m_animName = "DushEnd";

	m_stateNum = spOwner->CharacterStateName::DushEnd;
}

void Character::ActionBoostDushEnd::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("DushEnd\n");

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
		if (spOwner->m_spAnimator->IsAnimationEnd()) {
			spOwner->ChangeActionState(std::make_shared<ActionStandUp>());
			return;
		}
	}
	else
	{	
		if (spOwner->m_spAnimator->GetProgress() < 0.3f) {
			if (spOwner->m_isGround) {
				spOwner->ChangeActionState(std::make_shared<ActionBoostDush>());
				return;
			}
		}
		else {
				spOwner->ChangeActionState(std::make_shared<ActionMove>());
				return;
		}
	}

	if (CameraManager::Instance().GetNowType() == CameraManager::Lock)
	{
		if (spOwner->m_prvAction->GetMove() == MoveType::Front)
		{
			m_direction = ActionStateBase::Direct(owner, false);
		}
		else  if (spOwner->m_prvAction->GetMove() == MoveType::Back) {
			auto vec = spOwner->m_mWorld.Forward();
			vec.Normalize();
			m_direction = vec;
		}
		else  if (spOwner->m_prvAction->GetMove() == MoveType::Right) {
			auto vec = spOwner->m_mWorld.Right();
			vec.Normalize();
			m_direction = vec;
		}
		else  if (spOwner->m_prvAction->GetMove() == MoveType::Left) {
			auto vec = spOwner->m_mWorld.Left();
			vec.Normalize();
			m_direction = vec;
		}
	}
	

	auto progress = spOwner->m_spAnimator->GetProgress() * DirectX::XM_PI;
	if (progress > 1.0f) {
		progress = 1.0f;
	}

	m_easeSpeed = m_speed - (m_speed * progress);
	Application::Instance().m_log.AddLog("EaseSpeed:%.2f\n",m_easeSpeed);

	auto flg =
	spOwner->MoveSwept(m_easeSpeed, m_direction, KdCollider::TypeGround);
		if (flg) {
			Application::Instance().m_log.AddLog("FetchSuccess\n");
		}
		else {
			Application::Instance().m_log.AddLog("Move\n");
		}
	//エフェクト
	if (spOwner->m_spAnimator->GetProgress() < 0.6f) {

		EffectUpdate(owner);
	}
	else {
		EffectExit();
	}


}

void Character::ActionBoostDushEnd::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 60.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());

}

void Character::ActionBoostDushEnd::Exit(std::weak_ptr<Character>& owner)
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

	spOwner->LockOn();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("RightBladeAttackBef"), 20.0f, false);

	m_direction = ActionStateBase::Direct(owner, true);

	m_speed = spOwner->m_bladeAttackSpeed * spOwner->m_speedMag;

	spOwner->UnEnableTrail();

	auto& am = KdAudioManager::Instance();
	am.Play("Asset/Sounds/Sound/burst_start.wav")->SetVolume(am.GetSEVolume());

	//エフェクト
	{
		KdModelWork::Node* pNode = spOwner->m_spModelWork->FindWorkNode("CBP");
		if (pNode)
		{
			std::shared_ptr<Effect> effect = std::make_shared<Effect>();
			effect->name = "Thruster.efkefc";
			effect->pNodeMat = pNode->m_worldTransform;
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("Thruster.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 3.0f, 1.0f);
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	m_animName = "RightBladeAttackBef";

	m_stateNum = spOwner->CharacterStateName::RightSorwdBef;

	m_isDuration = true;
}

void Character::ActionRightAttack::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("AttackBef\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	Checkkey(owner);

	if (!m_isRightAttack)
	{
		if (m_isDuration) {
			m_isDuration = false;
		}
	}

	spOwner->ChangeEnableRightAttack(true);

	if (spOwner->m_spAnimator->GetProgress() > 0.6f)
	{
		if (m_isBoost)
		{
			spOwner->ChangeEnableRightAttack(false);
			spOwner->ChangeEnableAttack(false);
			spOwner->ChangeActionState(std::make_shared<ActionBoost>());
			return;
		}
	}

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		if (!m_isDuration)
		{
			spOwner->ChangeActionState(std::make_shared<ActionRightAttackMid>());
			return;
		}
		else {
			spOwner->ChangeActionState(std::make_shared<ActionRightAttackCharge>());
			return;
		}
	}

	if (spOwner->SwordRangeCheck()) {
			spOwner->ChangeActionState(std::make_shared<ActionRightAttackAf>());
			return;

	}


	//敵が一定範囲内なら敵のほうに向いて敵に
	auto target = spOwner->m_wpCharacterTarget.lock();
	if (target)
	{
		Math::Vector3 targetPos = target->GetCorrectionMatrix().Translation() + target->GetMatrix().Translation();
		m_direction = targetPos - spOwner->GetPos();
		m_direction.Normalize();
	}

	//owner.Move(m_speed, m_direction, KdCollider::TypeDamage, true);
	auto flg =
	spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);
		if (flg) {
			Application::Instance().m_log.AddLog("FetchSuccess\n");
		}
		else {
			Application::Instance().m_log.AddLog("Move\n");
		}

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
			effect->wpEffect = KdEffekseerManager::GetInstance().Play("Thruster.efkefc", pNode->m_worldTransform.Translation() * spOwner->m_mWorld.Translation(), 3.0f, 1.0f);
			effect->handle = effect->wpEffect.lock()->GetHandle();
			m_spEffects.push_back(effect);
		}
	}

	m_animName = "RightSowrdMid";

	m_stateNum = spOwner->CharacterStateName::RightSorwdMid;


	spOwner->ChangeEnableRightAttack(true);
	spOwner->ChangeEnableAttack(true);

}

void Character::ActionRightAttackMid::Update(std::weak_ptr<Character>& owner)
{

	Application::Instance().m_log.AddLog("AttackNow\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	
	spOwner->ChangeEnableRightAttack(true);

	//敵が一定範囲内なら敵のほうに向いて敵に
	auto target = spOwner->m_wpCharacterTarget.lock();
	if (target)
	{
		Math::Vector3 targetPos = target->GetCorrectionMatrix().Translation() + target->GetMatrix().Translation();
		m_direction = targetPos - spOwner->GetPos();
		m_direction.Normalize();
	}

	if (spOwner->SwordRangeCheck()) {
		spOwner->ChangeActionState(std::make_shared<ActionRightAttackAf>());
		return;

	}

	{
		auto flg =
		spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);
			if (flg) {
				Application::Instance().m_log.AddLog("FetchSuccess\n");
			}
			else {
				Application::Instance().m_log.AddLog("Move\n");
			}
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
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("RightSowrd"), 40.0f, false);

	m_speed = spOwner->m_bladeAttackSpeed * spOwner->m_speedMag;

	m_direction = ActionStateBase::Direct(owner, false);

	m_stiffnessTime = 1.0f;

	m_animName = "RightSowrd";

	m_stateNum = spOwner->CharacterStateName::RightSorwdAf;	
}

void Character::ActionRightAttackAf::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("AttackAf\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	
	Checkkey(owner);

	//移動
	if (m_durationStiffness <= m_stiffnessTime / 8)
	{
		auto flg =
		spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);
			if (flg) {
				Application::Instance().m_log.AddLog("FetchSuccess\n");
			}
			else {
				Application::Instance().m_log.AddLog("Move\n");
			}

			spOwner->ChangeEnableRightAttack(true);

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
		if (m_isRightAttack) 
		{
			spOwner->ChangeActionState(std::make_shared<ActionRightAttackSecond>());
			return;
		}

		if (m_durationStiffness >= m_stiffnessTime / 2)
		{
			spOwner->ChangeEnableRightAttack(false);
		}

		return;
	}

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
	spOwner->UnEnableTrail();
	spOwner->ChangeEnableAttack(false);
	spOwner->ChangeEnableRightAttack(false);

}

void Character::ActionRightAttackSecond::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("RightBladeAttack2"), 40.0f, false);

	m_speed = spOwner->m_bladeAttackSpeed * spOwner->m_speedMag;

	m_direction = ActionStateBase::Direct(owner, false);

	m_stiffnessTime = 1.0f;

	m_animName = "RightBladeAttack2";

	m_stateNum = spOwner->CharacterStateName::RightSorwdSeco;

	spOwner->ChangeEnableRightAttack(true);

	auto& am = KdAudioManager::Instance();
	am.Play("Asset/Sounds/Sound/burst_start.wav")->SetVolume(am.GetSEVolume());


}

void Character::ActionRightAttackSecond::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("AttackSeco\n");

	std::shared_ptr<Character> spOwner = owner.lock();
	

	//移動
	if (!spOwner->m_spAnimator->IsAnimationEnd())
	{
		if (!spOwner->SwordRangeCheck())
		{
			auto flg =
				spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);
			if (flg) {
				Application::Instance().m_log.AddLog("FetchSuccess\n");
			}
			else {
				Application::Instance().m_log.AddLog("Move\n");
			}
		}
		
		spOwner->ChangeEnableRightAttack(true);
	}
	else
	{
		spOwner->UnEnableTrail();
		spOwner->ChangeEnableAttack(false);

		//加算
		m_durationStiffness += KdFPSController::GetInstance().GetDeltaTime();
	}

	//エフェクト
	EffectUpdate(owner);



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

void Character::ActionRightAttackSecond::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 150.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}

void Character::ActionRightAttackSecond::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	//エフェクト
	EffectExit();

	spOwner->UnEnableTrail();
	spOwner->ChangeEnableRightAttack(false);
}

void Character::ActionRightAttackCharge::Enter(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModelWork->GetData()->GetAnimation("BladeAccumulationAttack"), 40.0f, false);

	m_speed = spOwner->m_bladeAttackSpeed * spOwner->m_speedMag;

	m_direction = ActionStateBase::Direct(owner, false);

	m_stiffnessTime = 1.0f;

	m_animName = "BladeAccumulationAttack";

	m_stateNum = spOwner->CharacterStateName::RightSorwdCharge;

	spOwner->ChangeEnableRightAttack(true);
}

void Character::ActionRightAttackCharge::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("ChargeAttack\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->ChangeEnableRightAttack(true);

	if (spOwner->m_spAnimator->GetProgress() <= 0.6f)
	{
		if (!spOwner->SwordRangeCheck())
		{
			auto flg =
				spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround);
			if (flg) {
				Application::Instance().m_log.AddLog("FetchSuccess\n");
			}
			else {
				Application::Instance().m_log.AddLog("Move\n");
			}
		}
	}


	if (spOwner->m_spAnimator->IsAnimationEnd() == true)
	{
		spOwner->ChangeEnableAttack(false);

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

void Character::ActionRightAttackCharge::PostUpdate(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	// アニメーションの更新
	spOwner->m_spAnimator->AdvanceTime(spOwner->m_spModelWork->WorkNodes(), 60.0f * spOwner->m_speedMag);

	Trans(owner, spOwner->m_spAnimator->GetProgress());
}

void Character::ActionRightAttackCharge::Exit(std::weak_ptr<Character>& owner)
{
	std::shared_ptr<Character> spOwner = owner.lock();

	//エフェクト
	EffectExit();
	spOwner->UnEnableTrail();
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

	//KdAudioManager::Instance().Play("Asset/Sounds/Noize1.wav");

	m_animName = "Hited";

	m_stateNum = spOwner->CharacterStateName::Hited;

}

void Character::ActionHited::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("Hited\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround, false, false);

	//グリッチ表現
	auto time = KdFPSController::GetInstance().GetFPS() + spOwner->m_rand.GetFloat(0.0,4.0f);
	if (spOwner->m_spAnimator->GetProgress() <= 0.02f)
	{
		UINT kind = KdShaderManager::Instance().m_postProcessShader.Glitch;
		KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
		KdShaderManager::Instance().m_postProcessShader.
			SetGlitch({ 10,10 }, time, 4.5f, 0.8f, 0, 0, { 0.5f,0.5f });
	}

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

	auto& am = KdAudioManager::Instance();
	am.Play("Asset/Sounds/Sound/down_player.wav")->SetVolume(am.GetSEVolume());


	UIManager::GetInstance().SceneUICreate("Asset/Data/FailedUI.scene");
}

void Character::ActionDestroyed::Update(std::weak_ptr<Character>& owner)
{
	Application::Instance().m_log.AddLog("Destroy\n");

	std::shared_ptr<Character> spOwner = owner.lock();

	//グリッチ表現
	if (spOwner->m_spAnimator->IsAnimationEnd() == false) {
		auto time = KdFPSController::GetInstance().GetFPS();

		UINT kind = KdShaderManager::Instance().m_postProcessShader.Glitch;
		KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
		KdShaderManager::Instance().m_postProcessShader.
			SetGlitch({ 1,1 }, time, 5.0f, 0.8f, 0, 0, { 0.5f,0.5f });

	}
	else {
		CameraManager::Instance().ResetMultiLocks();
		SceneManager::Instance().SetNextScene(SceneManager::SceneType::Retry);
		UINT kind = KdShaderManager::Instance().m_postProcessShader.Normal;
		KdShaderManager::Instance().m_postProcessShader.SetCombine(kind);
	}


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

	m_stiffnessTime = 2.0f;

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
		spOwner->MoveSwept(m_speed, m_direction, KdCollider::TypeGround, false, false);
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

