﻿#include "KdAnimation.h"
#include "../Direct3D/KdModel.h"

// 二分探索で、指定時間から次の配列要素のKeyIndexを求める関数
// list		… キー配列
// time		… 時間
// 戻り値	… 次の配列要素のIndex
template<class T>
int BinarySearchNextAnimKey(const std::vector<T>& list, float time)
{
	int low = 0;
	int high = (int)list.size();
	while (low < high)
	{
		int mid = (low + high) / 2;
		float midTime = list[mid].m_time;

		if (midTime <= time) low = mid + 1;
		else high = mid;
	}
	return low;
}

bool KdAnimationData::Node::InterpolateTranslations(Math::Vector3& result, float time)
{
	if (m_translations.size() == 0)return false;

	// キー位置検索
	UINT keyIdx = BinarySearchNextAnimKey(m_translations, time);

	// 先頭のキーなら、先頭のデータを返す
	if (keyIdx == 0) {
		result = m_translations.front().m_vec;
		return true;
	}
	// 配列外のキーなら、最後のデータを返す
	else if (keyIdx >= m_translations.size()) {
		result = m_translations.back().m_vec;
		return true;
	}
	// それ以外(中間の時間)なら、その時間の値を補間計算で求める
	else {
		auto& prev = m_translations[keyIdx - 1];	// 前のキー
		auto& next = m_translations[keyIdx];		// 次のキー
		// 前のキーと次のキーの時間から、0～1間の時間を求める
		float f = (time - prev.m_time) / (next.m_time - prev.m_time);
		// 補間
		result = DirectX::XMVectorLerp(
			prev.m_vec,
			next.m_vec,
			f
		);
	}

	return true;
}

bool KdAnimationData::Node::InterpolateRotations(Math::Quaternion& result, float time)
{
	if (m_rotations.size() == 0)return false;

	// キー位置検索
	UINT keyIdx = BinarySearchNextAnimKey(m_rotations, time);
	// 先頭のキーなら、先頭のデータを返す
	if (keyIdx == 0) {
		result = m_rotations.front().m_quat;
	}
	// 配列外のキーなら、最後のデータを返す
	else if (keyIdx >= m_rotations.size()) {
		result = m_rotations.back().m_quat;
	}
	// それ以外(中間の時間)なら、その時間の値を補間計算で求める
	else {
		auto& prev = m_rotations[keyIdx - 1];	// 前のキー
		auto& next = m_rotations[keyIdx];		// 次のキー
		// 前のキーと次のキーの時間から、0～1間の時間を求める
		float f = (time - prev.m_time) / (next.m_time - prev.m_time);
		// 補間
		result = DirectX::XMQuaternionSlerp(
			prev.m_quat,
			next.m_quat,
			f
		);
	}

	return true;
}

bool KdAnimationData::Node::InterpolateScales(Math::Vector3& result, float time)
{
	if (m_scales.size() == 0)return false;

	// キー位置検索
	UINT keyIdx = BinarySearchNextAnimKey(m_scales, time);

	// 先頭のキーなら、先頭のデータを返す
	if (keyIdx == 0) {
		result = m_scales.front().m_vec;
		return true;
	}
	// 配列外のキーなら、最後のデータを返す
	else if (keyIdx >= m_scales.size()) {
		result = m_scales.back().m_vec;
		return true;
	}
	// それ以外(中間の時間)なら、その時間の値を補間計算で求める
	else {
		auto& prev = m_scales[keyIdx - 1];	// 前のキー
		auto& next = m_scales[keyIdx];		// 次のキー
		// 前のキーと次のキーの時間から、0～1間の時間を求める
		float f = (time - prev.m_time) / (next.m_time - prev.m_time);
		// 補間
		result = DirectX::XMVectorLerp(
			prev.m_vec,
			next.m_vec,
			f
		);
	}

	return true;
}

void KdAnimationData::Node::Interpolate(Math::Matrix& rDst, float time)
{
	// ベクターによる拡縮補間
	bool isChange = false;
	Math::Matrix scale;
	Math::Vector3 resultVec;
	if (InterpolateScales(resultVec, time))
	{
		scale = scale.CreateScale(resultVec);
		isChange = true;
	}

	// クォタニオンによる回転補間
	Math::Matrix rotate;
	Math::Quaternion resultQuat;
	if (InterpolateRotations(resultQuat, time))
	{
		rotate = rotate.CreateFromQuaternion(resultQuat);
		isChange = true;
	}

	// ベクターによる座標補間
	Math::Matrix trans;
	if (InterpolateTranslations(resultVec, time))
	{
		trans = trans.CreateTranslation(resultVec);
		isChange = true;
	}

	if (isChange)
	{
		rDst = scale * rotate * trans;
	}
}

void KdAnimator::AdvanceTime(std::vector<KdModelWork::Node>& rNodes, float speed)
{
	if (!m_spAnimation) { return; }

	bool isBlending = (m_spNextAnimation && m_blendTime < m_blendDuration);

	// 全てのアニメーションノード（モデルの行列を補間する情報）の行列補間を実行する
	for (auto& rAnimNode : m_spAnimation->m_nodes)
	{
		// 対応するモデルノードのインデックス
		UINT idx = rAnimNode.m_nodeOffset;

		

		Math::Matrix matA = Math::Matrix::Identity;
		Math::Matrix matB = Math::Matrix::Identity;

		//現在のアニメーションのボーンのマトリックス情報
		if (idx < m_spAnimation->m_nodes.size())
		{
			m_spAnimation->m_nodes[idx].Interpolate(matA, m_time);
		}

		//次のアニメーションのボーンのマトリックス情報
		if (isBlending && idx < m_spNextAnimation->m_nodes.size())
		{
			m_spNextAnimation->m_nodes[idx].Interpolate(matB, 0.0f);
		}
		else
		{
			matB = matA;
		}

		//アニメーション遷移時
		if (isBlending)
		{
			float t = m_blendTime / m_blendDuration;
			Math::Vector3 sA, sB, pA, pB;
			Math::Quaternion rA, rB;

			matA.Decompose(sA, rA, pA);
			matB.Decompose(sB, rB, pB);

			Math::Vector3 s = sA+((sB-Math::Vector3::One)*t);
			Math::Vector3 p = XMVectorLerp(pA, pB, t);
			Math::Quaternion r = XMQuaternionSlerp(rA, rB, t);

			rNodes[idx].m_localTransform =
				Math::Matrix::CreateScale(s) *
				Math::Matrix::CreateFromQuaternion(r) *
				Math::Matrix::CreateTranslation(p);
		}
		else
		{
			auto prev = rNodes[idx].m_localTransform;

			// アニメーションデータによる行列補間
			rAnimNode.Interpolate(rNodes[idx].m_localTransform, m_time);

			prev = rNodes[idx].m_localTransform;
		}
	}
	// アニメーションのフレームを進める
	m_time += speed;

	if (isBlending)
	{
		m_blendTime += speed;
		//ブレンド完了
		if (m_blendTime >= m_blendDuration)
		{
			m_spAnimation = m_spNextAnimation;
			m_spNextAnimation = nullptr;
			m_time = 0.0f;
		}
	}

	// アニメーションデータの最後のフレームを超えたら
	if (m_time >= m_spAnimation->m_maxLength)
	{
		if (m_isLoop)
		{
			// アニメーションの最初に戻る（ループさせる
			m_time = 0.0f;
		}
		else
		{
			//アニメーションの最後の時間に設定
			m_time = m_spAnimation->m_maxLength;
		}
	}
}

void KdAnimator::BlendToAnimation(const std::shared_ptr<KdAnimationData>& nextAnim, float duration,bool isLoop)
{
	m_spNextAnimation = nextAnim;
	m_blendDuration = duration;
	m_blendTime = 0.0f;
	m_isLoop = isLoop;
}
