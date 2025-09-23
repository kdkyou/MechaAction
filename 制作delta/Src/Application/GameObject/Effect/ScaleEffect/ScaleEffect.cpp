#include "ScaleEffect.h"

#include "../../Camera/CameraManager.h"
#include "../../Camera/CameraBase.h"

void ScaleEffect::Init()
{
	m_name = "ScaleEffect";


}

void ScaleEffect::Update()
{
	if (m_durationAlive >= m_aliveTime) {
		m_isExpired = true;
	}
	
	m_durationAlive += KdFPSController::GetInstance().GetDeltaTime();

	float scale = 1.0f;

	auto progress = m_durationAlive / m_aliveTime;
	scale = std::lerp(m_scale, 0.0f, progress);

	if (scale < 0.0f)
	{
		scale = 0.0f;
		m_isExpired = true;
	}

	m_spPoly->SetScale(scale);
}

void ScaleEffect::PostUpdate()
{
	const auto mat = m_mWorld;

	auto cam = CameraManager::Instance().GetCurrentCamera().lock();

	auto camMat = cam->GetMatrix();

	 camMat._42 = camMat._43 = 0;

	// GameObjectの位置情報
	auto posMat = Math::Matrix::CreateTranslation(mat.Translation());
	
	////追加のZ回転角度
	auto zRotMat = Math::Matrix::CreateRotationZ(m_rot.x * KdToRadians);
	//auto yRotMat = Math::Matrix::CreateRotationY(m_rot.x * KdToRadians);
	auto xRotMat = Math::Matrix::CreateRotationX(m_rot.x * KdToRadians);
	//
	//行列合成
	auto totalMat = xRotMat * zRotMat * camMat * posMat;

	//m_mWorld = totalMat;

}

void ScaleEffect::DrawUnLit()
{
	if (!m_spPoly) { return; }
	//KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spPoly, m_mWorld);
	//KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
}

void ScaleEffect::SetPolygonParam(const std::string& texPath, float scale, float alive)
{
	if(texPath == ""){}
	m_spPoly = std::make_shared<KdSquarePolygon>();
	SetTexture(texPath);

	m_scale = scale;

	m_aliveTime = alive;

}
