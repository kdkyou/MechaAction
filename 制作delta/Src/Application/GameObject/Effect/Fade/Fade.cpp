#include "Fade.h"

void Fade::SetFade(FadeType type, float time, const bool OutorIn)
{
	if (m_completeFade || type == FadeIn || type == FadeOut)
	{
		m_completeFade = false;
		fadeFrame = 0;
		fadeTime = time;
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
	fadeFrame = 0;
	fadeTime = 20;
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
		fadeFrame+= KdFPSController::GetInstance().GetDeltaTime();
		m_alpha += m_IncDec * 1.0f / fadeTime;
		
		if (m_alpha > 1.0f)
		{
			m_alpha = 1.0f;
		}
		if (m_alpha < 0.0f)
		{
			m_alpha = 0.0f;
		}

		if (fadeFrame >= fadeTime)
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
