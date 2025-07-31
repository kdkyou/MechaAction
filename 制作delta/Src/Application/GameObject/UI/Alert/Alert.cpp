#include "Alert.h"

#include "../../Camera/CameraManager.h"
#include "../../Camera/CameraBase.h"

void Alert::Init()
{
	std::string str = "Asset/Textures/UI/Alert.png";

	/*m_spPoly = std::make_shared<KdSquarePolygon>();
	m_spPoly->SetMaterial(KdAssets::Instance().m_textures.GetData(str));*/

	m_spTex = KdAssets::Instance().m_textures.GetData(str);

	m_durationAlive = 0.05f;
}

void Alert::Update()
{
	if (m_durationAlive < 0) {
		m_isExpired = true;
	}

	m_durationAlive -= KdFPSController::GetInstance().GetDeltaTime();
	
	/*auto camMat = CameraManager::Instance().GetCurrentCamera().lock()->GetMatrix();
	*/
	Math::Vector3 frontPos = {0.0f,0.0f,0.11f};


	m_mWorld = Math::Matrix::CreateScale(1) * Math::Matrix::CreateRotationZ(DirectX::XMConvertToRadians(m_angle)) * Math::Matrix::CreateTranslation(frontPos);
}

void Alert::PostUpdate()
{
}

void Alert::DrawUnLit()
{
	if (m_spPoly == nullptr) { return; }

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spPoly,m_mWorld);

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
}

void Alert::DrawSprite()
{
	if (m_spTex == nullptr) { return; }

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	KdShaderManager::Instance().m_spriteShader.SetMatrix(m_mWorld);

	KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex,0,0);

	KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
}

void Alert::CalcPos(const Math::Vector2& dir2D)
{
	auto dir = dir2D;

	// 左右・上下でどちらが強いか比較
	if (fabs(dir.x) > fabs(dir.y))
	{
		if (dir.x > 0) { 
		m_nowDraw = AlertP::Right; 
		m_angle = 270;
		
		}
		else { m_nowDraw = AlertP::Left; 
		m_angle = 90;
		}
	}
	else {
		if (dir.y > 0) { m_nowDraw = AlertP::Up; 
		m_angle = 0;
		}
		else { m_nowDraw = AlertP::Down; 
		m_angle = 180;
		}
	}

}
