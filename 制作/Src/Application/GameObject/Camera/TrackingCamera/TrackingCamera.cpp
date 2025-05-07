#include"../CameraBase.h"
#include"TrackingCamera.h"

void TrackingCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateTranslation(3, 12.5f, -10.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	m_pos = {};
}

void TrackingCamera::PostUpdate()
{
	Math::Vector3 vMove=Math::Vector3::Zero;

	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpTarget.lock();
	
	if (_spTarget == nullptr) { return; }

	Math::Vector3 targetPos = _spTarget->GetMatrix().Translation();

	//----------------------------------------------------------------
	// ⓵　現在のカメラの注視点と視点を使って、XZ平面上での、
	//     注視点から視点までのベクトル(toCameraPosXZ)と長さ(toCameraPosXZLen)を求める。
	//----------------------------------------------------------------
//	Math::Vector3 toCameraPosXZ = m_pos - m_target;
//	float height = toCameraPosXZ.y;                  //視点へのY方向の高さは、後で使うのでバックアップしておく。
//	toCameraPosXZ.y = 0.0f;                          //XZ平面にするので、Yは0にする。
//	float toCameraPosXZLen = toCameraPosXZ.Length(); //XZ平面上での視点と注視点の距離を求める。
//	toCameraPosXZ.Normalize();                       //単位ベクトル化。
//
//	//----------------------------------------------------------------
//	// ⓶　新しい注視点をアクターの座標から決める。
//	//----------------------------------------------------------------
//	Math::Vector3 target = targetPos;
//	target.y += 30.0f;
//
//	
//	//----------------------------------------------------------------
//	// ⓷　新しい注視点と現在のカメラの視点を使って、XZ平面上での、
//	//     注視点から視点までのベクトル(toNewCameraPos)を求める。
//	//----------------------------------------------------------------
//	Math::Vector3 toNewCameraPos = m_pos-targetPos;
//	toNewCameraPos.y = 0.0f;				    //XZ平面にするので、Yは0にする。
//	toNewCameraPos.Normalize();         //単位ベクトル化。
//
//	//----------------------------------------------------------------
//	// ⓸　１と２と３で求めた情報を使って、新しい視点を決定する。
//	//----------------------------------------------------------------
//	//ちょっとづつ追尾。
//	float weight = 0.2f;  //このウェイトの値は0.0～1.0の値をとる。1.0に近づくほど追尾が強くなる。
//	toNewCameraPos = toNewCameraPos * weight + toCameraPosXZ * (1.0f - weight);
//	toNewCameraPos.Normalize();
//	toNewCameraPos *= toCameraPosXZLen;
//	toNewCameraPos.y = height;
//	Math::Vector3 pos = target + toNewCameraPos;  //これで新しい視点が決定。
//
////	if ()
//	{
//
//		//----------------------------------------------------------------
//		//角度調整
//		//----------------------------------------------------------------
//		Math::Matrix nowRotMat = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_DegAng.y));
//		//➁現在の方向を求める
//		Math::Vector3 nowVec = Math::Vector3::TransformNormal(Math::Vector3(1, 0, 1), nowRotMat);
//
//		//向きたい方向
//		Math::Vector3 toVec = _spTarget->GetMatrix().Backward();
//		toVec.y = 0.0f;
//		toVec.Normalize();
//
//		//内積を使って回転する角度を求める
//		//ベクトルA*ベクトルB*cosΘ(ベクトルAとベクトルBのなす角)
//		//			1	*	1	* cosΘ			
//		float d = nowVec.Dot(toVec);
//		//dの中にはコサインΘが入っている
//
//		//角度求める(でも残念ながらラジアン角)11
//		float ang = DirectX::XMConvertToDegrees(acos(d));
//
//		//内積から角度を求めて少しでも角度が変わったら
//		//ゆっくり回転するようにする
//		if (ang >= 0.1f)
//		{
//			if (ang > 5)
//			{
//				ang = 5.0f;
//			}
//
//			//外積を求める（どっっちに回転するのか調べる）
//			Math::Vector3 c = toVec.Cross(nowVec);
//
//			if (c.y >= 0)
//			{
//				//右回転
//				m_DegAng.y -= ang;
//			}
//			else
//			{
//				//左回転
//				m_DegAng.y += ang;
//			}
//		}
//
//		if (m_DegAng.y > 360)
//		{
//			m_DegAng.y -= 360;
//		}
//		else if (m_DegAng.y < 0)
//		{
//			m_DegAng.y += 360;
//		}
//
//
//	}
//
//	//回転行列
//	Math::Matrix rotMat = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_DegAng.y));
//
//	//----------------------------------------------------------------
//	// ⓹　視点と注視点をカメラに設定して終わり。
//	//----------------------------------------------------------------
//	m_pos = pos;
//	m_target = target;


	m_pos = Math::Vector3::Lerp(
		m_pos,
		targetPos,
		0.2f			//進行速度*デルタタイム
		);

	UpdateRotateByMouse();
	m_mRotation = GetRotationMatrix();

	m_mWorld = m_mLocalPos *m_mRotation * Math::Matrix::CreateTranslation(m_pos)  ;
//	m_mWorld =  rotMat * Math::Matrix::CreateTranslation(m_pos);
}

