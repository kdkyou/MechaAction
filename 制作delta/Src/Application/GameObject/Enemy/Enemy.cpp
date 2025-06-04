#include"Enemy.h"

void Enemy::Init()
{
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Another/Another.gltf");
		// 初期のアニメーションをセットする
		m_spAnimator = std::make_shared<KdAnimator>();
		m_spAnimator->SetAnimation(m_spModel->GetData()->GetAnimation("StandUp"),10.0f ,true);
	}

	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();

		DirectX::BoundingOrientedBox box;

		box.Center = m_mWorld.Translation();
		box.Extents = { 3,10,3 };

		m_pCollider->RegisterCollisionShape("Enemy",box, KdCollider::TypeDamage);
	}

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	SetPos({ 15.0f,0.0f,10.0f });

	//初期状態を「待機状態」へ設定
	ChangeActionState(std::make_shared<Stand>());

	m_dist = { 10.0f,40.0f };
}

void Enemy::Update()
{
	//各種「状態に応じた」更新処理を実行する
	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();

		if (spTarget == nullptr) { return; }

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
	m_spAnimator->AdvanceTime(m_spModel->WorkNodes());

	m_pDebugWire->AddDebugBox(m_mWorld, {3,10,3}, {}, true, {1,0,0,1});

}

void Enemy::GenerateDepthMapFromLight()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void Enemy::DrawLit()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void Enemy::ChangeActionState(std::shared_ptr<ActionStateBase> nextAction)
{
	std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();

	if (m_nowAction)m_nowAction->Exit(m_wpThis, spTarget);
	m_nowAction = nextAction;
	m_nowAction->Enter(m_wpThis, spTarget);
}

void Enemy::Stand::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("Stand"),10.0f);
}

void Enemy::Stand::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	auto difference = spObj->GetMatrix().Translation() - spOwner->m_mWorld.Translation();

	
	int rando = rand()%10 ;

	if (rando < 7)
	{

		if (difference.Length() < spOwner->m_dist.y)
		{
			spOwner->ChangeActionState(std::make_shared<MoveForward>());
			return;
		}
	}
	else if (rando < 9)
	{
		spOwner->ChangeActionState(std::make_shared<Hited>());
		return;
	}
	else {
		spOwner->ChangeActionState(std::make_shared<Destoroy>());
		return;
	}


}

void Enemy::Stand::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
}

void Enemy::MoveForward::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("FLeaning"), 10.0f,false);
}

void Enemy::MoveForward::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	float speed = 30.0f;

	auto deltaTime = KdFPSController::GetInstance().GetDeltaTime();

	//現在の座標
	Math::Vector3 nowPos = spOwner->GetMatrix().Translation();

	//追尾対象の座標
	Math::Vector3 targetPos = spObj->GetMatrix().Translation();

	//対象への長さ
	Math::Vector3 difference = targetPos - nowPos;

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

	if (difference.Length() > spOwner->m_dist.x)
	{
		Math::Vector3 vec = {};

		Math::Matrix mat = rotMat * Math::Matrix::CreateTranslation(nowPos);

		vec = mat.Backward();

		vec.Normalize();

		nowPos += vec * speed*deltaTime;
	}
	else
	{
		spOwner->ChangeActionState(std::make_shared<Attack>());
		return;
	}

	Math::Matrix transMat = Math::Matrix::CreateTranslation(nowPos);

	spOwner->m_mWorld = rotMat * transMat;
}

void Enemy::MoveForward::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
}

void Enemy::Attack::Enter(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();
	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("LeftArmAction"), 10.0f,false);
}

void Enemy::Attack::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}
}

void Enemy::Attack::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

}

void Enemy::Hited::Enter(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("Hited"),10.0f, false);
}

void Enemy::Hited::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	//攻撃をくらったとき
	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}
}

void Enemy::Hited::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

}

void Enemy::Destoroy::Enter(std::weak_ptr<Enemy>& owner, const std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	spOwner->m_spAnimator->SetAnimation(spOwner->m_spModel->GetData()->GetAnimation("Destroyed"),5.0f,false);
}

void Enemy::Destoroy::Update(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

	//死亡時
	if (spOwner->m_spAnimator->IsAnimationEnd())
	{
		spOwner->ChangeActionState(std::make_shared<Stand>());
		return;
	}

}

void Enemy::Destoroy::Exit(std::weak_ptr<Enemy>& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	std::shared_ptr<Enemy> spOwner = owner.lock();

}
