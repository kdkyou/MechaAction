#include"Enemy.h"

void Enemy::Init()
{
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Leg/Leg.gltf");	
		// 初期のアニメーションをセットする
		m_spAnimator = std::make_shared<KdAnimator>();
		m_spAnimator->SetAnimation(m_spModel->GetData()->GetAnimation("FLeaning"),false);
	}

	//初期状態を「待機状態」へ設定
	ChangeActionState(std::make_shared<MoveForward>());
}

void Enemy::Update()
{
	//各種「状態に応じた」更新処理を実行する
	if (m_nowAction)
	{
		std::shared_ptr<KdGameObject> spTarget = m_wpTarget.lock();

	//	if(spTarget==nullptr){}

		m_nowAction->Update(*this,spTarget);
	}
}

void Enemy::PostUpdate()
{
	m_spAnimator->AdvanceTime(m_spModel->WorkNodes());
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

	if (m_nowAction)m_nowAction->Exit(*this,spTarget);
	m_nowAction = nextAction;
	m_nowAction->Enter(*this,spTarget);
}

void Enemy::Stand::Enter(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("Stand"));
}

void Enemy::Stand::Update(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	//if()
	

}

void Enemy::Stand::Exit(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
}

void Enemy::MoveForward::Enter(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("FLeaning"),false);
}

void Enemy::MoveForward::Update(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	
	Math::Vector3 vec = {};
	float speed = 0.4f;

	Math::Vector3 nowPos = owner.GetMatrix().Translation();
	Math::Vector3 nowVec = owner.GetMatrix().Backward();

	Math::Vector3 targetPos = spObj->GetMatrix().Translation();
	Math::Vector3 targetVec = spObj->GetMatrix().Backward();

	Math::Vector3 difference = targetPos - nowPos;


	nowVec.Normalize();
	targetVec.Normalize();

	float _nowAng = atan2(nowVec.x, nowVec.z);
	_nowAng = DirectX::XMConvertToDegrees(_nowAng);

	float _targetAng = atan2(targetVec.x, targetVec.z);
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

	float rotateAng = std::clamp(_betweenAng, -owner.m_angle, -owner.m_angle);
	owner.m_worldRot.y += rotateAng;
	
	Math::Matrix rotMat = Math::Matrix::CreateFromYawPitchRoll(owner.m_worldRot);

	Math::Matrix transMat = Math::Matrix::CreateTranslation(nowPos);

	if (difference.Length() > 30.0f &&  _betweenAng<3.0f)
	{

		vec = difference;

		vec.Normalize();

		nowPos += vec * speed;

		 transMat = Math::Matrix::CreateTranslation(nowPos);

	}

	owner.m_mWorld = rotMat * transMat;

}

void Enemy::MoveForward::Exit(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
}

void Enemy::Attack::Enter(Enemy& owner, const std::shared_ptr<KdGameObject>& spObj)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("LeftArmAction"), false);
}

void Enemy::Attack::Update(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	if (owner.m_spAnimator->IsAnimationEnd())
	{
		owner.ChangeActionState(std::make_shared<Stand>());
		return;
	}
}

void Enemy::Attack::Exit(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
}
