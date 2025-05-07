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
	ChangeActionState(std::make_shared<Stand>());
}

void Enemy::Update()
{
	//各種「状態に応じた」更新処理を実行する
	if (m_nowAction)
	{
		m_nowAction->Update(*this);
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
	if (m_nowAction)m_nowAction->Exit(*this);
	m_nowAction = nextAction;
	m_nowAction->Enter(*this);
}

void Enemy::Stand::Enter(Enemy& owner)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("Stand"));
}

void Enemy::Stand::Update(Enemy& owner)
{
}

void Enemy::Stand::Exit(Enemy& owner)
{
}

void Enemy::MoveForward::Enter(Enemy& owner)
{
	owner.m_spAnimator->SetAnimation(owner.m_spModel->GetData()->GetAnimation("BoostDush"));
}

void Enemy::MoveForward::Update(Enemy& owner)
{
	static int i = 0;
	++i;
	
	Math::Vector3 vec = {};
	float speed = 0.4f;

	if (i <= NITY*1) {
		vec = { 1,0,0 };
	}
	else if (i <= NITY*2)
	{
		vec = { 0,0,1 };
	}
	else if (i <= NITY*3)
	{
		vec = { -1,0,0 };
	}
	else
	{
		vec = { 0,0,-1 };
	}

	if (i > 360)
	{
		i = 0;
	}

	vec.Normalize();

	static Math::Vector3 pos = {};

	pos += vec * speed;

	owner.SetPos(pos);

}

void Enemy::MoveForward::Exit(Enemy& owner)
{
}

void Enemy::Attack::Enter(Enemy& owner)
{
}

void Enemy::Attack::Update(Enemy& owner)
{
}

void Enemy::Attack::Exit(Enemy& owner)
{
}
