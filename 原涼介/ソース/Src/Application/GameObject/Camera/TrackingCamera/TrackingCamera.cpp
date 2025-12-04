
#include"TrackingCamera.h"

#include "../../../main.h"

#include"../../Character/CharacterBase.h"

#include "../CameraManager.h"

#include "../../../Scene/SceneManager.h"


void TrackingCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateTranslation(m_localPos);

	m_localPos = { 3, 14.5f, -22.0f };
	m_basePos = { 3.0f, 14.5f, -28.0f };

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);


	if (m_wpTarget.expired() == false)
	{
		m_mWorld = m_mLocalPos * m_wpTarget.lock()->GetMatrix();

		m_pos = m_wpTarget.lock()->GetMatrix().Translation();
		
		m_DegAng;
	}


	m_name = "Tracking";
		CameraManager::Instance().EnableChangedCamera(false);

	ShowCursor(false);

}

void TrackingCamera::PostUpdate()
{

	Math::Vector3 vMove=Math::Vector3::Zero;

	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpTarget.lock();
	
	if (_spTarget == nullptr) { return; }

	Math::Vector3 targetPos = _spTarget->GetMatrix().Translation();

	m_unEnableChangeTime -= KdFPSController::GetInstance().GetDeltaTime();

	if (m_unEnableChangeTime < 0.0f)
	{
		m_unEnableChangeTime = 0;
		CameraManager::Instance().EnableChangedCamera(true);
	}
	
	
	// 画面範囲外判定
	CheckScreenPull();
	
	Math::Vector3 pos = {};
	float dt = KdFPSController::GetInstance().GetDeltaTime();
	// 補正のスムーズ化
	// - 範囲外時は秒単位の速度で補正（瞬間移動にならないように）
	// - 範囲内は通常の速度（m_speed）を DeltaTime でスケール
	const float pullSpeedPerSecond = 18.0f;	// 範囲外補正の速さ（秒あたりのLerp係数換算）
	const float maxPullLerp = 0.89f;			// Lerp係数の上限（安全対策）

	if (m_isPull)
	{
		pos = m_localPos;
		m_mLocalPos = Math::Matrix::CreateTranslation(pos);

		float lerpFactor = pullSpeedPerSecond * dt;
		lerpFactor = (lerpFactor > maxPullLerp) ? maxPullLerp : lerpFactor;

		m_pos = Math::Vector3::Lerp(
			m_pos,
			targetPos,
			lerpFactor
		);
	}
	else {
		pos = m_localPos;
		m_mLocalPos = Math::Matrix::CreateTranslation(pos);

		// 通常時は速度にデルタタイムを掛けた滑らかな補間
		float lerpFactor = m_speed * dt;
		if (lerpFactor > 1.0f) lerpFactor = 1.0f;

		m_pos = Math::Vector3::Lerp(
			m_pos,
			targetPos,
			lerpFactor			// 進行速度*デルタタイム
		);
	}

	UpdateRotateByMouse();
	m_mRotation = GetRotationMatrix();

	m_mWorld = m_mLocalPos *m_mRotation * Math::Matrix::CreateTranslation(m_pos);

	TerrainCheck();

	CameraBase::PostUpdate(); 
}

void TrackingCamera::Editor_ImGui()
{
	ImGui::SliderFloat("LerpSpeed", &m_speed, 0.0f, 100.0f);

	ImGui::Text("pos x:%.2f,y:%.2f,z:%.2f",m_mLocalPos.Translation().x, m_mLocalPos.Translation().y, m_mLocalPos.Translation().z);

}

void TrackingCamera::CheckScreenPull()
{
	// ヒステリシス（状態反転に必要な継続時間）を導入して
		// 「範囲外→範囲内」の頻繁な切り替えを防ぐ。
	static float s_stateTimer = 0.0f;
	const float hysteresisTime = 0.12f; // 秒、継続して状態が変化してから反映する

	auto _spTarget = m_wpTarget.lock();
	if (_spTarget == nullptr) return;
	auto targetPos = _spTarget->GetPos();

	Math::Vector3 clipPos = {};
	m_spCamera->ConvertWorldToScreenDetail(targetPos, clipPos);

	// 閾値（外側と内側でズレを持たせる）
	const float rightOuter = 430.0f;
	const float rightInner = 300.0f;
	const float leftOuter = -330.0f;
	const float leftInner = -300.0f;
	const float bottomOuter = -300.0f;
	const float bottomInner = -290.0f;
	// 上方向閾値が必要ならここに追加（現在の実装では下方向だけ使用されているため省略）

	bool wantPull = false;

	// 範囲外判定（外側閾値）
	if (clipPos.x > rightOuter || clipPos.x < leftOuter || clipPos.y < bottomOuter)
	{
		wantPull = true;
	}
	// 範囲内判定（内側閾値）
	else if (clipPos.x < rightInner && clipPos.x > leftInner && clipPos.y > bottomInner)
	{
		wantPull = false;
	}
	else
	{
		// どちらでもない（境界帯） -> 現在の状態を維持したいので wantPull = m_isPull とする
		wantPull = m_isPull;
	}

	// タイマー方式で安定化: 新しい状態が一定時間続いたら反映
	if (wantPull != m_isPull)
	{
		s_stateTimer += KdFPSController::GetInstance().GetDeltaTime();
		if (s_stateTimer >= hysteresisTime)
		{
			m_isPull = wantPull;
			s_stateTimer = 0.0f;
		}
	}
	else
	{
		// 状態が同じならタイマーをリセット
		s_stateTimer = 0.0f;
	}

	Application::Instance().m_log.AddLog("ClipPos X:%.2f,Y:%.2f, isPull:%d\n", clipPos.x, clipPos.y, (int)m_isPull);


}

