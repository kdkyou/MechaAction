#include "AnimationCamera.h"

void AnimationCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	m_name = "Animation";
	m_duration = 0.0f;
}

void AnimationCamera::Update()
{
}

void AnimationCamera::PostUpdate()
{

	m_duration += KdFPSController::GetInstance().GetDeltaTime();

	if (m_duration > m_animTime) {
		m_duration = m_animTime;
	}

	// 回転をクオータニオンに変換
	auto firstMat = Math::Matrix::CreateFromYawPitchRoll(m_firstRot * KdToRadians);
	Math::Quaternion firstQuat = Math::Quaternion::CreateFromRotationMatrix(firstMat);
	auto secondMat = Math::Matrix::CreateFromYawPitchRoll(m_SecondRot*KdToRadians);
	Math::Quaternion secondQuat = Math::Quaternion::CreateFromRotationMatrix(secondMat);

	float t = 1.0f/m_animTime * m_duration;

	// 球面線形補間
	Math::Quaternion newQuat = Math::Quaternion::Slerp(firstQuat,secondQuat, t);

	// 座標線形補間
	auto pos = Math::Vector3::Lerp(m_firstPos, m_SecondPos, t);

	// 回転行列へ戻す
	Math::Matrix mat = Math::Matrix::CreateFromQuaternion(newQuat) * Math::Matrix::CreateTranslation(pos);

	m_mWorld = mat;

	CameraBase::PostUpdate();
}


void AnimationCamera::Editor_ImGui()
{
	CameraBase::Editor_ImGui();

	if (ImGui::DragFloat((const char*)u8"アニメーション時間", &m_animTime, 0.01f, 0.0f)) {
		m_duration = 0.0f;
	}

	ImGui::DragFloat3((const char*)u8"一つ目の座標", &m_firstPos.x, 0.01f);
	ImGui::DragFloat3((const char*)u8"一つ目の回転", &m_firstRot.x, 0.01f);
	ImGui::DragFloat3((const char*)u8"二つ目の座標", &m_SecondPos.x, 0.01f);
	ImGui::DragFloat3((const char*)u8"二つ目の回転", &m_SecondRot.x, 0.01f);

}

void AnimationCamera::Deserialize(const nlohmann::json& jsonObj)
{
	CameraBase::Deserialize(jsonObj);

	KdJsonUtility::GetValue(jsonObj, "AnimTime", &m_animTime);
	KdJsonUtility::GetArray(jsonObj, "FirstPos", &m_firstPos.x, 3);
	KdJsonUtility::GetArray(jsonObj, "FirstRot", &m_firstRot.x, 3);
	KdJsonUtility::GetArray(jsonObj, "SecondPos", &m_SecondPos.x, 3);
	KdJsonUtility::GetArray(jsonObj, "SecondRot", &m_SecondRot.x, 3);

}

void AnimationCamera::Serialize(nlohmann::json& outJson) const
{
	CameraBase::Serialize(outJson);

	outJson["AnimTime"] = m_animTime;
	outJson["FirstPos"] = KdJsonUtility::CreateArray(&m_firstPos.x, 3);
	outJson["FirstRot"] = KdJsonUtility::CreateArray(&m_firstRot.x, 3);
	outJson["SecondPos"] = KdJsonUtility::CreateArray(&m_SecondPos.x, 3);
	outJson["SecondRot"] = KdJsonUtility::CreateArray(&m_SecondRot.x, 3);

}
