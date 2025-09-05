#include "TPSCamera.h"

#include"../../Character/CharacterBase.h"

#include "../../../Scene/SceneManager.h"

void TPSCamera::Editor_ImGui()
{
	CameraBase::Editor_ImGui();
	ImGui::DragFloat3((const char*)u8"相対座標", &m_localPos.x);
}

void TPSCamera::Deserialize(const nlohmann::json& jsonObj)
{
	CameraBase::Deserialize(jsonObj);

	KdJsonUtility::GetArray(jsonObj, "LocalPos", &m_localPos.x, 3);
}

void TPSCamera::Serialize(nlohmann::json& outJson) const
{
	CameraBase::Serialize(outJson);
	outJson["LocalPos"] = KdJsonUtility::CreateArray(&m_localPos.x, 3);

}

void TPSCamera::Init()
{
	// 親クラスの初期化呼び出し
	if (!m_spCamera)
	{
		m_spCamera = std::make_shared<KdCamera>();
	}

	// ↓画面中央座標
	m_FixMousePos.x = 640;
	m_FixMousePos.y = 360;

	KdEffekseerManager::GetInstance().SetCamera(m_spCamera);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	m_name = "TPS";

	ShowCursor(false);
}

void TPSCamera::PostUpdate()
{
	// ターゲットの行列(有効な場合利用する)
	Math::Matrix								_targetMat = Math::Matrix::Identity;
	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpLook.lock();
	if (_spTarget)
	{
		_targetMat = Math::Matrix::CreateTranslation(_spTarget->GetPos());
	}

	//// カメラの回転
	//UpdateRotateByMouse();
	auto ang = m_rot * KdToRadians;
	m_mLocalPos = Math::Matrix::CreateTranslation(m_localPos);
	m_mRotation = Math::Matrix::CreateFromYawPitchRoll(ang);
	m_mWorld = m_mLocalPos * m_mRotation * _targetMat;

	//// ↓めり込み防止の為の座標補正計算↓
	//// ①当たり判定(レイ判定)用の情報作成
	//KdCollider::RayInfo rayInfo;
	//// レイの発射位置を設定
	//rayInfo.m_pos = GetPos();

	//// レイの発射方向を設定
	//rayInfo.m_dir = Math::Vector3::Down;
	//// レイの長さを設定
	//rayInfo.m_range = 1000.f;
	//if (_spTarget)
	//{
	//	Math::Vector3 _targetPos = _spTarget->GetPos();
	//	_targetPos.y += 0.1f;
	//	rayInfo.m_dir = _targetPos - GetPos();
	//	rayInfo.m_range = rayInfo.m_dir.Length();
	//	rayInfo.m_dir.Normalize();
	//}

	//// 当たり判定をしたいタイプを設定
	//rayInfo.m_type = KdCollider::TypeGround;

	//// ②HIT判定対象オブジェクトに総当たり
	//for (auto& Obj : SceneManager::Instance().GetTerrainList())
	//{
	//	if (Obj)
	//	{
	//		std::list<KdCollider::CollisionResult> retRayList;
	//		Obj->Intersects(rayInfo, &retRayList);

	//		// ③ 結果を使って座標を補完する
	//		// レイに当たったリストから一番近いオブジェクトを検出
	//		float maxOverLap = 0;
	//		Math::Vector3 hitPos = {};
	//		bool hit = false;
	//		for (auto& ret : retRayList)
	//		{
	//			// レイを遮断しオーバーした長さが
	//			// 一番長いものを探す
	//			if (maxOverLap < ret.m_overlapDistance)
	//			{
	//				maxOverLap = ret.m_overlapDistance;
	//				hitPos = ret.m_hitPos;
	//				hit = true;
	//			}
	//		}
	//		if (hit)
	//		{
	//			// 何かしらの障害物に当たっている
	//			Math::Vector3 _hitPos = hitPos;
	//			_hitPos += rayInfo.m_dir * 0.4f;
	//			SetPos(_hitPos);
	//		}
	//	}
	//}

	CameraBase::PostUpdate();
}
