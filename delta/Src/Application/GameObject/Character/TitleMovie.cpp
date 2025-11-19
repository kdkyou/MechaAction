#include "TitleMovie.h"

#include "../Camera/CameraManager.h"

#include "../../main.h"

#include "../../Scene/SceneManager.h"
#include "../UI/UIManager.h"

void TitleMovie::Init()
{
	m_name = "TitleMovie";
}

void TitleMovie::Update()
{
	auto delta = KdFPSController::GetInstance().GetDeltaTime();

	switch (m_type)
	{
	case TitleMovie::OnetoTwo:

		// 待機時間
		m_durationWait += delta;

		if (m_durationWait > m_firstWaitTime) {

			// 時間増加
			m_durationMove += delta;

			auto t = m_durationMove / m_firstTime;

			m_pos = Math::Vector3::Lerp(m_firstPos, m_secondPos, t);

			if (t > 1.0f) {
				m_durationWait = 0.0f;
				m_durationMove = 0.0f;
				m_type = TwotoThree;

				auto& am = KdAudioManager::Instance();
				am.Play("Asset/Sounds/SE/CatapultTail.wav")->SetVolume(am.GetSEVolume());
			}
		}
		break;
	case TitleMovie::TwotoThree:
		// 待機時間
		m_durationWait += delta;
		if (m_durationWait > m_secondWaitTime / 5.0f)
		{
			if (!m_isFirstAnim)
			{
				m_isFirstAnim = true;
				CameraManager::Instance().Setting("Asset/Data/TitleMovieCamera2.scene");
			}
		}
		
		if (m_durationWait > m_secondWaitTime) {

			// 時間増加
			m_durationMove += delta;

			auto t = m_durationMove / m_secondTime;

			m_pos = Math::Vector3::Lerp(m_secondPos, m_thirdPos, t);

			if (t > 1.0f) {
				m_durationWait = 0.0f;
				m_durationMove = 0.0f;
				m_type = ThreetoFour;
				CameraManager::Instance().Setting("Asset/Data/TitleMovieCamera4.scene");
				UIManager::GetInstance().SceneUIAdd("Asset/Data/UI/SystemUI.scene");
				if (m_spAnimator) {
				m_spAnimator->SetAnimation(m_spModel->GetAnimation(m_animPath), 10.0f, false,false);
				
				}
			}
		}
		break;
	case TitleMovie::ThreetoFour:

		if (m_spAnimator->IsAnimationEnd())
		{
			// 待機時間
			m_durationWait += delta;

			if (!m_isSecondAnim)
			{
				m_isSecondAnim = true;
				CameraManager::Instance().Setting("Asset/Data/TitleMovieCamera2.scene");

				if (!m_isThurster) {
					m_isThurster = true;
					KdModelWork::Node* pNode = m_spModel->FindWorkNode("CBP");
					if (pNode) {
						auto nodeMat = pNode->m_worldTransform;
						auto mat = nodeMat.Translation() * m_mWorld.Translation();
						m_efk = KdEffekseerManager::GetInstance().Play("MovieThruster.efkefc", mat, 2.0f, 0.3f, false);
						Application::Instance().m_log.AddLog("Create\n");
						auto& am = KdAudioManager::Instance();
						am.Play("Asset/Sounds/Sound/burst_start.wav")->SetVolume(am.GetSEVolume());
					}
				}


			}
		}

		if (m_durationWait > m_thirdWaitTime) {

			// 時間増加
			m_durationMove += delta;

			auto t = m_durationMove / m_thirdTime;

			m_pos = Math::Vector3::Lerp(m_thirdPos, m_forthPos, t);

			if (t > 1.0f) {
				m_type = FourtoFive;
				m_durationWait = 0.0f;
				m_durationMove = 0.0f;
			}
		}
		break;
	case TitleMovie::FourtoFive:
		
		KdEffekseerManager::GetInstance().StopAllEffect();
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Game
		);


		break;
	default:
		break;
	}


	auto mScale = Math::Matrix::CreateScale(m_scale);
	m_mWorld = mScale * Math::Matrix::CreateTranslation(m_pos);
}

void TitleMovie::PostUpdate()
{
	if (m_spAnimator)
	{
		if (m_spModel) {
			m_spAnimator->AdvanceTime(m_spModel->WorkNodes(), m_animSpeed);
			m_spModel->CalcNodeMatrices();
		}
	}
	

	if (m_isThurster)
	{
		if (!m_spModel) { return; }
		auto spefct = m_efk.lock();

		/*if (!spefct)
		{
			KdModelWork::Node* pNode = m_spModel->FindWorkNode("CBP");
			if (pNode) {
				auto nodeMat = pNode->m_worldTransform;
				auto mat = nodeMat.Translation() * m_mWorld.Translation();
				m_efk = KdEffekseerManager::GetInstance().Play("MovieThruster.efkefc", mat, 0.5f, 1.0f, false);
			}
		}*/
		
		if (spefct)
		{
			Math::Matrix mat = Math::Matrix::Identity;

			KdModelWork::Node* pNode = m_spModel->FindWorkNode("CBP");
			if (pNode) {
				mat = pNode->m_worldTransform;
			}

			Effekseer::Handle handle = 0;

			auto matrix = mat * m_mWorld;
			spefct->SetWorldMatrix(matrix);
		}
	}
}

void TitleMovie::DrawLit()
{
	if (!m_spModel) return;

	for (auto& point : m_points)
	{
		KdShaderManager::Instance().WorkAmbientController().AddPointLight(point);
	}

	KdShaderManager::Instance().ChangeRasterizerState(KdRasterizerState::CullNone);
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld, m_modelColor, m_emmisive);
	KdShaderManager::Instance().ChangeRasterizerState(KdRasterizerState::CullBack);
}

void TitleMovie::Editor_ImGui()
{
	DrawTerrain::Editor_ImGui();

}
