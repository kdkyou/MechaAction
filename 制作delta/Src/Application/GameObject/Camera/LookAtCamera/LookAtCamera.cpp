#include "LookAtCamera.h"

#include "../../Character/CharacterBase.h"

void LookAtCamera::Init()
{
	CameraBase::Init();

	m_name = "LookAt";

}

void LookAtCamera::PostUpdate()
{

	Look();

	m_mWorld = m_mRotation * Math::Matrix::CreateTranslation(m_pos);

	CameraBase::PostUpdate();
}

void LookAtCamera::Look()
{
	auto spTarget = m_wpLook.lock();

	if (!spTarget) { return; }

	auto targetMat = spTarget->GetMatrix();
	Math::Vector3 targetPos = targetMat.Translation();

	Math::Vector3 pos = m_mWorld.Translation();

	Math::Vector3 nowVec = m_mWorld.Backward();

	Math::Vector3 toVec = targetPos - pos;
	auto distance = toVec.Length();
	if (distance < 0.001) { return; }

	// 注視用のビュー行列を作る（Z軸がtoVecを向く）
	Math::Matrix lookMat = Math::Matrix::CreateLookAt(targetPos, pos, { 0, 1, 0 });
	lookMat = lookMat.Invert(); // LookAt行列をワールド行列に変換

	// 回転をクオータニオンに変換
	Math::Quaternion targetQuat = Math::Quaternion::CreateFromRotationMatrix(lookMat);
	Math::Quaternion currentQuat = Math::Quaternion::CreateFromRotationMatrix(m_mWorld);

	// 回転差（角度）を取得
	float angleDiffRad = currentQuat.Dot(targetQuat);
	angleDiffRad = std::clamp(angleDiffRad, -1.0f, 1.0f);
	float deg = DirectX::XMConvertToDegrees(acosf(angleDiffRad) * 2.0f); // クオータニオンの角度差

	// 補間スピード設定
	float baseSpeedDeg = 90.0f;
	float boostSpeedDeg = 720.0f;

	float boostRate = std::clamp(deg / 20.0f, 0.0f, 1.0f);
	float rotateSpeedDeg = baseSpeedDeg + (boostSpeedDeg - baseSpeedDeg) * boostRate;


	float delta = KdFPSController::GetInstance().GetDeltaTime();
	float t = std::clamp(rotateSpeedDeg * delta / 180.0f, 0.0f, 1.0f); // normalize補間係数

	// 球面線形補間
	Math::Quaternion newQuat = Math::Quaternion::Slerp(currentQuat, targetQuat, t);

	// 回転行列へ戻す
	Math::Matrix rotMat = Math::Matrix::CreateFromQuaternion(newQuat);

	m_mRotation = rotMat;

	

}
