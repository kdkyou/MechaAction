#include"Enemy.h"

void Enemy::Init()
{
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelWork>();
		m_spModel->SetModelData("Asset/Models/Grint/grint.gltf");	
		// 初期のアニメーションをセットする
		m_spAnimator = std::make_shared<KdAnimator>();
		m_spAnimator->SetAnimation(m_spModel->GetData()->GetAnimation("Stand"));
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
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("BoostDush"));
}

void Enemy::MoveForward::Update(Enemy& owner, const  std::shared_ptr<KdGameObject>& spObj)
{
	
	Math::Vector3 vec = {};
	float speed = 0.4f;

	Math::Vector3 nowPos = owner.GetMatrix().Translation();
	Math::Vector3 targetPos = spObj->GetMatrix().Translation();

	Math::Vector3 difference = targetPos - nowPos;

	if (difference.Length() > 30.0f)
	{

		vec = difference;

		vec.Normalize();

		static Math::Vector3 pos = {};

		pos += vec * speed;

		owner.SetPos(pos);

	}
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
