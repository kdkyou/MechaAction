#include"LockCamera.h"

#include"../../Character/CharacterBase.h"

#include "../../Camera/CameraManager.h"

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

	m_lockPos = {};

	m_spPolygon = std::make_shared<KdSquarePolygon>();

	m_spPolygon->SetMaterial("Asset/Textures/UI/LockOn.png");

	m_spPolygon->SetScale(1.0f);

	m_lockPos = {0.0f,0.0f,0.1f};

	m_texAlpha = 0.2f;

	m_texScale = 0.03f;
	m_durationScale = 0.2f;

	CameraManager::Instance().EnableChangedCamera(false);
}

void LockCamera::Update()
{
	ShowCursor(false);

	if (!m_isReduce)
	{
		m_texAlpha += 5 * KdFPSController::GetInstance().GetDeltaTime();
		m_durationScale -= KdFPSController::GetInstance().GetDeltaTime();

		if (m_durationScale < m_texScale)
		{
			m_durationScale = m_texScale;
			m_isReduce = true;
		}

		if (m_texAlpha > 1.0f)
		{
			m_texAlpha = 1.0f;
		}
	}
	else {
		
		CameraManager::Instance().EnableChangedCamera(true);

		auto& mouse = KeyInput::GetInstance().GetMouseState();
		auto& key = KeyInput::GetInstance().GetKeyboardState();
		if (mouse.middleButton) {
			CameraManager::Instance().SetNextType(CameraManager::CameraType::Tracking);
		}
	}
}

void LockCamera::PostUpdate()
{
	// ターゲットの行列(有効な場合利用する)
	Math::Matrix								_targetMat = Math::Matrix::Identity;
	const std::shared_ptr<const CharacterBase>	_spTarget = m_wpTarget.lock();
	if (_spTarget)
	{
		if (_spTarget->IsDestroy() == true) { 
			CameraManager::Instance().EnableChangedCamera(true);
			CameraManager::Instance().SetNextType(CameraManager::CameraType::Tracking);
			return; 
		}
		_targetMat = Math::Matrix::CreateTranslation(_spTarget->GetMatrix().Translation());
	}
	

	UpdateRotateByMouse();

	Lock();

	m_mWorld = m_mLocalPos * m_mRotation * _targetMat;

	TerrainCheck();

	CameraBase::PostUpdate();
}

void LockCamera::DrawUnLit()
{
	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	auto camMat = m_mWorld;
	
	Math::Color color = { 1.0f,1.0f,1.0f,m_texAlpha };

	m_spPolygon->SetColor(color);

	Math::Matrix mat = Math::Matrix::CreateScale(m_durationScale) * Math::Matrix::CreateTranslation(m_lockPos) * camMat;

	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spPolygon,mat);

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
}

void LockCamera::DrawSprite()
{

}

void LockCamera::Lock()
{
	auto spTarget = m_wpLockTarget.lock();
	if (!spTarget) {
		CameraManager::Instance().SetNextType(CameraManager::CameraType::Tracking);
		return;
	}

	auto targetMat = spTarget->GetCorrectionMatrix() * spTarget->GetMatrix();
	Math::Vector3 targetPos = targetMat.Translation();

	Math::Vector3 pos = m_mWorld.Translation();

	Math::Vector3 nowVec = m_mWorld.Backward();

	Math::Vector3 toVec = targetPos - pos;
	toVec.y = 0.0f;
	auto distance = toVec.Length();

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

	
	if (distance < 27.001) {
			return; 
	}


	// 補間スピード設定
	float baseSpeedDeg = 90.0f;
	float boostSpeedDeg = 1080.0f;

	
	float boostRate = std::clamp(deg / 5.0f, 0.0f, 1.0f);
	float rotateSpeedDeg = baseSpeedDeg + (boostSpeedDeg - baseSpeedDeg) * boostRate;
	
	
	float delta = KdFPSController::GetInstance().GetDeltaTime();
	float t = std::clamp(rotateSpeedDeg * delta / 180.0f, 0.0f, 1.0f); // normalize補間係数

	// 球面線形補間
	Math::Quaternion newQuat = Math::Quaternion::Slerp(currentQuat, targetQuat, t);

	// 角度保存
	Math::Vector3 euler;
	euler = newQuat.ToEuler();
	m_DegAng.x = DirectX::XMConvertToDegrees(euler.x);
	m_DegAng.y = DirectX::XMConvertToDegrees(euler.y);
	m_DegAng.z = DirectX::XMConvertToDegrees(0);

	// 回転行列へ戻す
	Math::Matrix rotMat = Math::Matrix::CreateFromYawPitchRoll(m_DegAng*KdToRadians);

	m_mRotation = rotMat;

}
