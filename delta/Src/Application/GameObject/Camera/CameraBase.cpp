#include "CameraBase.h"

#include "CameraManager.h"

#include "../Character/CharacterBase.h"

#include "../../Scene/SceneManager.h"

void CameraBase::Init()
{
	if (!m_spCamera)
	{
		m_spCamera = std::make_shared<KdCamera>();
	}
	// ↓画面中央座標
	m_FixMousePos.x = 640;
	m_FixMousePos.y = 360;

	m_localPos = { 3, 14.5f, -22.0f };

	KdEffekseerManager::GetInstance().SetCamera(m_spCamera);

	ShowCursor(false);
}

void CameraBase::PostUpdate()
{
	if (!m_spCamera) { return; }

	KdAudioManager::Instance().SetListnerMatrix(m_mWorld);
}


void CameraBase::PreDraw()
{
	if (!m_spCamera) { return; }

	m_spCamera->SetCameraMatrix(m_mWorld);
	m_spCamera->SetToShader();
}

void CameraBase::SetLook(const std::weak_ptr<KdGameObject>& target)
{
	if (target.expired() == true) { return; }

	m_wpLook = target;
}

void CameraBase::SetTarget(const std::weak_ptr<CharacterBase>& target)
{
	if (target.expired() == true) { return; }

	m_wpTarget = target;
}

void CameraBase::SetLockTarget(const std::weak_ptr<CharacterBase>& target)
{
	if (target.expired() == true) { return; }

	m_wpLockTarget = target;
}

void CameraBase::Editor_ImGui()
{
	KdGameObject::Editor_ImGui();
}

void CameraBase::Deserialize(const nlohmann::json& jsonObj)
{
	KdJsonUtility::GetArray(jsonObj, "Pos", &m_pos.x, 3);
	KdJsonUtility::GetArray(jsonObj, "Rot", &m_rot.x, 3);
	KdJsonUtility::GetArray(jsonObj, "Scale", &m_scale.x, 3);

	
}

void CameraBase::Serialize(nlohmann::json& outJson) const
{
	outJson["Type"] = CameraManager::Instance().GetNowType();
	
	KdGameObject::Serialize(outJson);

}

void CameraBase::UpdateRotateByMouse()
{
	const std::shared_ptr<CharacterBase> _spTarget = m_wpTarget.lock();
	if (_spTarget->IsDestroy() == true) { return; }
	// マウスでカメラを回転させる処理
	POINT _nowPos;

	GetCursorPos(&_nowPos);
	auto& pad = KeyInput::GetInstance().GetGamePadState();

	bool moveCamera = false;

	POINT _mouseMove{};
	_mouseMove.x = _nowPos.x - m_FixMousePos.x;
	_mouseMove.y = _nowPos.y - m_FixMousePos.y;

	if (_mouseMove.x > 0.0f || _mouseMove.y > 0.0f)
	{
		moveCamera = true;
	}

	if (pad.IsRightThumbStickDown()) {
		_mouseMove.y += m_mouseSpeed.y;
		moveCamera = true;
	}
	if (pad.IsRightThumbStickUp()) {
		_mouseMove.y -= m_mouseSpeed.y;
		moveCamera = true;
	}
	if (pad.IsRightThumbStickLeft()) {
		_mouseMove.x -= m_mouseSpeed.x;
		moveCamera = true;
	}
	if (pad.IsRightThumbStickRight()) {
		_mouseMove.x += m_mouseSpeed.x;
		moveCamera = true;
	}

	

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	// 実際にカメラを回転させる処理(0.15はただの補正値)
	m_DegAng.x += _mouseMove.y * 0.15f;
	m_DegAng.y += _mouseMove.x * 0.15f;

	// 回転制御
	m_DegAng.x = std::clamp(m_DegAng.x, -45.f, 45.f);
}

void CameraBase::TerrainCheck()
{
	auto _spTarget = m_wpTarget.lock();

	KdCollider::RayInfo rayInfo;
	// レイの発射位置を設定
	rayInfo.m_pos = GetPos();

	// レイの発射方向を設定
	rayInfo.m_dir = Math::Vector3::Down;
	// レイの長さを設定
	rayInfo.m_range = 100.f;
	if (_spTarget)
	{
		Math::Vector3 _targetPos = _spTarget->GetPos();
		_targetPos.y += 0.1f;
		rayInfo.m_dir = _targetPos - GetPos();
		rayInfo.m_range = rayInfo.m_dir.Length();
		rayInfo.m_dir.Normalize();
	}

	// 当たり判定をしたいタイプを設定
	rayInfo.m_type = KdCollider::TypeGround;

	// ②HIT判定対象オブジェクトに総当たり
	for (std::weak_ptr<KdGameObject> wpGameObj : SceneManager::Instance().GetTerrainList())
	{
		std::shared_ptr<KdGameObject> spGameObj = wpGameObj.lock();
		if (spGameObj)
		{
			std::list<KdCollider::CollisionResult> retRayList;
			spGameObj->Intersects(rayInfo, &retRayList);

			// ③ 結果を使って座標を補完する
			// レイに当たったリストから一番近いオブジェクトを検出
			float maxOverLap = 0;
			Math::Vector3 hitPos = {};
			bool hit = false;
			for (auto& ret : retRayList)
			{
				// レイを遮断しオーバーした長さが
				// 一番長いものを探す
				if (maxOverLap < ret.m_overlapDistance)
				{
					maxOverLap = ret.m_overlapDistance;
					hitPos = ret.m_hitPos;
					hit = true;
				}
			}
			if (hit)
			{
				// 何かしらの障害物に当たっている
				Math::Vector3 _hitPos = hitPos;
				_hitPos += rayInfo.m_dir * 0.4f;
				SetPos(_hitPos);
			}
		}
	}
}
