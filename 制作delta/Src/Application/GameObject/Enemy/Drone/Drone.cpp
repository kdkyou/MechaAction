#include "Drone.h"

#include "../../../Scene/SceneManager.h"

#include "../../../main.h"

#include "../../UI/Alert/Alert.h"
#include "../../Camera/CameraManager.h"

void Drone::Init()
{
	m_limEnable = true;
	m_limColor = { 0.12f,0.09f,0.08f };
	m_limPow = 0.3f;
						
	m_correction = { 0.0f,1.0f,0.0f };

	m_correctionMat = Math::Matrix::CreateTranslation({0.0f,1.0f,0.0f});

	m_boxExtents = { 4.0f,3.0f,3.0f };

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape("Enemy", m_spModelWork, KdCollider::TypeDamage);

	m_dist = { 20.0f,80.0f };

	ChangeActionState(std::make_shared<Idle>());

	m_name = "Drone";

	m_hp = 200;

	m_viewAngle = 60.0f;

	m_burnPath = "Asset/Textures/GameObject/Burn.png";

	m_spMrkModel = KdAssets::Instance().m_modeldatas.GetData("Asset/Models/Marker/Enemy.gltf");


}

void Drone::Update()
{


	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpCharacterTarget.lock();

		if (!spTarget)
		{
			spTarget = m_wpTarget.lock();
		}

		std::shared_ptr<Drone> spThis = m_wpThis.lock();

		if (spThis == nullptr)
		{
			return;
		}
		m_nowAction->Update(m_wpThis, spTarget);

	}


	UpdateCollision();

	m_pDebugWire->AddDebugBox(m_mWorld, m_boxExtents);

}

void Drone::PostUpdate()
{
	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpCharacterTarget.lock();

		if (!spTarget)
		{
			spTarget = m_wpTarget.lock();
		}

		std::shared_ptr<Drone> spThis = m_wpThis.lock();

		if (spThis == nullptr)
		{
			return;
		}
		m_nowAction->PostUpdate(m_wpThis, spTarget);
	}

}

void Drone::DrawLit()
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

void Drone::OnHit()
{
	if (m_hp <= 0)
	{
		ChangeActionState(std::make_shared<Destroyed>());
		m_parameter = 0;
		return;
	}


}

void Drone::UpdateRotate(const Math::Vector3& srcMoveVec)
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

void Drone::UpdateCollision()
{
	DirectX::BoundingOrientedBox box;

	box.Center = GetPos();
	box.Extents = m_boxExtents;
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
				HitDamage(obj->GetParameter());
				m_parameter = obj->GetParameter();
				OnHit();
			}
		}
	}



}

bool Drone::Search(bool areaOnly)
{
	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = m_mWorld.Translation() + m_correction;
	sphere.m_sphere.Radius = m_dist.y;
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

	if (retList.empty() == true) { 
		return false; }

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
			if (obj)
			{
				auto toEnemy = m_mWorld.Translation() - obj->GetPos();
				toEnemy.Normalize();

				auto playerView = obj->GetMatrix().Backward();
				playerView.Normalize();

				float dot = playerView.Dot(toEnemy);
				float angle = DirectX::XMConvertToDegrees(acos(dot));

				if (angle < 10.0f) // プレイヤーが敵をある程度向いている
				{
					m_wpTarget = obj;
					return true;
				}
			}
			
			Math::Vector3 vec = hitPos - m_mWorld.Translation();
			vec.Normalize();

			bool isClear = SeaarchObstacle(hitPos,vec, m_overRap);

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



void Drone::Editor_ImGui()
{
	CharacterBase::Editor_ImGui();

	auto mat = GetCorrectionMatrix() * GetMatrix();
	ImGui::Text("CorrectionMat Translation: %.3f %.3f %.3f", mat._41, mat._42, mat._43);
}

void Drone::Deserialize(const nlohmann::json& jsonObj)
{
	CharacterBase::Deserialize(jsonObj);
}

void Drone::Serialize(nlohmann::json& outJson) const
{
	CharacterBase::Serialize(outJson);
}



void Drone::ChangeActionState(std::shared_ptr<ActionStateBase> nextAction)
{
	if (m_nowAction)m_nowAction->Exit(m_wpThis, m_wpTarget);
	m_prevAction = m_nowAction;
	m_nowAction = nextAction;
	m_nowAction->Enter(m_wpThis, m_wpTarget);
}

void Drone::ActionStateBase::ChangeStateWithDistance(std::weak_ptr<Drone>& owner, float targetLength)
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

bool Drone::ActionStateBase::ChangeStateObstacle(std::weak_ptr<Drone>& owner)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return false; }

	bool flg = spOwner->Search(false);

	if (flg)
	{
		return false;
	}

		auto target = spOwner->GetCharacterTarget().lock();

		auto vec = target->GetPos() - spOwner->GetPos();
		auto length = vec.Length();
		vec.Normalize();

		if (spOwner->SeaarchObstacle(spOwner->GetPos(), vec.Left, length))
		{
			spOwner->ChangeActionState(std::make_shared<MoveMent>());
			spOwner->m_nowAction->SetMoveDir(Left);
			return true;
		}
		else if (spOwner->SeaarchObstacle(spOwner->GetPos(), vec.Right, length))
		{
			spOwner->ChangeActionState(std::make_shared<MoveMent>());
			spOwner->m_nowAction->SetMoveDir(Right);
			return true;
		}
	

	return false;
}


void Drone::Idle::Enter(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();

	m_durationState = 1.0f;

	m_speed = 0.0f;


}

void Drone::Idle::Update(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();

	if (spOwner == nullptr) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	if (spOwner->SearchPlayer())
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

void Drone::Idle::PostUpdate(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	// おそらくエフェクト関連
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();

}

void Drone::Idle::Exit(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();
}

void Drone::MoveMent::Enter(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();

	m_speed = 30.0f;

	m_durationState = 0.95f;
}

void Drone::MoveMent::Update(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
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

	diff.y = 0.0f;

	Math::Vector3 vec = diff;

	if (m_side == TargetSide::Left)
	{
		vec = diff.Left;
	}
	else if (m_side == TargetSide::Right)
	{
		vec = diff.Right;
	}


	spOwner->MoveSwept(m_speed,vec , KdCollider::TypeGround);

	if (m_durationState < 0)
	{
		float len = (targetPos - pos).Length();
		ChangeStateWithDistance(owner, len);
		return;
	}

}

void Drone::MoveMent::PostUpdate(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();
}

void Drone::MoveMent::Exit(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();
}

void Drone::Attack::Enter(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();

	if (spOwner == nullptr) { return; }

	m_speed = 20.0f;

	m_durationState = 0.36f;

	bool flg = ChangeStateObstacle(owner);
	if (!flg)
	{
		spOwner->ChangeEnableRightAttack(true);
		spOwner->ChangeEnableLeftAttack(true);

		auto alert = std::make_shared<Alert>();
		auto pos = CameraManager::Instance().GetLocalDirectionTo(spOwner->GetMatrix().Translation());
		alert->CalcPos(pos);
		alert->Init();
		SceneManager::Instance().AddObject(alert);
	}
}

void Drone::Attack::Update(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
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

	spOwner->MoveSwept(m_speed, diff, KdCollider::TypeGround);

	if (m_durationState < 0)
	{
		float len = (targetPos - pos).Length();
		ChangeStateWithDistance(owner, len);
		return;
	}
}

void Drone::Attack::PostUpdate(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();
}

void Drone::Attack::Exit(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();

	if (spOwner == nullptr) { return; }

	spOwner->ChangeEnableRightAttack(false);

}


void Drone::Destroyed::Enter(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	m_speed = 30.0f;

	m_durationState = 2.0f;

	auto spOwner = owner.lock();
	auto spTarget = obj.lock();

	spOwner->m_isDestroy = true;

	spOwner->ChangeEnableRightAttack(false);
	spOwner->ChangeEnableLeftAttack(false);

	auto& am = KdAudioManager::Instance();
	am.Play("Asset/Sounds/Sound/drone_explode.wav")->SetVolume(am.GetSEVolume());

	spOwner->Burn();
}

void Drone::Destroyed::Update(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();

	if (spOwner == nullptr) { return; }

	m_durationState -= KdFPSController::GetInstance().GetDeltaTime();

	spOwner->MoveSwept(m_speed,Math::Vector3::Down, KdCollider::TypeGround);


	if (m_durationState <= 0)
	{
		spOwner->m_isExpired = true;
	}

}

void Drone::Destroyed::PostUpdate(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();
}

void Drone::Destroyed::Exit(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();
}

void Drone::Backed::Enter(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();

	m_speed = 20.0f;

	m_durationState = 0.85f;
}

void Drone::Backed::Update(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
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

	spOwner->MoveSwept(m_speed, diff, KdCollider::TypeGround, false, false);

	if (m_durationState < 0)
	{
		float len = (targetPos - pos).Length();
		ChangeStateWithDistance(owner, len);
		return;
	}
}

void Drone::Backed::PostUpdate(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();
}

void Drone::Backed::Exit(std::weak_ptr<Drone>& owner, const std::weak_ptr<KdGameObject>& obj)
{
	auto spOwner = owner.lock();
	auto spTarget = obj.lock();
}
