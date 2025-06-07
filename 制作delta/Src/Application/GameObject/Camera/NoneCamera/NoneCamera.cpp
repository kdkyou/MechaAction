#include"NoneCamera.h"

void NoneCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateTranslation(0, 15.f, 0.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

}

void NoneCamera::PostUpdate()
{
	UpdateRotateByMouse();
	m_mWorld = m_mLocalPos;
}
