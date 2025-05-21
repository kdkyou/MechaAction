#include"RockCamera.h"


void RockCamera::Init()
{
	// 親クラスの初期化呼び出し
	CameraBase::Init();

	// 注視点
	m_mLocalPos = Math::Matrix::CreateTranslation(0, 12.5f, -10.0f);

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);


}

void RockCamera::PostUpdate()
{
	// ターゲットの行列(有効な場合利用する)
	Math::Matrix								_targetMat = Math::Matrix::Identity;
	const std::shared_ptr<const KdGameObject>	_spTarget = m_wpTarget.lock();
	if (_spTarget)
	{
		_targetMat = Math::Matrix::CreateTranslation(_spTarget->GetPos());
	}

	Rock();

	m_mRotation = GetRotationMatrix();
	m_mWorld = m_mLocalPos * m_mRotation * _targetMat;

}

void RockCamera::Rock()
{
	const std::shared_ptr<const KdGameObject> spTarget = m_wpRockTarget.lock();
	if (spTarget == nullptr) { return; }

	Math::Vector3 targetPos = spTarget->GetMatrix().Translation();
	Math::Vector3 pos = m_mWorld.Translation();

	Math::Vector3 nowVec = m_mWorld.Backward();

	Math::Vector3 toVec = targetPos - pos;
	toVec.y = 0.0f;
	toVec.Normalize();


			//内積を使って回転する角度を求める
			//ベクトルA*ベクトルB*cosΘ(ベクトルAとベクトルBのなす角)
			//			1	*	1	* cosΘ			
			float d = nowVec.Dot(toVec);
			//dの中にはコサインΘが入っている
	
			//角度求める(でも残念ながらラジアン角)11
			float ang = DirectX::XMConvertToDegrees(acos(d));
	
			//内積から角度を求めて少しでも角度が変わったら
			//ゆっくり回転するようにする
			if (ang >= 0.1f)
			{
				if (ang > 10)
				{
					ang = 10.0f;
				}
	
				//外積を求める（どっっちに回転するのか調べる）
				Math::Vector3 c = toVec.Cross(nowVec);
	
				if (c.y >= 0)
				{
					//右回転
					m_DegAng.y -= ang;
				}
				else
				{
					//左回転
					m_DegAng.y += ang;
				}
			}
	
			if (m_DegAng.y > 360)
			{
				m_DegAng.y -= 360;
			}
			else if (m_DegAng.y < 0)
			{
				m_DegAng.y += 360;
			}

}
