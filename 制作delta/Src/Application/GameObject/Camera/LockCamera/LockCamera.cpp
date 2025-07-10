#include"LockCamera.h"


void LockCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateTranslation(m_localPos);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	m_mWorld = m_mLocalPos * m_wpTarget.lock()->GetMatrix();

	m_name = "Lock";

	ShowCursor(false);

}

void LockCamera::PostUpdate()
{
	// ターゲットの行列(有効な場合利用する)
	Math::Matrix								_targetMat = Math::Matrix::Identity;
	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpTarget.lock();
	if (_spTarget)
	{
		_targetMat = Math::Matrix::CreateTranslation(_spTarget->GetPos());
	}

	UpdateRotateByMouse();

	Lock();

	m_mRotation = GetRotationMatrix();
	m_mWorld = m_mLocalPos * m_mRotation * _targetMat;

}

void LockCamera::Lock()
{
	const std::shared_ptr<const KdGameObject> spTarget = m_wpRockTarget.lock();
	if (spTarget == nullptr) { return; }

	
	Math::Vector3 targetPos = spTarget->GetMatrix().Translation();
	Math::Vector3 pos = m_mWorld.Translation();

	Math::Vector3 nowVec = m_mWorld.Backward();

	Math::Vector3 toVec = targetPos - pos;
	toVec.y = 0.0f;
	toVec.Normalize();

	float targetYaw = atan2f(toVec.x, toVec.z); // ラジアンでY軸方向の向きを計算
	float currentYaw = atan2f(nowVec.x, nowVec.z);

	float rotateSpeed = 0.1f; // 補間スピード
	float newYaw = (1.0f - rotateSpeed) * currentYaw + rotateSpeed * targetYaw;

	// ラジアン → 度に変換して角度を設定
	m_DegAng.y = DirectX::XMConvertToDegrees(newYaw);


	if (m_DegAng.y > 360)
	{
		m_DegAng.y -= 360;
	}
	else if (m_DegAng.y < 0)
	{
		m_DegAng.y += 360;
	}

	


	CameraBase::PostUpdate();
}
