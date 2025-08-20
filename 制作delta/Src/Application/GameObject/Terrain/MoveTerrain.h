#pragma once

class MoveTerrain :public KdGameObject
{
public:

	void Init()override;

	void Update()override;
	void PostUpdate()override;

	void DrawLit()override;
	void DrawUnLit()override;

	void SetPos(const Math::Vector3& vec)override { m_pos = vec; }

	void SetModel(const std::string& path);

	virtual void Editor_ImGui() override;

	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override
	{
		KdGameObject::Deserialize(jsonObj);

		KdJsonUtility::GetValue(jsonObj, "ModelPath", &m_modelPath);
		KdJsonUtility::GetValue(jsonObj, "AnimPath", &m_animPath);
		KdJsonUtility::GetArray(jsonObj, "Emmisive", &m_emmisive.x, 3);
		KdJsonUtility::GetValue(jsonObj, "AnimSpeed", &m_animSpeed);
		KdJsonUtility::GetValue(jsonObj, "Speed", &m_moveSpeed);
		KdJsonUtility::GetArray(jsonObj, "MoveVec", &m_moveVec.x, 3);
		KdJsonUtility::GetValue(jsonObj, "WaitTime", &m_waitTime);

		SetModel(m_modelPath);
		AnimationPlay(m_animPath);
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override
	{
		KdGameObject::Serialize(outJson);

		outJson["ModelPath"] = m_modelPath;
		outJson["AnimPath"] = m_animPath;
		outJson["Emmisive"] = KdJsonUtility::CreateArray(&m_emmisive.x, 3);
		outJson["AnimSpeed"] = m_animSpeed;
		outJson["Speed"] = m_moveSpeed;
		outJson["MoveVec"] = KdJsonUtility::CreateArray(&m_moveVec.x, 3);
		outJson["WaitTime"] = m_waitTime;

	}

private:

	void AnimationPlay(const std::string& path);

	std::shared_ptr<KdModelWork> m_spModel = nullptr;

	std::string					m_modelPath = "";

	std::shared_ptr<KdAnimator> m_spAnimator = nullptr;

	std::string					m_animPath = "";

	Math::Color					m_modelColor = kWhiteColor;

	float						m_waitTime = 0.0f;
	float						m_durationWait = 0.0f;
	float						m_moveSpeed = 0.0f;
	Math::Vector3				m_moveVec = {};

	float						m_animSpeed = 0.0f;

	Math::Vector3				m_emmisive = Math::Vector3::Zero;

	std::vector<PointLight>		m_points;

};