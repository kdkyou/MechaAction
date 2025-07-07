#include "CharacterBase.h"

void CharacterBase::GenerateDepthMapFromLight()
{
	if (m_spModelWork)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelWork, m_mWorld);
	}

	if (m_spModelData)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModelData, m_mWorld);
	}
}

void CharacterBase::HitDamage(float damage)
{
	if (m_hp - damage < 0)
	{
		m_hp = 0;
	}
	else
	{
		m_hp -= damage;
	}
}

bool CharacterBase::SearchDetect(const Math::Vector3& hitPos, const Math::Matrix& myMat, float viewRange)
{
	// オブジェクトの位置から方向を取得
	auto targetDirect = hitPos - myMat.Translation();

	// 現在の方向
	auto nowDir = myMat.Backward();

	// オブジェクトの角度算出
	float d = nowDir.Dot(targetDirect);
	// 丸め誤差使用
	d = std::clamp(d, -1.0f, 1.0f);
	// デグリー角に
	float targetAngle = DirectX::XMConvertToDegrees(acos(d));

	if (targetAngle < viewRange)
	{
		return true;
	}

	return false;
}
