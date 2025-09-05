#pragma once

#include "../Terrain/DrawTerrain.h"

class TitleMovie :public DrawTerrain
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
		KdJsonUtility::GetArray(jsonObj, "FirstPos", &m_firstPos.x, 3);
		KdJsonUtility::GetArray(jsonObj, "SecondPos", &m_secondPos.x, 3);
		KdJsonUtility::GetArray(jsonObj, "ThirdPos", &m_thirdPos.x, 3);
		KdJsonUtility::GetArray(jsonObj, "ForthPos", &m_forthPos.x, 3);
		KdJsonUtility::GetValue(jsonObj, "FirstTime", &m_firstTime);
		KdJsonUtility::GetValue(jsonObj, "SecondTime",&m_secondTime);
		KdJsonUtility::GetValue(jsonObj, "ThirdTime", &m_thirdTime);
		KdJsonUtility::GetValue(jsonObj, "ForthTime", &m_forthTime);
		KdJsonUtility::GetValue(jsonObj, "FirstWaitTime",&m_firstWaitTime);
		KdJsonUtility::GetValue(jsonObj, "SecondWaitTime",&m_secondWaitTime);
		KdJsonUtility::GetValue(jsonObj, "ThirdWaitTime",&m_thirdWaitTime);
		KdJsonUtility::GetValue(jsonObj, "ForthWaitTime",&m_forthWaitTime);
		
		m_spAnimator->SetAnimation(m_spModel->GetAnimation("Stand"), 10.0f, false);

	}

	// このクラスの内容をJSONデータ化する
	virtual void Serialize(nlohmann::json& outJson) const override
	{
		DrawTerrain::Serialize(outJson);

		outJson["FirstPos"] = KdJsonUtility::CreateArray(&m_firstPos.x, 3);
		outJson["SecondPos"] = KdJsonUtility::CreateArray(&m_secondPos.x, 3);
		outJson["ThirdPos"] = KdJsonUtility::CreateArray(&m_thirdPos.x, 3);
		outJson["ForthPos"] = KdJsonUtility::CreateArray(&m_forthPos.x, 3);
		outJson["FirstTime"]   = m_firstTime;
		outJson["SecondTime"] = m_secondTime;
		outJson["ThirdTime"]   = m_thirdTime;
		outJson["ForthTime"]   = m_forthTime;
		outJson["FirstWaitTime"]   = m_firstWaitTime;
		outJson["SecondWaitTime"] = m_secondWaitTime;
		outJson["ThirdWaitTime"]   = m_thirdWaitTime;
		outJson["ForthWaitTime"]   = m_forthWaitTime;
	}

private:

	enum ProgressType {
		OnetoTwo,
		TwotoThree,
		ThreetoFour,
		FourtoFive,
	};

	Math::Vector3 m_firstPos = {};
	float m_firstTime = 1.0f;
	float m_durationMove = 0.0f;
	
	Math::Vector3 m_secondPos = {};
	float m_secondTime = 1.0f;
	
	Math::Vector3 m_thirdPos = {};
	float m_thirdTime = 1.0f;
	
	Math::Vector3 m_forthPos = {};
	float m_forthTime = 1.0f;

	float m_firstWaitTime = 0.2f;
	float m_secondWaitTime = 0.2f;
	float m_thirdWaitTime = 0.2f;
	float m_forthWaitTime = 0.2f;
	float m_durationWait = 0.0f;

	bool m_isThurster = false;
	bool m_isFirstAnim = true;
	bool m_isSecondAnim = true;

	std::weak_ptr<KdEffekseerObject> m_efk;

	ProgressType m_type = OnetoTwo;
};