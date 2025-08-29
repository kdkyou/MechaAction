#pragma once

#include "DrawTerrain.h"

class EmmisiveTerrain :public DrawTerrain 
{
public:

	EmmisiveTerrain() {}
	virtual ~EmmisiveTerrain()		override {}

	void Init()				override;
	void Update()			override;
	void PostUpdate()		override;
	void DrawLit()			override;

	virtual void Editor_ImGui() override;

	// JSONデータから、クラスの内容を設定
	virtual void Deserialize(const nlohmann::json& jsonObj)override
	{
		DrawTerrain::Deserialize(jsonObj);

		KdJsonUtility::GetValue(jsonObj, "Wait", &m_waitTime);
		KdJsonUtility::GetValue(jsonObj, "ChangeTime", &m_changeTime);
		KdJsonUtility::GetValue(jsonObj, "Chain", &m_isChain);
		KdJsonUtility::GetArray(jsonObj, "SecondEmmisive", &m_secondEmmisive.x, 3);
		KdJsonUtility::GetArray(jsonObj, "FirstEmmisive", &m_firstEmmisive.x, 3);
		KdJsonUtility::GetValue(jsonObj, "WaitChangeSpeed", &m_waitChangeSpeed);
		KdJsonUtility::GetValue(jsonObj, "ChangeSpeed", &m_changeSpeed);
	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override
	{
		DrawTerrain::Serialize(outJson);
		outJson["ChangeTime"] = m_changeTime;
		outJson["Chain"] = m_isChain;
		outJson["Wait"] = m_waitTime;
		outJson["FirstEmmisive"] = KdJsonUtility::CreateArray(&m_firstEmmisive.x,3);
		outJson["SecondEmmisive"] = KdJsonUtility::CreateArray(&m_secondEmmisive.x,3);
		outJson["WaitChangeSpeed"] = m_waitChangeSpeed;
		outJson["ChangeSpeed"] = m_changeSpeed;
	}

private:

	float m_waitTime = 4.0f;
	float m_durationWait = 0.0f;
	float m_waitChangeSpeed = 1.0f;

	float m_changeTime = 2.0f;
	float m_durationChange = 0.0f;
	float m_changeSpeed = 3.0f;


	Math::Vector3 m_firstEmmisive = {};
	Math::Vector3 m_secondEmmisive = {};

	bool m_isEnableChange = false;

	bool m_isChain = true;

	int m_subScript = 0;

};