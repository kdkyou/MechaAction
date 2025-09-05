#pragma once

// アニメーションキー(クォータニオン
struct KdAnimKeyQuaternion
{
	float				m_time = 0;		// 時間
	Math::Quaternion	m_quat;			// クォータニオンデータ
};

// アニメーションキー(ベクトル
struct KdAnimKeyVector3
{
	float				m_time = 0;		// 時間
	Math::Vector3		m_vec;			// 3Dベクトルデータ
};

//キー情報のみ
struct KeyInfo
{
	KdAnimKeyVector3 m_translations;
	KdAnimKeyQuaternion m_rotations;
	KdAnimKeyVector3 m_scales;
};

//============================
// アニメーションデータ
//============================
struct KdAnimationData
{
	// アニメーション名
	std::string		m_name;
	// アニメの長さ
	float			m_maxLength = 0;

	// １ノードのアニメーションデータ
	struct Node
	{
		int			m_nodeOffset = -1;	// 対象モデルノードのOffset値

		// 各チャンネル
		std::vector<KdAnimKeyVector3>		m_translations;	// 位置キーリスト
		std::vector<KdAnimKeyQuaternion>	m_rotations;	// 回転キーリスト
		std::vector<KdAnimKeyVector3>		m_scales;		// 拡縮キーリスト

		void Interpolate(Math::Matrix& rDst, float time);
		bool InterpolateTranslations(Math::Vector3& result, float time);
		bool InterpolateRotations(Math::Quaternion& result, float time);
		bool InterpolateScales(Math::Vector3& result, float time);


		void InterpolateComp(Math::Matrix& rDst, KeyInfo _keyInfo, float time, float compCnt);
		bool InterpolateTranslationsComp(Math::Vector3& result, KeyInfo _keyInfo, float time, float compCnt);
		bool InterpolateRotationsComp(Math::Quaternion& result, KeyInfo _keyInfo, float time, float compCnt);
		bool InterpolateScalesComp(Math::Vector3& result, KeyInfo _keyInfo, float time, float compCnt);
	};

	// 全ノード用アニメーションデータ
	std::vector<Node>	m_nodes;
};

class KdAnimator
{
public:




	inline void SetAnimation(const std::shared_ptr<KdAnimationData>& rData,const float compSpd, bool isLoop = true, bool isComp = true)
	{
		if (m_spAnimation && isComp)
		{
			//最後のキー情報を取得
			SetLastKeyInfo();
			m_compCnt = 0.0;
			m_isComp = true;
			m_compSpd = compSpd;
		}

		m_spAnimation = rData;
		m_isLoop = isLoop;

		m_time = 0.0f;
	}

	// アニメーションが終了してる？
	bool IsAnimationEnd() const
	{
		if (m_spAnimation == nullptr) { return true; }
		if (m_time >= m_spAnimation->m_maxLength) { return true; }
		/*float progress = m_time / m_spAnimation->m_maxLength;
		if (1.0f <=progress) { return true; }*/

		return false;
	}

	// アニメーションの更新
	void AdvanceTime(std::vector<KdModelWork::Node>& rNodes, float speed = 1.0f);

	//現在のアニメーション位置を返す
	const float GetAdvanceTime() const { return m_time; }
	const float GetProgress()const { return m_time / m_spAnimation->m_maxLength; }
	const float GetComp()const { return m_compCnt; }
	const bool IsComp()const { return m_isComp; }
	void SetAdvanceTime(const float advanceTime) { m_time = advanceTime; }
	//現在のanimation名を返す
	const std::string GetAnimName() {

		std::string _name = {};
		if (m_spAnimation != nullptr)_name = m_spAnimation->m_name;
		return _name;
	}

	//ループ終了
	void SetLoopEnd() { m_isLoop = false; }

private:

	std::vector<KeyInfo> m_lastKeyInfo;				//最後のキー情報
	std::shared_ptr<KdAnimationData>	m_spAnimation = nullptr;	// 再生するアニメーションデータ

	float m_time = 0.0f;

	bool m_isLoop = false;

	//前animationとの補完
	void SetLastKeyInfo();
	bool m_isComp = false;
	float  m_compCnt = 0.0;

	float m_compSpd = 0.0f;
};
