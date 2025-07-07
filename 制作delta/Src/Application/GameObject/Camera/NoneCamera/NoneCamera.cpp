#include"NoneCamera.h"

void NoneCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateTranslation(0, 0.0f, 0.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	m_name = "None";

	ShowCursor(true);
}

void NoneCamera::PostUpdate()
{
}
