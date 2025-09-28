#include "TransAC.h"

void TransAC::Init()
{
	m_spModelWork = std::make_shared<KdModelWork>();
	m_spModelWork->SetModelData("Asset/Models/Grint/Grint.gltf");

	m_spAnimator = std::make_shared<KdAnimator>();
	m_spAnimator->SetAnimation(m_spModelWork->GetData()->GetAnimation("Boost"), 0.0f, false, false);
	
	m_dissolveSpeed = 3.0f;
	m_dissolveColor = {0.8f,0.0f,0.3f};

}



void TransAC::Update()
{
	if (m_isExpired == true) { return; }

	if (m_dissolveProgress > 1.0f) {
		m_isExpired = true;
		return;
	}

	auto delta = KdFPSController::GetInstance().GetDeltaTime();
	m_dissolveProgress += m_dissolveSpeed * delta;


}

void TransAC::PostUpdate()
{
	if (m_isExpired == true) { return; }

	m_spAnimator->AdvanceTime(m_spModelWork->WorkNodes(), 0.0f);
	m_spModelWork->CalcNodeMatrices();

}

void TransAC::DrawUnLit()
{
	//if (m_isExpired == true) { return; }
	//KdShaderManager::Instance().m_StandardShader.SetDissolve(m_dissolveProgress, nullptr, &m_dissolveColor);

	//Math::Vector3 emis = { 2,2,2 };
	//KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld, kWhiteColor, emis);

}

void TransAC::DrawLit()
{
	if (m_isExpired == true) { return; }

	KdShaderManager::Instance().m_StandardShader.SetDissolve(m_dissolveProgress, nullptr, &m_dissolveColor);

	Math::Vector3 emis = { 2,2,2 };
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld, kWhiteColor, emis);

}


void TransAC::DrawBright()
{
	/*if (m_isExpired == true) { return; }

	KdShaderManager::Instance().m_StandardShader.SetDissolve(m_dissolveProgress, nullptr, &m_dissolveColor);

	Math::Vector3 emis = { 2,2,2 };
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld, kWhiteColor, emis);*/

}



void TransAC::SetTransACData(const std::string& modelPath,const std::string& animPath,float animprogress, const Math::Matrix& mat, float dissolveProgress, float dissolvespeed, const Math::Vector3 dissolveColor)
{
	if (modelPath.empty() == true) { return; }
	
	m_spModelWork = std::make_shared<KdModelWork>();
	m_spModelWork->SetModelData(modelPath);
	
	m_spAnimator = std::make_shared<KdAnimator>();

	auto animation = m_spModelWork->GetData()->GetAnimation(animPath);
	if (animation)
	{
		m_spAnimator->SetAnimation(animation, 0.0f, false, false);
		float animAdvance = m_spModelWork->GetData()->GetAnimation(animPath)->m_maxLength * animprogress;
		m_spAnimator->SetAdvanceTime(animAdvance);
	}
	else {
		m_spAnimator->SetAnimation(m_spModelWork->GetData()->GetAnimation("Stand"), 0.0f, false, false);
		float animAdvance = m_spModelWork->GetData()->GetAnimation(animPath)->m_maxLength * animprogress;
		m_spAnimator->SetAdvanceTime(animAdvance);
	}
	
	m_mWorld = mat;

	m_dissolveProgress = dissolveProgress;
	m_dissolveSpeed = dissolvespeed;
	m_dissolveColor = dissolveColor;

}
