#pragma once

#include "DrawTerrain.h"

class MoveTerrain :public DrawTerrain
{
public:

	void Init()override;

	void Update()override;
	void PostUpdate()override;

	void DrawLit()override;

	virtual void Editor_ImGui() override;

	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override
	{
		DrawTerrain::Deserialize(jsonObj);

		
		KdJsonUtility::GetValue(jsonObj, "Speed", &m_moveSpeed);
		KdJsonUtility::GetArray(jsonObj, "MoveVec", &m_moveVec.x, 3);
		KdJsonUtility::GetValue(jsonObj, "WaitTime", &m_waitTime);
		KdJsonUtility::GetValue(jsonObj, "AnimWait", &m_animWaitTime);
		KdJsonUtility::GetValue(jsonObj, "MoveTime", &m_moveTime);
		KdJsonUtility::GetArray(jsonObj, "StartPos", &m_startPos.x, 3);
		KdJsonUtility::GetArray(jsonObj, "EndPos", &m_endPos.x, 3);
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override
	{
		DrawTerrain::Serialize(outJson);

		outJson["Speed"] = m_moveSpeed;
		outJson["MoveVec"] = KdJsonUtility::CreateArray(&m_moveVec.x, 3);
		outJson["WaitTime"] = m_waitTime;
		outJson["AnimWait"] = m_animWaitTime;
		outJson["MoveTime"] = m_moveTime;
		outJson["StartPos"] = KdJsonUtility::CreateArray(&m_startPos.x, 3);
		outJson["EndPos"] = KdJsonUtility::CreateArray(&m_endPos.x, 3);
	}

private:

	Math::Color					m_modelColor = kWhiteColor;

	float						m_waitTime = 0.0f;
	float						m_durationWait = 0.0f;
	float						m_moveSpeed = 0.0f;
	float						m_moveTime = 1.0f;
	float						m_durationMove = 0.0f;
	Math::Vector3				m_startPos = {};
	Math::Vector3				m_endPos = {};
	Math::Vector3				m_moveVec = {};

	bool						m_isMove = false;
	bool						m_isMovable = false;

	float						m_animWaitTime = 0.0f;
	float						m_durationAnimWait = 0.0f;
	
};