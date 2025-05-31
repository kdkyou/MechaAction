#include "KdAnimation.h"
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

//前animationとの補完////////////////////
void KdAnimationData::Node::InterpolateComp(Math::Matrix& rDst, KeyInfo _keyInfo, float time, float compCnt)
{
	// ベクターによる拡縮補間
	bool isChange = false;
	Math::Matrix scale;
	Math::Vector3 resultVec;
	if (InterpolateScalesComp(resultVec, _keyInfo, time, compCnt))
	{
		scale = scale.CreateScale(resultVec);
		isChange = true;
	}

	// クォタニオンによる回転補間
	Math::Matrix rotate;
	Math::Quaternion resultQuat;
	if (InterpolateRotationsComp(resultQuat, _keyInfo, time, compCnt))
	{
		rotate = rotate.CreateFromQuaternion(resultQuat);
		isChange = true;
	}

	// ベクターによる座標補間
	Math::Matrix trans;
	if (InterpolateTranslationsComp(resultVec, _keyInfo, time, compCnt))
	{
		trans = trans.CreateTranslation(resultVec);
		isChange = true;
	}

	if (isChange)
	{
		rDst = scale * rotate * trans;
	}
}

bool KdAnimationData::Node::InterpolateTranslationsComp(Math::Vector3& result, KeyInfo _keyInfo, float time, float compCnt)
{
	if (m_translations.size() == 0)return false;

	// キー位置検索
	UINT keyIdx = BinarySearchNextAnimKey(m_translations, time);


	auto& prev = _keyInfo.m_translations;	// 前のキー
	auto& next = m_translations[keyIdx];		// 次のキー
	// 補間
	result = DirectX::XMVectorLerp(
		prev.m_vec,
		next.m_vec,
		compCnt
	);

	return true;
}

bool KdAnimationData::Node::InterpolateRotationsComp(Math::Quaternion& result, KeyInfo _keyInfo, float time, float compCnt)
{
	if (m_rotations.size() == 0)return false;

	// キー位置検索
	UINT keyIdx = BinarySearchNextAnimKey(m_rotations, time);


	auto& prev = _keyInfo.m_rotations;	// 前のキー
	auto& next = m_rotations[keyIdx];		// 次のキー
	// 補間
// 補間
	result = DirectX::XMQuaternionSlerp(
		prev.m_quat,
		next.m_quat,
		compCnt
	);

	return true;
}

bool KdAnimationData::Node::InterpolateScalesComp(Math::Vector3& result, KeyInfo _keyInfo, float time, float compCnt)
{
	if (m_scales.size() == 0)return false;

	// キー位置検索
	UINT keyIdx = BinarySearchNextAnimKey(m_scales, time);


	auto& prev = _keyInfo.m_scales;	// 前のキー
	auto& next = m_scales[keyIdx];		// 次のキー
	// 補間
	result = DirectX::XMVectorLerp(
		prev.m_vec,
		next.m_vec,
		compCnt
	);

	return true;
}
//////////////////////////////////////////////////

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

	// 全てのアニメーションノード（モデルの行列を補間する情報）の行列補間を実行する
	int i = 0;
	for (auto& rAnimNode : m_spAnimation->m_nodes)
	{
		// 対応するモデルノードのインデックス
		UINT idx = rAnimNode.m_nodeOffset;

		auto prev = rNodes[idx].m_localTransform;

		// アニメーションデータによる行列補間
		if (m_isComp)
		{

			//前animationとの補完
			rAnimNode.InterpolateComp(rNodes[idx].m_localTransform, m_lastKeyInfo[i], m_time, m_compCnt);
		}
		else
		{
			rAnimNode.Interpolate(rNodes[idx].m_localTransform, m_time);
		}


		prev = rNodes[idx].m_localTransform;

		i++;
	}

	// アニメーションのフレームを進める
	m_time += speed;

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
			m_time = m_spAnimation->m_maxLength;
		}
	}

	//animation補完のカウント
	if (m_isComp)
	{
		m_compCnt += KdFPSController::GetInstance().GetDeltaTime() * m_compSpd;
		if (m_compCnt > 1.0)
		{
			//補完終了
			m_isComp = false;
		}
	}
}

void KdAnimator::SetLastKeyInfo()
{
	if (m_spAnimation == nullptr)return;

	//リストのクリア
	m_lastKeyInfo.clear();

	//ノード分回す
	for (auto& rAnimNode : m_spAnimation->m_nodes)
	{
		KeyInfo _keyInfo;

		// キー位置検索
		UINT _keyIdx = BinarySearchNextAnimKey(rAnimNode.m_translations, m_time);
		// 配列外のキーなら、最後のデータを返す
		if (_keyIdx >= rAnimNode.m_translations.size()) {
			_keyIdx = rAnimNode.m_translations.size() - 1;
		}
		_keyInfo.m_translations = rAnimNode.m_translations[_keyIdx];


		_keyIdx = BinarySearchNextAnimKey(rAnimNode.m_rotations, m_time);
		// 配列外のキーなら、最後のデータを返す
		if (_keyIdx >= rAnimNode.m_rotations.size()) {
			_keyIdx = rAnimNode.m_rotations.size() - 1;
		}
		_keyInfo.m_rotations = rAnimNode.m_rotations[_keyIdx];

		_keyIdx = BinarySearchNextAnimKey(rAnimNode.m_scales, m_time);
		// 配列外のキーなら、最後のデータを返す
		if (_keyIdx >= rAnimNode.m_scales.size()) {
			_keyIdx = rAnimNode.m_scales.size() - 1;
		}
		_keyInfo.m_scales = rAnimNode.m_scales[_keyIdx];

		m_lastKeyInfo.push_back(_keyInfo);
	}
}
