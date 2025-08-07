#include"LockCamera.h"

#include"../../Character/CharacterBase.h"


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

	/*m_spTex = std::make_shared<KdTexture>();
	m_spTex = KdAssets::Instance().m_textures.GetData("Asset/Textures/UI/LockOn.png");*/

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
}

void LockCamera::PostUpdate()
{
	// ターゲットの行列(有効な場合利用する)
	Math::Matrix								_targetMat = Math::Matrix::Identity;
	const std::shared_ptr<const CharacterBase>	_spTarget = m_wpTarget.lock();
	if (_spTarget)
	{
		_targetMat = Math::Matrix::CreateTranslation(_spTarget->GetMatrix().Translation());
	}

	UpdateRotateByMouse();

	Lock();

	//m_mRotation = GetRotationMatrix();
	m_mWorld = m_mLocalPos * m_mRotation * _targetMat;


	CameraBase::PostUpdate();
}

void LockCamera::DrawUnLit()
{
	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	auto camMat = m_mWorld;
	//camMat.Translation(Math::Vector3::Zero);

	Math::Color color = { 1.0f,1.0f,1.0f,m_texAlpha };

	m_spPolygon->SetColor(color);

	Math::Matrix mat = Math::Matrix::CreateScale(m_durationScale) * Math::Matrix::CreateTranslation(m_lockPos) * camMat;

	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_spPolygon,mat);

	KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
}

void LockCamera::DrawSprite()
{
	//KdShaderManager::Instance().ChangeBlendState(KdBlendState::Add);

	//KdShaderManager::Instance().m_spriteShader.DrawTex(m_spTex, m_lockPos.x, m_lockPos.y);

	//KdShaderManager::Instance().ChangeBlendState(KdBlendState::Alpha);
}

void LockCamera::Lock()
{
	auto spTarget = m_wpRockTarget.lock();
	if (spTarget == nullptr) { return; }

	auto targetMat = spTarget->GetCorrectionMatrix() * spTarget->GetMatrix();
	Math::Vector3 targetPos = targetMat.Translation();

	Math::Vector3 pos = m_mWorld.Translation();

	Math::Vector3 nowVec = m_mWorld.Backward();

	Math::Vector3 toVec = targetPos - pos;
	auto distance = toVec.Length();
	if (distance < 0.001) { return; }

	//toVec.Normalize();

	//float targetYaw = atan2f(toVec.x, toVec.z);
	//float currentYaw = DirectX::XMConvertToRadians(m_DegAng.y); // m_DegAng.y は度 → ラジアンに変換

	//// 差分を求めて -π～+π の範囲に調整（最短角度補間）
	//float deltaYaw = targetYaw - currentYaw;

	//while (deltaYaw > DirectX::XM_PI)     deltaYaw -= DirectX::XM_2PI;
	//while (deltaYaw < -DirectX::XM_PI)    deltaYaw += DirectX::XM_2PI;

	//// 補間
	//float speedRatio = std::clamp(distance / 20.0f, m_speedRatio.x, m_speedRatio.y);
	//float rotateSpeed = 90.0f * speedRatio; // deg/s

	//float delta = DirectX::XMConvertToRadians(rotateSpeed * KdFPSController::GetInstance().GetDeltaTime());
	//if (fabsf(deltaYaw) < delta)
	//{
	//	currentYaw = targetYaw; // 吸収してピタッと止める
	//}
	//else
	//{
	//	currentYaw += (deltaYaw > 0 ? 1 : -1) * delta;
	//}

	//// 結果をDegに変換
	//m_DegAng.y = DirectX::XMConvertToDegrees(currentYaw);


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

	//float deg = Math::Quaternion::Angle(currentQuat, targetQuat);
	// 補間スピード設定
	float baseSpeedDeg = 90.0f;
	float boostSpeedDeg = 720.0f;

	
	//float boostRate = std::clamp((distance - 5.0f) / 10.0f, 0.0f, 1.0f);
	float boostRate = std::clamp(deg / 20.0f, 0.0f, 1.0f);
	float rotateSpeedDeg = baseSpeedDeg + (boostSpeedDeg - baseSpeedDeg) * boostRate;
	
	
	float delta = KdFPSController::GetInstance().GetDeltaTime();
	float t = std::clamp(rotateSpeedDeg * delta / 180.0f, 0.0f, 1.0f); // normalize補間係数

	// 球面線形補間
	Math::Quaternion newQuat = Math::Quaternion::Slerp(currentQuat, targetQuat, t);

	// 回転行列へ戻す
	Math::Matrix rotMat = Math::Matrix::CreateFromQuaternion(newQuat);

	m_mRotation = rotMat;

	// 角度保存
	Math::Vector3 euler;
	euler = newQuat.ToEuler();
	m_DegAng.x = DirectX::XMConvertToDegrees(euler.x);
	m_DegAng.y = DirectX::XMConvertToDegrees(euler.y);
	m_DegAng.z = DirectX::XMConvertToDegrees(euler.z);

}
