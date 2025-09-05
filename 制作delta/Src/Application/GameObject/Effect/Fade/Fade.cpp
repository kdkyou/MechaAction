#include "Fade.h"

void Fade::SetFade(FadeType type, float time, const bool OutorIn)
{
	if (m_completeFade || type == FadeIn || type == FadeOut)
	{
		m_completeFade = false;
		m_durationFade = 0;
		m_fadeTime = time;
		nowFadeType = type;
		if (OutorIn == true)
		{
			m_IncDec = 1;
			m_alpha = 0.0f;
		}
		else
		{
			m_IncDec = -1;
			m_alpha = 1.0f;
		}

		switch (nowFadeType)
		{
		case FadeIn:
		case FadeInLeftRight:
		case FadeInRightLeft:
			m_fillDisplay = false;
			break;
		}
	}
	isLaunch = true;
}

bool Fade::IsFillDisplay() const
{
	return m_fillDisplay;
}

bool Fade::IsCompleteFade() const
{
	return m_completeFade;
}

void Fade::Release()
{
	m_spTex = nullptr;
}

Fade::~Fade()
{
	Release();
}

void Fade::Init()
{
	m_spTex = KdAssets::Instance().m_textures.GetData("Asset/Textures/UI/BlackBack.png");
	m_completeFade = false;
	m_durationFade = 0;
	m_fadeTime = 20.0f;
	m_fillDisplay = false;
	m_IncDec = 0;
	m_alpha = 0.0f;
	isLaunch = false;

}

void Fade::DrawSprite()
{
	Math::Rectangle _rct = { 0, 0,(long)m_spTex->GetWidth(),(long)m_spTex->GetHeight() };
	Math::Color _color = { 1,1,1,m_alpha };
	KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, 0, 0, &_rct, &_color);
}

void Fade::Update()
{
	if (!m_completeFade)
	{
		auto delta = KdFPSController::GetInstance().GetDeltaTime();
		m_durationFade += delta;
		
		float t = m_durationFade / m_fadeTime;
		t = std::clamp(t, 0.0f, 1.0f);

		// フェード方向に応じてアルファ更新
		if (m_IncDec > 0) {
			m_alpha = t;
		}
		else {
			m_alpha = 1.0f - t ;
		}

		if (m_durationFade >= m_fadeTime)
		{
			m_completeFade = true;

			switch (nowFadeType)
			{
			case FadeOut:
			case FadeOutLeftRight:
			case FadeOutRightLeft:
				m_fillDisplay = true;
				break;
			}

		}
	}
}
